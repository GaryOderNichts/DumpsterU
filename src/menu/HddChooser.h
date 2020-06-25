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