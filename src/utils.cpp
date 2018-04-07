//
// Zero Editor
// Copyright (C) 2018 Matthias Bolte <matthias.bolte@googlemail.com>
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

#include "utils.h"

#include <QAbstractButton>
#include <QFont>
#include <QListWidgetItem>
#include <QStandardItem>

namespace Utils {

void setFontUnderline(QAbstractButton *button, bool underline)
{
    QFont font(button->font());

    font.setUnderline(underline);

    button->setFont(font);
}

void setFontUnderline(QListWidgetItem *item, bool underline)
{
    QFont font(item->font());

    font.setUnderline(underline);

    item->setFont(font);
}

void setFontUnderline(QStandardItem *item, bool underline)
{
    QFont font(item->font());

    font.setUnderline(underline);

    item->setFont(font);
}

}
