#pragma once

#include <string>
#include "wfslib/WfsLib.h"
#include <gtkmm.h>
#include <boost/filesystem.hpp>
#include <map>

#include <mutex>

class DumpProgress
{
public:
    DumpProgress(Glib::RefPtr<Gtk::Builder> builder, const std::shared_ptr<FileDevice>& device, std::vector<uint8_t>& key);
    ~DumpProgress();

    void startDump(const std::string& dumpPath, const std::string& outputPath);
    void cancelDump();

    Gtk::Window* getWindow() { return dumpProgressWindow; }
    bool getDumpDone() { return dumpDone; }

    bool updateProgress();

private:
    void dumpDirectory(const boost::filesystem::path& target, const std::shared_ptr<Directory>& dir, const boost::filesystem::path& path);
    void countFiles(const std::shared_ptr<Directory>& dir);
    void dumpThreadFunction(const std::string& dumpPath, const std::string& outputPath);

    Gtk::Window* dumpProgressWindow = nullptr;
    Gtk::ProgressBar* progressBar = nullptr;
    Gtk::Label* currentDumpFile = nullptr;
    Gtk::Label* progressSize = nullptr;
    Gtk::Label* finishedLabel = nullptr;
    Gtk::Label* errorLabel = nullptr;
    Gtk::Button* buttonOk = nullptr;
    Gtk::Button* buttonCancel = nullptr;

    std::shared_ptr<FileDevice> device;
    std::vector<uint8_t> key;

    int totalFileCount = 0;
    int currentFileCount = 0;
    int currentErrorCount = 0;

    std::string currentFileName;
    size_t currentTotalFileSize = 0;
    size_t currentFileSize = 0;
    std::string rawOutputPath;

    sigc::connection timeoutConn;

    bool stopDump = false;
    bool dumpDone = false;

    std::mutex cancelMutex;

    std::vector<sigc::connection> conns;
};