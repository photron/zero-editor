#ifndef STYLE_H
#define STYLE_H

#include <QProxyStyle>

class Style : public QProxyStyle
{
public:
    Style(QStyle *style);

    QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *option = NULL, const QWidget *widget = NULL) const;
    void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget = NULL) const;
};

#endif // STYLE_H
