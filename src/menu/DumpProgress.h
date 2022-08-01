/**
 *  Copyright (C) 2020 GaryOderNichts
 *  This file is part of DumpsterU <https://github.com/GaryOderNichts/DumpsterU>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <string>
#include "include/wfslib/wfslib.h"
#include <gtkmm.h>
#include <boost/filesystem.hpp>
#include <map>

#include <mutex>

class DumpProgress
{
public:
    DumpProgress(Glib::RefPtr<Gtk::Builder> builder, const std::shared_ptr<FileDevice> &device, std::vector<std::byte> &key);
    ~DumpProgress();

    void startDump(const std::string &dumpPath, const std::string &outputPath);
    void cancelDump();

    Gtk::Window *getWindow() { return dumpProgressWindow; }
    bool getDumpDone() { return dumpDone; }

    bool updateProgress();

private:
    void dumpDirectory(const boost::filesystem::path &target, const std::shared_ptr<Directory> &dir, const boost::filesystem::path &path);
    void countFiles(const std::shared_ptr<Directory> &dir);
    void dumpThreadFunction(const std::string &dumpPath, const std::string &outputPath);

    Gtk::Window *dumpProgressWindow = nullptr;
    Gtk::ProgressBar *progressBar = nullptr;
    Gtk::Label *currentDumpFile = nullptr;
    Gtk::Label *progressSize = nullptr;
    Gtk::Label *finishedLabel = nullptr;
    Gtk::Label *errorLabel = nullptr;
    Gtk::Button *buttonOk = nullptr;
    Gtk::Button *buttonCancel = nullptr;

    std::shared_ptr<FileDevice> device;
    std::vector<std::byte> key;

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