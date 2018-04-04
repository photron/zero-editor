//
// Zero Editor
// Copyright (C) 2015-2018 Matthias Bolte <matthias.bolte@googlemail.com>
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

#include "editorcolors.h"

// Use a QPalette pointer here to avoid potential static initialization order problems
QPalette *EditorColors::s_basicPalette = NULL;
QColor EditorColors::s_currentLineHighlightColor;
QColor EditorColors::s_innerWrapMarkerColor;
QColor EditorColors::s_outerWrapMarkerColor;
QColor EditorColors::s_infoBackgroundColor;

// static
void EditorColors::initialize()
{
    s_basicPalette = new QPalette; // FIXME: This leaks memory

    // Ensure that the (window) text is black
    s_basicPalette->setColor(QPalette::WindowText, Qt::black);
    s_basicPalette->setColor(QPalette::Text, Qt::black);

    // Ensure that highlighted text does not turn gray if widget is not in focus
    s_basicPalette->setColor(QPalette::Inactive, QPalette::Highlight,
                             s_basicPalette->color(QPalette::Highlight));
    s_basicPalette->setColor(QPalette::Inactive, QPalette::HighlightedText,
                             s_basicPalette->color(QPalette::HighlightedText));

    // Calculate current line highlight color (formula taken from Qt Creator 3.5.0)
    QColor forground = s_basicPalette->color(QPalette::Highlight);
    QColor background = s_basicPalette->color(QPalette::Base);
    qreal smallRatio = 0.3; // 0.05 for search scope
    qreal largeRatio = 0.6; // 0.4 for search scope
    qreal ratio = ((s_basicPalette->color(QPalette::Text).value() < 128) ^
                   (s_basicPalette->color(QPalette::HighlightedText).value() < 128)) ? smallRatio : largeRatio;

    s_currentLineHighlightColor = QColor::fromRgbF(forground.redF()   * ratio + background.redF()   * (1.0 - ratio),
                                                   forground.greenF() * ratio + background.greenF() * (1.0 - ratio),
                                                   forground.blueF()  * ratio + background.blueF()  * (1.0 - ratio));
    s_currentLineHighlightColor.setAlpha(128);

    s_innerWrapMarkerColor = QColor(194, 235, 194);
    s_outerWrapMarkerColor = QColor(235, 194, 194);
    s_infoBackgroundColor = QColor(255, 255, 225);
}
