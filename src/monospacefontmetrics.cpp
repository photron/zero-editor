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

#include "monospacefontmetrics.h"

#include <QDebug>
#include <QFontMetricsF>

qreal MonospaceFontMetrics::m_charWidth = 0;
qreal MonospaceFontMetrics::m_lineSpacing = 0;

// static
void MonospaceFontMetrics::initialize()
{
    QFontMetricsF metrics(font());

    m_charWidth = metrics.width('x');
    m_lineSpacing = metrics.lineSpacing();

    QString ascii = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

    if (metrics.width(ascii) != m_charWidth * ascii.length()) {
        qDebug() << font().family() << "is NOT monospace, char width" << m_charWidth;
    } else {
        qDebug() << font().family() << "is monospace, char width" << m_charWidth;
    }
}
