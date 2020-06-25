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