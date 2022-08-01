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

#include "HddChooser.h"

#include "utils/TitleParser.h"
#include "utils/IconParser.h"
#include <string>

HddChooser::HddChooser(Glib::RefPtr<Gtk::Builder> builder)
{
    this->builder = builder;

    builder->get_widget("chooseHddWindow", chooseHddWindow);
    chooseHddWindow->set_default_icon_list(IconParser::getIcons());

    Gtk::ImageMenuItem *aboutItem = nullptr;
    builder->get_widget("hddChooserAbout", aboutItem);
    aboutItem->signal_activate().connect(sigc::mem_fun(*this, &HddChooser::on_about_click));

    Gtk::Button *button = nullptr;
    builder->get_widget("chooseHddOkButton", button);
    button->signal_clicked().connect(sigc::mem_fun(*this, &HddChooser::on_chooseHddOkButton_clicked));

    builder->get_widget("chooseHddComboBox", hdds);

    HddModelColumns columns;
    treeModel = Gtk::ListStore::create(columns);
    hdds->set_model(treeModel);

    on_chooseHddRefreshButton_clicked();

    hdds->pack_start(columns.name);
    hdds->pack_start(columns.size);

    Gtk::Button *refreshButton = nullptr;
    builder->get_widget("chooseHddRefreshButton", refreshButton);
    refreshButton->signal_clicked().connect(sigc::mem_fun(*this, &HddChooser::on_chooseHddRefreshButton_clicked));

    builder->get_widget("chooseHddOtpFileChooser", otpButton);

    builder->get_widget("chooseHddSeepromFileChoser", seepromButton);
}

HddChooser::~HddChooser()
{
}

void HddChooser::on_chooseHddOkButton_clicked()
{
    std::string otpPath = otpButton->get_filename();
    std::string seepromPath = seepromButton->get_filename();

    if (otpPath.empty() || seepromPath.empty())
    {
        return;
    }

    Gtk::TreeModel::Row row = *(hdds->get_active());
    DiskUtils::Disk disk = disks[row[columns.index]];

    std::unique_ptr<OTP> otp;
    std::unique_ptr<SEEPROM> seeprom;
    try
    {
        otp.reset(OTP::LoadFromFile(otpPath));
        seeprom.reset(SEEPROM::LoadFromFile(seepromPath));
    }
    catch (const std::exception &e)
    {
        Gtk::MessageDialog errDialog(*chooseHddWindow, "Error", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, false);
        errDialog.set_secondary_text(e.what());
        errDialog.run();
        return;
    }

    std::vector<std::byte> key = seeprom->GetUSBKey(*otp);
    std::shared_ptr<FileDevice> device = nullptr;
    try
    {
        device = std::make_shared<FileDevice>(disk.deviceId);
        Wfs::DetectDeviceSectorSizeAndCount(device, key);
    }
    catch (const std::exception &e)
    {
        Gtk::MessageDialog errDialog(*chooseHddWindow, "Error", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, false);
        if (std::string(e.what()).compare("Unexpected WFS version (bad key?)") == 0)
        {
            errDialog.set_secondary_text("Not a WFS Device or bad key");
        }
        else if (std::string(e.what()).compare("FileDevice: Failed to open file") == 0)
        {
            errDialog.set_secondary_text("Cannot open disk");
        }
        else
        {
            errDialog.set_secondary_text(e.what());
        }
        errDialog.run();
        return;
    }

    std::vector<TitleParser::TitleInfo> infos = TitleParser::getTitleInfos(device, key);
    list = new GameList(builder, infos, device, key);

    chooseHddWindow->get_application()->add_window(*list->getWindow());
    chooseHddWindow->hide();
}

void HddChooser::on_chooseHddRefreshButton_clicked()
{
    treeModel->clear();

    disks = DiskUtils::getDisks();
    for (unsigned int i = 0; i < disks.size(); i++)
    {
        Gtk::TreeModel::Row row = *(treeModel->append());
        row[columns.index] = i;
        row[columns.name] = disks[i].name;
        row[columns.size] = disks[i].size;

        // set the first mount active
        if (i == 0)
            hdds->set_active(row);
    }
}

void HddChooser::on_about_click()
{
    builder->get_widget("aboutWindow", aboutDialog);

#ifdef _WIN32
    Gtk::Button *closeButton = (Gtk::Button *)aboutDialog->get_widget_for_response(GTK_RESPONSE_DELETE_EVENT);
    aboutConn = closeButton->signal_clicked().connect(sigc::mem_fun(*this, &HddChooser::on_aboutClose_click));
#endif

    aboutDialog->show();
}

void HddChooser::on_aboutClose_click()
{
    aboutDialog->hide();
    aboutConn.disconnect();
}