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