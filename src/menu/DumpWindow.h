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

#include "wfslib/WfsLib.h"
#include "utils/TitleParser.h"
#include <vector>

#include "DumpProgress.h"

class DumpWindow
{
public:
    class UserModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        UserModelColumns()
        {
            add(index);
            add(user);
        }

        Gtk::TreeModelColumn<int> index;
        Gtk::TreeModelColumn<Glib::ustring> user;
    };

    DumpWindow(Glib::RefPtr<Gtk::Builder> builder, const TitleParser::TitleInfo& info, const std::shared_ptr<FileDevice>& device, std::vector<uint8_t>& key);
    ~DumpWindow();

    void on_button_folder_clicked();

    void on_savelist_row_activated(const Gtk::TreePath& treePath, Gtk::TreeViewColumn* const& column);
    void on_dumpGameButton_click();
    void on_dumpUpdateButton_click();
    void on_dumpDlcButton_click();
    void on_progressWindow_closed();
    bool on_dumpWindow_delete_event(GdkEventAny* event);

    Gtk::Window* getWindow() { return dumpWindow; }

private:
    Glib::RefPtr<Gtk::Builder> builder;
    std::shared_ptr<FileDevice> device;
    std::vector<uint8_t> key;

    Gtk::Window* dumpWindow = nullptr;

    Gtk::TreeView* treeView = nullptr;
    UserModelColumns columns;
    TitleParser::AdditionalInfo additionalInfo;

    Gtk::FileChooserButton* outputChooser = nullptr;
    DumpProgress* progressWindow = nullptr;
    sigc::connection progressWindowConn;
    sigc::connection dumpWindowDeleteConn;

    std::vector<sigc::connection> connections;
};