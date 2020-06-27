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

#include <gtkmm.h>
#include <string>
#include <vector>
#include "wfslib/WfsLib.h"

#include "DumpWindow.h"
#include "utils/TitleParser.h"

class GameList
{
public:
    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        ModelColumns()
        {
            add(index);
            add(icon);
            add(titleId);
            add(name);
        }

        Gtk::TreeModelColumn<int> index;
        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> icon;
        Gtk::TreeModelColumn<Glib::ustring> titleId;
        Gtk::TreeModelColumn<Glib::ustring> name;
    };

    GameList(Glib::RefPtr<Gtk::Builder> builder, std::vector<TitleParser::TitleInfo>& infos, const std::shared_ptr<FileDevice>& device, std::vector<uint8_t>& key);
    ~GameList();

    void on_gamelist_row_activated(const Gtk::TreePath& treePath, Gtk::TreeViewColumn* const& column);
    bool on_gamelist_delete_event(GdkEventAny* event);
    void on_dumpWindow_closed();
    void on_about_click();
    void on_aboutClose_click();

    Gtk::Window* getWindow() { return gameListWindow; }

    bool on_search_equal(const Glib::RefPtr<Gtk::TreeModel>& model, int column, const Glib::ustring& key, const Gtk::TreeModel::iterator& iter);

private:
    Gtk::Window* gameListWindow = nullptr;
    Glib::RefPtr<Gtk::Builder> builder;
    std::vector<uint8_t> key;
    std::shared_ptr<FileDevice> device = nullptr;

    Gtk::TreeView* treeView = nullptr;
    ModelColumns columns;
    std::vector<TitleParser::TitleInfo> infos;

    DumpWindow* dumpWindow = nullptr;
    sigc::connection dumpWindowConn;
    sigc::connection deleteConn;

    Gtk::AboutDialog* aboutDialog = nullptr;
    sigc::connection aboutConn;
};