#include "DumpProgress.h"

#include <thread>
#include <chrono>

DumpProgress::DumpProgress(Glib::RefPtr<Gtk::Builder> builder, const std::shared_ptr<FileDevice>& device, std::vector<uint8_t>& key)
{
    this->device = device;
    this->key = key;

    builder->get_widget("progressWindow", dumpProgressWindow);
    dumpProgressWindow->show();

    builder->get_widget("progressScrolledWindow", scrolledWindow);

    builder->get_widget("progressConsole", progressConsole);
    progressConsole->get_buffer()->erase(progressConsole->get_buffer()->begin(), progressConsole->get_buffer()->end());

    timeoutConn = Glib::signal_timeout().connect(sigc::mem_fun(*this, &DumpProgress::consoleUpdate), 75);
}

DumpProgress::~DumpProgress()
{
    timeoutConn.disconnect();
}

void DumpProgress::dumpDirectory(const boost::filesystem::path& target, const std::shared_ptr<Directory>& dir, const boost::filesystem::path& path, int* outFilesAmount, int* outErrorAmount)
{
    if (stopDump)
    {
        return;
    }

	if (!boost::filesystem::exists(target / path))
    {
		if (!boost::filesystem::create_directories(target / path))
        {
			writeToConsole(std::string("Error: Cannot create directory: ") + (target / path).string(), false, ConsoleColor::Red);
            (*outErrorAmount)++;
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
			writeToConsole(std::string("Dumping ") + npath.string());
			if (item->IsDirectory())
            {
                dumpDirectory(target, std::dynamic_pointer_cast<Directory>(item), npath, outFilesAmount, outErrorAmount);
            }
			else if (item->IsFile()) 
            {
				auto file = std::dynamic_pointer_cast<File>(item);
				std::ofstream output_file((target / npath).string(), std::ios::binary | std::ios::out);
				size_t size = file->GetSize();
                size_t to_read = size;
				File::stream stream(file);
				std::vector<char> data(0x2000);
                bool err = false;
				while (to_read > 0) 
                {
					stream.read(&*data.begin(), std::min(data.size(), to_read));
					auto read = stream.gcount();
					if (read <= 0) 
                    {
						writeToConsole(std::string("Error: Failed to read ") + npath.string(), false, ConsoleColor::Red);
                        err = true;
						break;
					}
					output_file.write((char*)&*data.begin(), read);
					to_read -= static_cast<size_t>(read);

                    // don't update if we have to many pending messages or if the file isnt large enough
                    if (pendingMessages.size() < 1000 && size > (1024 * 1024 * 20))
                    {
                        writeToConsole(item->GetRealName() + std::string(" ") + std::to_string(size - to_read) + std::string(" / ") + std::to_string(size), true);
                    }

                    if (stopDump)
                    {
                        // Remove the not fully written file
                        std::remove((target / npath).c_str());
                        break;
                    }
				}

                if (err)
                {
                    (*outErrorAmount)++;
                }
                else
                {
                    (*outFilesAmount)++;
                }
                

				output_file.close();
			}
		}
	}
	catch (Block::BadHash&) 
    {
		writeToConsole(std::string("Error: Failed to dump folder ") + path.string(), false, ConsoleColor::Red);
        (*outErrorAmount)++;
	}

    consoleMutex.lock();
    if (path.empty())
        threadDone = true;
    consoleMutex.unlock();
}

bool DumpProgress::consoleUpdate()
{
    if (threadDone)
    {
        return false;
    }

    if (progressConsole)
    {
        Glib::RefPtr<Gtk::TextBuffer> buf = progressConsole->get_buffer();

        for (auto message : pendingMessages)
        {
            if (message.cr && lastCr)
            {
                Gtk::TextBuffer::iterator it = buf->get_iter_at_line(buf->get_line_count() - 1);
                it.backward_char();
                buf->erase(it, buf->end());
                it = buf->get_iter_at_line(buf->get_line_count() - 1);
                buf->erase(it, buf->end());
            }
            lastCr = message.cr;

            if (!threadDone)
            {
                buf->insert_markup(buf->end(), consoleColorMap[message.color] + message.message + std::string("</span>\n"));
                buf->place_cursor(buf->end());
            }
        }
        consoleMutex.lock();
        pendingMessages.clear();
        consoleMutex.unlock();

        // scroll to bottom
        Glib::RefPtr<Gtk::Adjustment> adj = scrolledWindow->get_vadjustment();
        adj->set_value(adj->get_upper());

        // // Update
        // while (Gtk::Main::events_pending())
        // {
        //     Gtk::Main::iteration();
        // }
    }

    return !threadDone;
}

void DumpProgress::writeToConsole(const std::string& toWrite, bool cr, const ConsoleColor& color)
{
    PendingMessage message = { toWrite, cr, color };
    consoleMutex.lock();
    pendingMessages.push_back(message);
    consoleMutex.unlock();
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
    std::shared_ptr<Directory> dir = Wfs(device, key).GetDirectory(dumpPath);
    if (dir)
    {

        int fileAmount = 0;
        int errAmount = 0;
        std::thread dumpThread(&DumpProgress::dumpDirectory, this, outputPath, dir, "", &fileAmount, &errAmount);
        dumpThread.join();

        writeToConsole(std::string("Finished dumping ") + std::to_string(fileAmount) + std::string(" file(s) to ") + outputPath, false, ConsoleColor::Green);
        writeToConsole(std::to_string(errAmount) + std::string(" error(s) occured"), false, errAmount > 0 ? ConsoleColor::Red : ConsoleColor::Green);
    }
    else
    {
        writeToConsole(std::string("Error: Cannot open Game Directory ") + dumpPath, false, ConsoleColor::Red);
    }

    // // scroll to bottom
    // Glib::RefPtr<Gtk::Adjustment> adj = scrolledWindow->get_vadjustment();
    // adj->set_value(adj->get_upper());

    cancelMutex.lock();
    dumpDone = true;
    cancelMutex.unlock();
}