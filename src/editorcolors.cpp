/*
 * Zero Editor
 * Copyright (C) 2015 Matthias Bolte <matthias.bolte@googlemail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "editorcolors.h"

#include <QPalette>

QColor EditorColors::m_currentLineHighlight;

// static
void EditorColors::initialize()
{
    QPalette palette;

    // Calculate current line highlight color (formula taken from Qt Creator 3.5.0)
    QColor forground = palette.color(QPalette::Highlight);
    QColor background = palette.color(QPalette::Base);
    qreal smallRatio = 0.3; // 0.05 for search scope
    qreal largeRatio = 0.6; // 0.4 for search scope
    qreal ratio = ((palette.color(QPalette::Text).value() < 128) ^
                   (palette.color(QPalette::HighlightedText).value() < 128)) ? smallRatio : largeRatio;

    m_currentLineHighlight = QColor::fromRgbF(forground.redF()   * ratio + background.redF()   * (1.0 - ratio),
                                              forground.greenF() * ratio + background.greenF() * (1.0 - ratio),
                                              forground.blueF()  * ratio + background.blueF()  * (1.0 - ratio));
    m_currentLineHighlight.setAlpha(128);
}

