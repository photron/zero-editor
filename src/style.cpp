
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
