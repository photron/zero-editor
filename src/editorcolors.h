//
// Zero Editor
// Copyright (C) 2015 Matthias Bolte <matthias.bolte@googlemail.com>
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

#ifndef EDITORCOLORS_H
#define EDITORCOLORS_H

#include <QColor>
#include <QPalette>

class EditorColors
{
public:
    static void initialize();

    static QPalette basicPalette() { return *s_basicPalette; }
    static QColor currentLineHighlightColor() { return s_currentLineHighlightColor; }
    static QColor innerWrapMarkerColor() { return s_innerWrapMarkerColor; }
    static QColor outerWrapMarkerColor() { return s_outerWrapMarkerColor; }

private:
    EditorColors();

    static QPalette *s_basicPalette;
    static QColor s_currentLineHighlightColor;
    static QColor s_innerWrapMarkerColor;
    static QColor s_outerWrapMarkerColor;
};

#endif // EDITORCOLORS_H
