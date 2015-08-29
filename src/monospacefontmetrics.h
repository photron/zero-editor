#ifndef MONOSPACEFONTMETRICS_H
#define MONOSPACEFONTMETRICS_H

#include <QFont>

class MonospaceFontMetrics
{
public:
    static void initialize();

    static QFont font() { return QFont("DejaVu Sans Mono", 10); }
    static qreal charWidth() { return m_charWidth; }
    static qreal lineSpacing() { return m_lineSpacing; }

private:
    static qreal m_charWidth;
    static qreal m_lineSpacing;
};

#endif // MONOSPACEFONTMETRICS_H
