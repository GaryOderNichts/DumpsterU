#include <gtkmm.h>

#include "menu/HddChooser.h"

int main(int argc, char** argv)
{
    Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "com.garyodernichts.wfstools");
    Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_resource("/wfstools/ui/wfstools.ui");

    Glib::RefPtr<Gtk::CssProvider> css = Gtk::CssProvider::create();
    css->load_from_resource("/wfstools/ui/custom.css");
    Gtk::StyleContext::add_provider_for_screen(Gdk::Screen::get_default(), css, 1000);

    HddChooser* chooser = new HddChooser(builder);

    app->run(*chooser->getWindow());

    delete chooser->getList();
    delete chooser;

    return 0;
}