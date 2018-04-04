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

#include "style.h"

#include <QDebug>
#include <QPainter>
#include <QStyleOptionComplex>

Style::Style(QStyle *style) :
    QProxyStyle(style)
{
}

QIcon Style::standardIcon(StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const
{
    // Override default line edit clear button icon
    if (standardIcon == QStyle::SP_LineEditClearButton) {
        return QIcon(":/icons/14x14/clear.png");
    } else {
        return QProxyStyle::standardIcon(standardIcon, option, widget);
    }
}

void Style::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter,
                          const QWidget *widget) const
{
#ifdef Q_OS_WIN
    // Make the item view selection rect always cover the whole width of the item view to match the normal Windows
    // style better. Without this the selection rect starts some pixels left of the item bounding box. This looks ugly
    // in tree views, as it creates any empty space left of the item.
    if (element == PE_PanelItemViewItem) {
        const_cast<QStyleOption *>(option)->rect.setLeft(0);
        painter->setClipRect(option->rect, Qt::ReplaceClip);
    }
#endif

    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

void Style::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter,
                               const QWidget *widget) const
{
    // Strip menu style flag for QToolButton because the menu indicator overlaps the text
    if (control == QStyle::CC_ToolButton) {
        static_cast<QStyleOptionToolButton *>(const_cast<QStyleOptionComplex *>(option))->features &= ~QStyleOptionToolButton::HasMenu;
    }

    QProxyStyle::drawComplexControl(control, option, painter, widget);
}
