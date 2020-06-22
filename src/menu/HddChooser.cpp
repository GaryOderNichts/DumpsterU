#include "HddChooser.h"

#include "utils/DiskUtils.h"

HddChooser::HddChooser(Glib::RefPtr<Gtk::Builder> builder)
{
    this->builder = builder;

    builder->get_widget("chooseHddWindow", chooseHddWindow);

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

    chooseHddWindow->close();

    Gtk::TreeModel::Row row = *(hdds->get_active());
    Glib::ustring id = row[columns.name];
    std::string devName = id;

    list = new GameList(builder, otpPath, seepromPath, std::string("/dev/") + devName);
    chooseHddWindow->get_application()->add_window(*list->getWindow());
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