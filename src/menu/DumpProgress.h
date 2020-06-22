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
    enum ConsoleColor
    {
        Default = 0,
        Red = 1,
        Green = 2
    };

    static constexpr const char* consoleColorMap[] = 
    {
        "<span color=\"white\">",
        "<span color=\"red\">",
        "<span color=\"green\">",
    };

    struct PendingMessage
    {
        std::string message;
        bool cr;
        ConsoleColor color;
    };

    DumpProgress(Glib::RefPtr<Gtk::Builder> builder, const std::shared_ptr<FileDevice>& device, std::vector<uint8_t>& key);
    ~DumpProgress();

    void writeToConsole(const std::string& toWrite, bool cr = false, const ConsoleColor& color = ConsoleColor::Default);
    void startDump(const std::string& dumpPath, const std::string& outputPath);
    void cancelDump();

    Gtk::Window* getWindow() { return dumpProgressWindow; }
    bool getDumpDone() { return dumpDone; }

    bool consoleUpdate();

private:
    void dumpDirectory(const boost::filesystem::path& target, const std::shared_ptr<Directory>& dir, const boost::filesystem::path& path, int* outFilesAmount, int* outErrorAmount);
    void dumpThreadFunction(const std::string& dumpPath, const std::string& outputPath);

    Gtk::Window* dumpProgressWindow = nullptr;
    Gtk::TextView* progressConsole = nullptr;
    Gtk::ScrolledWindow* scrolledWindow;
    std::shared_ptr<FileDevice> device;
    std::vector<uint8_t> key;

    std::vector<PendingMessage> pendingMessages;
    std::mutex consoleMutex;
    std::mutex cancelMutex;
    bool lastCr = false;
    bool threadDone = false;
    bool dumpDone = false;

    sigc::connection timeoutConn;

    bool stopDump = false;
};