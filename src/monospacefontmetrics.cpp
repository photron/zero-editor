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

#include "monospacefontmetrics.h"

#include <QDebug>
#include <QFontMetrics>

// Use a QFont pointer here to avoid potential static initialization order problems
QFont *MonospaceFontMetrics::s_font = NULL;
int MonospaceFontMetrics::s_charWidth = 0;
int MonospaceFontMetrics::s_lineHeight = 0;

// static
void MonospaceFontMetrics::initialize()
{
    s_font = new QFont("DejaVu Sans Mono", 10); // FIXME: This leaks memory

    // Force integer metrics, otherwise the DejaVu Sans Mono font at size 10pt will have a fractional char width of
    // 7.8125px on Linux. It has an integer char width of exactly 8px on Windows. That fractional char width will
    // result in error prone calculations including the need for correct rounding to get pixel perfect results. Just
    // avoid all of this trouble by forcing an integer char width of exactly 8px on Linux as well.
    s_font->setStyleStrategy(QFont::ForceIntegerMetrics);

    QFontMetrics metrics(*s_font);

    s_charWidth = metrics.width('x');
    s_lineHeight = metrics.height();

    QString ascii = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

    qDebug() << "MonospaceFontMetrics: Monospace:" << (metrics.width(ascii) == s_charWidth * ascii.length());
    qDebug() << "MonospaceFontMetrics: Char Width:" << s_charWidth;
    qDebug() << "MonospaceFontMetrics: Height:" << s_lineHeight;
    qDebug() << "MonospaceFontMetrics: Ascent:" << metrics.ascent();
    qDebug() << "MonospaceFontMetrics: Descent:" << metrics.descent();
    qDebug() << "MonospaceFontMetrics: Leading:" << metrics.leading();
    qDebug() << "MonospaceFontMetrics: Line Spacing:" << metrics.lineSpacing();
}
