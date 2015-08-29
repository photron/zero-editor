
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
        qDebug() << font().family() << "is not monospace";
    } else {
        qDebug() << font().family() << "is monospace";
    }
}
