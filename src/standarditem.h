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

#ifndef STANDARDITEM_H
#define STANDARDITEM_H

#include <QStandardItem>

class StandardItem : public QStandardItem
{
public:
    StandardItem();
    explicit StandardItem(const QString &text);
    StandardItem(const QIcon &icon, const QString &text);
    explicit StandardItem(int rows, int columns = 1);

    void setFontUnderline(bool underline);
};

#endif // STANDARDITEM_H
