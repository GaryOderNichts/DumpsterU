#include "GameList.h"

#include "wfslib/WfsLib.h"
#include <iostream>

GameList::GameList(Glib::RefPtr<Gtk::Builder> builder, std::vector<TitleParser::TitleInfo>& infos, const std::shared_ptr<FileDevice>& device, std::vector<uint8_t>& key)
{
    this->builder = builder;
    this->device = device;
    this->key = key;
    this->infos = infos;

    builder->get_widget("gameListWindow", gameListWindow);
    gameListWindow->show();

    Gtk::ImageMenuItem* aboutItem = nullptr;
    builder->get_widget("gameListAbout", aboutItem);
    aboutItem->signal_activate().connect(sigc::mem_fun(*this, &GameList::on_about_click));

    builder->get_widget("gameTree", treeView);
    treeView->signal_row_activated().connect(sigc::mem_fun(*this, &GameList::on_gamelist_row_activated));
    
    Glib::RefPtr<Gtk::ListStore> treeModel = Gtk::ListStore::create(columns);
    treeView->set_model(treeModel);

    for (unsigned int i = 0; i < infos.size(); i++)
    {
        Gtk::TreeModel::Row row = *(treeModel->append());

        row[columns.index] = i;
        if (infos[i].icon)
        {
            row[columns.icon] = infos[i].icon->scale_simple(48, 48, Gdk::INTERP_BILINEAR);
        }
        row[columns.name] = infos[i].name;
        row[columns.titleId] = infos[i].titleId;
    }

    treeView->append_column("Icon", columns.icon);
    treeView->get_column(0)->set_min_width(64);

    treeView->append_column("TitleID", columns.titleId);
    treeView->get_column(1)->set_min_width(140);

    treeView->append_column("Name", columns.name);
    treeView->get_column(2)->set_min_width(512);

    // Search for name
    treeView->set_search_column(3);
}

GameList::~GameList()
{
    delete dumpWindow;
}

void GameList::on_gamelist_row_activated(const Gtk::TreePath& treePath, Gtk::TreeViewColumn* const& column)
{
    Glib::RefPtr<Gtk::TreeSelection> selection = treeView->get_selection();
    Gtk::TreeModel::Row row = *selection->get_selected();

    dumpWindow = new DumpWindow(builder, infos[row[columns.index]], device, key);
    gameListWindow->get_application()->add_window(*dumpWindow->getWindow());
    dumpWindowConn = dumpWindow->getWindow()->signal_hide().connect(sigc::mem_fun(*this, &GameList::on_dumpWindow_closed));
    dumpWindow->getWindow()->set_transient_for(*gameListWindow);
    dumpWindow->getWindow()->set_modal(true);
    gameListWindow->set_sensitive(false);
    deleteConn = gameListWindow->signal_delete_event().connect(sigc::mem_fun(*this, &GameList::on_gamelist_delete_event));
}

void GameList::on_dumpWindow_closed()
{
    dumpWindowConn.disconnect();
    deleteConn.disconnect();
    dumpWindow->getWindow()->set_modal(false);
    delete dumpWindow;
    dumpWindow = nullptr;

    gameListWindow->set_sensitive(true);
}

bool GameList::on_gamelist_delete_event(GdkEventAny* event)
{
    // Do not allow closing the window
    return true;
}

void GameList::on_about_click()
{
    builder->get_widget("aboutWindow", aboutDialog);

#ifdef _WIN32
    Gtk::ButtonBox* aboutButtonBox = nullptr;
    builder->get_widget("aboutButtonBox", aboutButtonBox);

    std::vector<Gtk::Widget*> children = aboutButtonBox->get_children();
    Gtk::Button* closeButton = (Gtk::Button*) (children[1]);
    aboutConn = closeButton->signal_clicked().connect(sigc::mem_fun(*this, &GameList::on_aboutClose_click));
#endif

    aboutDialog->show();
}

void GameList::on_aboutClose_click()
{
    aboutDialog->hide();
    aboutConn.disconnect();
}