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

#include "GameList.h"
#include "utils/DiskUtils.h"

class HddChooser
{
public:
    class HddModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        HddModelColumns() 
        { 
            add(index);
            add(name);
            add(size);
        }

        Gtk::TreeModelColumn<int> index;
        Gtk::TreeModelColumn<Glib::ustring> name;
        Gtk::TreeModelColumn<Glib::ustring> size;
    };

    HddChooser(Glib::RefPtr<Gtk::Builder> builder);
    ~HddChooser();

    Gtk::Window* getWindow() const { return chooseHddWindow; }
    GameList* getList() const { return list; }

    void on_chooseHddOkButton_clicked();
    void on_chooseHddRefreshButton_clicked();
    void on_about_click();
    void on_aboutClose_click();

private:
    Glib::RefPtr<Gtk::Builder> builder;

    Gtk::Window* chooseHddWindow = nullptr;

    Gtk::ComboBox* hdds = nullptr;
    HddModelColumns columns;
    Glib::RefPtr<Gtk::ListStore> treeModel;
    std::vector<DiskUtils::Disk> disks;

    Gtk::FileChooserButton* otpButton = nullptr;
    Gtk::FileChooserButton* seepromButton = nullptr;

    GameList* list = nullptr;

    Gtk::AboutDialog* aboutDialog = nullptr;
    sigc::connection aboutConn;
};