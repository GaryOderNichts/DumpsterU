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

#include <gtkmm.h>

#include "menu/HddChooser.h"

#ifdef _WIN32
    #include <windows.h>
#endif

int main(int argc, char** argv)
{
#ifdef _WIN32
    FreeConsole();
    if (AttachConsole(ATTACH_PARENT_PROCESS))
    {
        AllocConsole();
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }
#endif
    Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "com.garyodernichts.dumpsteru");
    Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_resource("/dumpsteru/ui/dumpsteru.ui");

    Glib::RefPtr<Gtk::CssProvider> css = Gtk::CssProvider::create();
    css->load_from_resource("/dumpsteru/ui/custom.css");
    Gtk::StyleContext::add_provider_for_screen(Gdk::Screen::get_default(), css, 1000);

    HddChooser* chooser = new HddChooser(builder);

    app->run(*chooser->getWindow());

    delete chooser->getList();
    delete chooser;

    return 0;
}