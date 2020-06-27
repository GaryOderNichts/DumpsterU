#pragma once

#include <vector>
#include <gtkmm.h>

class IconParser
{
public:
    static std::vector<Glib::RefPtr<Gdk::Pixbuf>> getIcons();
};