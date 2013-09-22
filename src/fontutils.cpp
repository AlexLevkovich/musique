/* $BEGIN_LICENSE

This file is part of Musique.
Copyright 2013, Flavio Tordini <flavio.tordini@gmail.com>

Musique is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Musique is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Musique.  If not, see <http://www.gnu.org/licenses/>.

$END_LICENSE */

#include "fontutils.h"

static const int MIN_PIXEL_SIZE = 11;

const QFont &FontUtils::smaller() {
    static QFont font;
    static bool initialized = false;
    if (!initialized) {
      initialized = true;
      font.setPointSize(font.pointSize()*.85);
      if (font.pixelSize() < MIN_PIXEL_SIZE) font.setPixelSize(MIN_PIXEL_SIZE);
    }
    return font;
}

const QFont &FontUtils::smallerBold() {
    static QFont font;
    static bool initialized = false;
    if (!initialized) {
      initialized = true;
      font.setPointSize(font.pointSize()*.85);
      font.setBold(true);
      if (font.pixelSize() < MIN_PIXEL_SIZE) font.setPixelSize(MIN_PIXEL_SIZE);
    }
    return font;
}

const QFont &FontUtils::bigger() {
    static QFont font;
    static bool initialized = false;
    if (!initialized) {
      initialized = true;
      font.setPointSize(font.pointSize()*1.5);
    }
    return font;
}

const QFont &FontUtils::biggerBold() {
    static QFont font;
    static bool initialized = false;
    if (!initialized) {
      initialized = true;
      font.setPointSize(font.pointSize()*1.5);
      font.setBold(true);
    }
    return font;
}
