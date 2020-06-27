#include "IconParser.h"

std::vector<Glib::RefPtr<Gdk::Pixbuf>> IconParser::getIcons()
{
    std::vector<Glib::RefPtr<Gdk::Pixbuf>> icons;

    icons.push_back(Gdk::Pixbuf::create_from_resource("/dumpsteru/icons/16.png"));
    icons.push_back(Gdk::Pixbuf::create_from_resource("/dumpsteru/icons/24.png"));
    icons.push_back(Gdk::Pixbuf::create_from_resource("/dumpsteru/icons/32.png"));
    icons.push_back(Gdk::Pixbuf::create_from_resource("/dumpsteru/icons/48.png"));
    icons.push_back(Gdk::Pixbuf::create_from_resource("/dumpsteru/icons/64.png"));
    icons.push_back(Gdk::Pixbuf::create_from_resource("/dumpsteru/icons/96.png"));
    icons.push_back(Gdk::Pixbuf::create_from_resource("/dumpsteru/icons/128.png"));
    icons.push_back(Gdk::Pixbuf::create_from_resource("/dumpsteru/icons/256.png"));

    return icons;
}