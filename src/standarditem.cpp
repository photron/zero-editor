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

#include "standarditem.h"

StandardItem::StandardItem()
{
}

StandardItem::StandardItem(const QString &text) :
    QStandardItem(text)
{
}

StandardItem::StandardItem(const QIcon &icon, const QString &text) :
    QStandardItem(icon, text)
{
}

StandardItem::StandardItem(int rows, int columns) :
    QStandardItem(rows, columns)
{
}

void StandardItem::setFontUnderline(bool underline)
{
    QFont font_(font());

    font_.setUnderline(underline);

    setFont(font_);
}
