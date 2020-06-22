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