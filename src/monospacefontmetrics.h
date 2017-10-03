//
// Zero Editor
// Copyright (C) 2015-2017 Matthias Bolte <matthias.bolte@googlemail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef MONOSPACEFONTMETRICS_H
#define MONOSPACEFONTMETRICS_H

#include <QFont>

class MonospaceFontMetrics
{
public:
    static void initialize();

    static QFont font() { return *s_font; }
    static int charWidth() { return s_charWidth; }
    static int lineHeight() { return s_lineHeight; }

private:
    MonospaceFontMetrics();

    static QFont *s_font;
    static int s_charWidth;
    static int s_lineHeight;
};

#endif // MONOSPACEFONTMETRICS_H
