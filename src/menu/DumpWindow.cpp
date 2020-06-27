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

#include "DumpWindow.h"

#include <boost/filesystem.hpp>

DumpWindow::DumpWindow(Glib::RefPtr<Gtk::Builder> builder, const TitleParser::TitleInfo& info, const std::shared_ptr<FileDevice>& device, std::vector<uint8_t>& key)
{
    this->builder = builder;
    this->device = device;
    this->key = key;

    builder->get_widget("dumpWindow", dumpWindow);
    dumpWindow->show();

    Gtk::Image* iconImage = nullptr;
    builder->get_widget("dumpIcon", iconImage);
    iconImage->set(info.icon);

    Gtk::Label* name = nullptr;
    builder->get_widget("dumpTitle", name);
    name->set_text(info.name);

    Gtk::Fixed* fixed = nullptr;
    builder->get_widget("dumpWindowFixed", fixed);
    
    builder->get_widget("dumpOutputDir", outputChooser);

    Gtk::Button* dumpGameButton = nullptr;
    builder->get_widget("dumpGameButton", dumpGameButton);
    connections.push_back(dumpGameButton->signal_clicked().connect(sigc::mem_fun(*this, &DumpWindow::on_dumpGameButton_click)));

    Gtk::Button* dumpUpdateButton = nullptr;
    builder->get_widget("dumpUpdateButton", dumpUpdateButton);
    connections.push_back(dumpUpdateButton->signal_clicked().connect(sigc::mem_fun(*this, &DumpWindow::on_dumpUpdateButton_click)));

    Gtk::Button* dumpDlcButton = nullptr;
    builder->get_widget("dumpDLCButton", dumpDlcButton);
    connections.push_back(dumpDlcButton->signal_clicked().connect(sigc::mem_fun(*this, &DumpWindow::on_dumpDlcButton_click)));

    builder->get_widget("dumpSaveView", treeView);
    Glib::RefPtr<Gtk::ListStore> treeModel = Gtk::ListStore::create(columns);
    treeView->set_model(treeModel);
    treeView->set_search_column(-1);
    connections.push_back(treeView->signal_row_activated().connect(sigc::mem_fun(*this, &DumpWindow::on_savelist_row_activated)));

    Gtk::Label* versionLabel = nullptr;
    builder->get_widget("dumpWindowVersionLabel", versionLabel);

    additionalInfo = TitleParser::getAdditionalInfo(info.titleId, device, key);

    if (additionalInfo.updatePath.empty())
    {
        versionLabel->set_text("");
        dumpUpdateButton->set_sensitive(false);
    }
    else
    {
        versionLabel->set_text(std::string("Version: ") + additionalInfo.updateVersion);

        dumpUpdateButton->set_sensitive(true);
    }

    if (additionalInfo.dlcPath.empty())
    {
        dumpDlcButton->set_sensitive(false);
    }
    else
    {
        dumpDlcButton->set_sensitive(true);
    }

    for (unsigned int i = 0; i < additionalInfo.users.size(); i++)
    {
        Gtk::TreeModel::Row row = *(treeModel->append());
        row[columns.index] = i;
        row[columns.user] = additionalInfo.users[i];
    }

    // don't add the column if it already exists
    if (treeView->get_n_columns() == 0)
    {
        treeView->append_column("User", columns.user);
    }
}

DumpWindow::~DumpWindow()
{
    // Disconnect all connections
    for (auto conn : connections)
    {
        conn.disconnect();
    }

    delete progressWindow;
}

void DumpWindow::on_savelist_row_activated(const Gtk::TreePath& treePath, Gtk::TreeViewColumn* const& column)
{
    Glib::RefPtr<Gtk::TreeSelection> selection = treeView->get_selection();
    Gtk::TreeModel::Row row = *selection->get_selected();

    std::string outputPath = outputChooser->get_filename();
    if (!additionalInfo.savePath.empty() && !outputPath.empty())
    {
        boost::filesystem::path fullOutputPath = boost::filesystem::path(outputPath) / boost::filesystem::path(additionalInfo.shortTid) / boost::filesystem::path("saves") / boost::filesystem::path(additionalInfo.users[row[columns.index]]);
        boost::filesystem::path dumpPath = boost::filesystem::path(additionalInfo.savePath) / additionalInfo.users[row[columns.index]];

        progressWindow = new DumpProgress(builder, device, key);
        progressWindowConn = progressWindow->getWindow()->signal_hide().connect(sigc::mem_fun(*this, &DumpWindow::on_progressWindow_closed));
        progressWindow->getWindow()->set_transient_for(*dumpWindow);
        progressWindow->getWindow()->set_modal(true);
        dumpWindow->set_sensitive(false);

        dumpWindowDeleteConn = dumpWindow->signal_delete_event().connect(sigc::mem_fun(*this, &DumpWindow::on_dumpWindow_delete_event));

        progressWindow->startDump(dumpPath.string(), fullOutputPath.string());
    }
}

void DumpWindow::on_dumpGameButton_click()
{
    std::string outputPath = outputChooser->get_filename();
    if (!outputPath.empty())
    {
        boost::filesystem::path fullOutputPath = boost::filesystem::path(outputPath) / boost::filesystem::path(additionalInfo.shortTid) / boost::filesystem::path("game");

        progressWindow = new DumpProgress(builder, device, key);
        progressWindowConn = progressWindow->getWindow()->signal_hide().connect(sigc::mem_fun(*this, &DumpWindow::on_progressWindow_closed));
        progressWindow->getWindow()->set_transient_for(*dumpWindow);
        progressWindow->getWindow()->set_modal(true);
        dumpWindow->set_sensitive(false);

        dumpWindowDeleteConn = dumpWindow->signal_delete_event().connect(sigc::mem_fun(*this, &DumpWindow::on_dumpWindow_delete_event));

        progressWindow->startDump(additionalInfo.gamePath, fullOutputPath.string());
    }
}

void DumpWindow::on_dumpUpdateButton_click()
{
    std::string outputPath = outputChooser->get_filename();
    if (!outputPath.empty() && !additionalInfo.updatePath.empty())
    {
        boost::filesystem::path fullOutputPath = boost::filesystem::path(outputPath) / boost::filesystem::path(additionalInfo.shortTid) / boost::filesystem::path("update");

        progressWindow = new DumpProgress(builder, device, key);
        progressWindowConn = progressWindow->getWindow()->signal_hide().connect(sigc::mem_fun(*this, &DumpWindow::on_progressWindow_closed));
        progressWindow->getWindow()->set_transient_for(*dumpWindow);
        progressWindow->getWindow()->set_modal(true);
        dumpWindow->set_sensitive(false);

        dumpWindowDeleteConn = dumpWindow->signal_delete_event().connect(sigc::mem_fun(*this, &DumpWindow::on_dumpWindow_delete_event));

        progressWindow->startDump(additionalInfo.updatePath, fullOutputPath.string());
    }
}

void DumpWindow::on_dumpDlcButton_click()
{
    std::string outputPath = outputChooser->get_filename();
    if (!outputPath.empty() && !additionalInfo.dlcPath.empty())
    {
        boost::filesystem::path fullOutputPath = boost::filesystem::path(outputPath) / boost::filesystem::path(additionalInfo.shortTid) / boost::filesystem::path("dlc");

        progressWindow = new DumpProgress(builder, device, key);
        progressWindowConn = progressWindow->getWindow()->signal_hide().connect(sigc::mem_fun(*this, &DumpWindow::on_progressWindow_closed));
        progressWindow->getWindow()->set_transient_for(*dumpWindow);
        progressWindow->getWindow()->set_modal(true);
        dumpWindow->set_sensitive(false);

        dumpWindowDeleteConn = dumpWindow->signal_delete_event().connect(sigc::mem_fun(*this, &DumpWindow::on_dumpWindow_delete_event));

        progressWindow->startDump(additionalInfo.dlcPath, fullOutputPath.string());
    }
}

void DumpWindow::on_progressWindow_closed()
{
    progressWindow->cancelDump();
    while (!progressWindow->getDumpDone()) { }
    
    dumpWindowDeleteConn.disconnect();
    progressWindowConn.disconnect();

    progressWindow->getWindow()->set_modal(false);

    delete progressWindow;
    progressWindow = nullptr;

    dumpWindow->set_sensitive(true);
}

bool DumpWindow::on_dumpWindow_delete_event(GdkEventAny* event)
{
    // do not allow closing the window
    return true;
}