#include "DumpProgress.h"

#include <thread>
#include <boost/format.hpp>

DumpProgress::DumpProgress(Glib::RefPtr<Gtk::Builder> builder, const std::shared_ptr<FileDevice>& device, std::vector<uint8_t>& key)
{
    this->device = device;
    this->key = key;

    builder->get_widget("progressWindow", dumpProgressWindow);
    dumpProgressWindow->show();

    builder->get_widget("progressBar", progressBar);
    progressBar->set_fraction(0.0);

    builder->get_widget("currentDumpFile", currentDumpFile);
    builder->get_widget("progressSize", progressSize);

    builder->get_widget("finishedLabel", finishedLabel);
    finishedLabel->set_visible(false);

    builder->get_widget("progressErrorCount", errorLabel);
    errorLabel->set_visible(false);

    builder->get_widget("progressOk", buttonOk);
    buttonOk->set_sensitive(false);
    conns.push_back(buttonOk->signal_clicked().connect(sigc::mem_fun(*dumpProgressWindow, &Gtk::Window::hide)));

    builder->get_widget("progressCancel", buttonCancel);
    conns.push_back(buttonCancel->signal_clicked().connect(sigc::mem_fun(*this, &DumpProgress::cancelDump)));
    buttonCancel->set_sensitive();

    // update progress every 50ms
    timeoutConn = Glib::signal_timeout().connect(sigc::mem_fun(*this, &DumpProgress::updateProgress), 50);
}

DumpProgress::~DumpProgress()
{
    timeoutConn.disconnect();
    for (auto conn : conns)
    {
        conn.disconnect();
    }
}

void DumpProgress::dumpDirectory(const boost::filesystem::path& target, const std::shared_ptr<Directory>& dir, const boost::filesystem::path& path)
{
    if (stopDump)
    {
        return;
    }

	if (!boost::filesystem::exists(target / path))
    {
		if (!boost::filesystem::create_directories(target / path))
        {
			std::cerr << "Error: Cannot create directory: " << (target / path) << "\n";
            currentErrorCount++;
			return;
		}
	}

	try 
    {
		for (auto item : *dir) 
        {
            if (stopDump)
                break;

			boost::filesystem::path npath = path / item->GetRealName();
			if (item->IsDirectory())
            {
                dumpDirectory(target, std::dynamic_pointer_cast<Directory>(item), npath);
            }
			else if (item->IsFile()) 
            {
				auto file = std::dynamic_pointer_cast<File>(item);

				std::ofstream output_file((target / npath).string(), std::ios::binary | std::ios::out);

				size_t size = file->GetSize();
                size_t to_read = size;
                currentTotalFileSize = size;

				File::stream stream(file);
				std::vector<char> data(0x2000);

                currentFileName = file->GetRealName();

                bool err = false;
				while (to_read > 0) 
                {
					stream.read(&*data.begin(), std::min(data.size(), to_read));
					auto read = stream.gcount();
					if (read <= 0) 
                    {
						std::cerr << "Error: Failed to read " << npath << "\n";
                        err = true;
						break;
					}
					output_file.write((char*)&*data.begin(), read);
					to_read -= static_cast<size_t>(read);

                    currentFileSize = size - to_read;

                    if (stopDump)
                    {
                        // Remove the not fully written file
                        std::remove((target / npath).c_str());
                        break;
                    }
				}

                currentFileCount++;
                if (err)
                {
                    currentErrorCount++;
                }

				output_file.close();
			}
		}
	}
	catch (Block::BadHash&) 
    {
		std::cerr << "Error: Failed to dump folder " << path << "\n";
        currentErrorCount++;
	}
}

void DumpProgress::countFiles(const std::shared_ptr<Directory>& dir)
{
    if (!dir)
        return;

    for (auto item : *dir) 
    {
        if (item->IsDirectory())
        {
            countFiles(std::dynamic_pointer_cast<Directory>(item));
        }
        else if (item->IsFile())
        {
            totalFileCount++;
        }
    }
}

bool DumpProgress::updateProgress()
{
    progressBar->set_fraction((double) currentFileCount / totalFileCount);
    progressBar->set_text(std::to_string(currentFileCount) + std::string(" / ") + std::to_string(totalFileCount));

    currentDumpFile->set_text(std::string("Dumping ") + currentFileName.substr(0, 34) + std::string("..."));

    if (currentFileSize >  1024 * 1024 * 5) // Display in mb if larger than 5mb
    {
        progressSize->set_text(boost::str(boost::format("%.2f") % (currentFileSize / 1024 / 1024)) + std::string(" / ") + boost::str(boost::format("%.2f") % (currentTotalFileSize / 1024 / 1024)) + std::string(" MB"));
    }
    else // display in kb
    {
        progressSize->set_text(boost::str(boost::format("%.2f") % (currentFileSize / 1024)) + std::string(" / ") + boost::str(boost::format("%.2f") % (currentTotalFileSize / 1024)) + std::string(" KB"));
    }

    if (dumpDone)
    {
        if (!stopDump)
        {
            Pango::AttrList att = finishedLabel->get_attributes();
            Pango::Attribute green = Pango::Attribute::create_attr_foreground(0x8a8a, 0xe2e2, 0x3434);
            att.insert(green);
            finishedLabel->set_attributes(att);
            finishedLabel->set_text(std::string("Finished dumping ") + std::to_string(currentFileCount) + std::string(" file(s) to ") + rawOutputPath.substr(0, 23) + std::string("..."));
            finishedLabel->set_visible();

            att = errorLabel->get_attributes();
            Pango::Attribute red = Pango::Attribute::create_attr_foreground(0xEF00, 0x2900, 0x2900);
            att.insert(currentErrorCount > 0 ? red : green);
            errorLabel->set_attributes(att);
            errorLabel->set_text(std::to_string(currentErrorCount) + std::string(" error(s) occured"));
            errorLabel->set_visible();
        }
        else
        {
            Pango::AttrList att = finishedLabel->get_attributes();
            Pango::Attribute yellow = Pango::Attribute::create_attr_foreground(0xED00, 0xD400, 0x2900);
            att.insert(yellow);
            finishedLabel->set_attributes(att);
            finishedLabel->set_text(std::string("Dump cancelled"));
            finishedLabel->set_visible();
        }
        
        buttonOk->set_sensitive();
        buttonCancel->set_sensitive(false);
    }

    return !dumpDone;//(currentFileCount / totalFileCount) > 0.99;
}

void DumpProgress::startDump(const std::string& dumpPath, const std::string& outputPath)
{
    std::thread t(&DumpProgress::dumpThreadFunction, this, dumpPath, outputPath);
    t.detach();
}

void DumpProgress::cancelDump()
{
    cancelMutex.lock();
    stopDump = true;
    cancelMutex.unlock();
}

void DumpProgress::dumpThreadFunction(const std::string& dumpPath, const std::string& outputPath)
{
    rawOutputPath = outputPath;

    std::shared_ptr<Directory> dir = Wfs(device, key).GetDirectory(dumpPath);
    if (dir)
    {
        countFiles(dir);
        std::thread dumpThread(&DumpProgress::dumpDirectory, this, outputPath, dir, "");
        dumpThread.join();
    }
    else
    {
        std::cerr << "Error: Cannot open Directory " << dumpPath << "\n";
    }

    cancelMutex.lock();
    dumpDone = true;
    cancelMutex.unlock();
}