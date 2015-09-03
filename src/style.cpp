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

#include "style.h"

#include <QStyleOptionComplex>

Style::Style(QStyle *style) :
    QProxyStyle(style)
{
}

QPixmap Style::standardPixmap(StandardPixmap standardPixmap, const QStyleOption *option, const QWidget *widget) const
{
    // override default line-edit-clear-button icon
    if (standardPixmap == QStyle::SP_LineEditClearButton) {
        return QPixmap(":/icons/14x14/clear.png");
    } else {
        return QProxyStyle::standardPixmap(standardPixmap, option, widget);
    }
}

void Style::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    // strip menu style flag for QToolButton because the menu indicator overlaps the text
    if (control == QStyle::CC_ToolButton) {
        static_cast<QStyleOptionToolButton *>(const_cast<QStyleOptionComplex *>(option))->features &= ~QStyleOptionToolButton::HasMenu;
    }

    QProxyStyle::drawComplexControl(control,  option, painter, widget);
}
