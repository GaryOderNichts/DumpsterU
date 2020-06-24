#include "HddChooser.h"

#include "utils/DiskUtils.h"
#include "utils/TitleParser.h"
#include <string>

HddChooser::HddChooser(Glib::RefPtr<Gtk::Builder> builder)
{
    this->builder = builder;

    builder->get_widget("chooseHddWindow", chooseHddWindow);

    Gtk::ImageMenuItem* aboutItem = nullptr;
    builder->get_widget("hddChooserAbout", aboutItem);
    aboutItem->signal_activate().connect(sigc::mem_fun(*this, &HddChooser::on_about_click));

    Gtk::Button* button = nullptr;
    builder->get_widget("chooseHddOkButton", button);
    button->signal_clicked().connect(sigc::mem_fun(*this, &HddChooser::on_chooseHddOkButton_clicked));

    builder->get_widget("chooseHddComboBox", hdds);

    HddModelColumns columns;
    treeModel = Gtk::ListStore::create(columns);
    hdds->set_model(treeModel);

    on_chooseHddRefreshButton_clicked();

    hdds->pack_start(columns.name);
    hdds->pack_start(columns.size);

    Gtk::Button* refreshButton = nullptr;
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
    Glib::ustring id = row[columns.name];

#ifdef _WIN32
    std::string devName = id;
#else
    std::string devName = std::string("/dev/") + id;
#endif

    std::unique_ptr<OTP> otp;
    std::unique_ptr<SEEPROM> seeprom;
    try
    {
        otp.reset(OTP::LoadFromFile(otpPath));
        seeprom.reset(SEEPROM::LoadFromFile(seepromPath));
    }
    catch(const std::exception& e)
    {
        Gtk::MessageDialog errDialog(*chooseHddWindow, "Error", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, false);
        errDialog.set_secondary_text(e.what());
        errDialog.run();
        return;
    }

    std::vector<uint8_t> key = seeprom->GetUSBKey(*otp);
    std::shared_ptr<FileDevice>  device = std::make_shared<FileDevice>(devName);
    try
    {
        Wfs::DetectDeviceSectorSizeAndCount(device, key);
    }
    catch(const std::exception& e)
    {   
        Gtk::MessageDialog errDialog(*chooseHddWindow, "Error", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, false);
        if (std::string(e.what()).compare("Unexpected WFS version (bad key?)") == 0)
        {
            errDialog.set_secondary_text("Not a WFS Device or bad key");
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

    std::vector<std::string> disks = DiskUtils::getDisks();
    for (unsigned int i = 0; i < disks.size(); i++)
    {
        Gtk::TreeModel::Row row = *(treeModel->append());
        row[columns.name] = disks[i];

        // set the first mount active
        if (i == 0)
            hdds->set_active(row);
    }
}

void HddChooser::on_about_click()
{
    Gtk::AboutDialog* aboutDialog = nullptr;
    builder->get_widget("aboutWindow", aboutDialog);
    aboutDialog->run();
}