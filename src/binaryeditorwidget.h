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

#ifndef BINARYEDITORWIDGET_H
#define BINARYEDITORWIDGET_H

#include <QAbstractScrollArea>
#include <QBasicTimer>
#include <QByteArray>

class BinaryEditorExtraArea;

class BinaryEditorWidget : public QAbstractScrollArea
{
    Q_OBJECT

public:
    BinaryEditorWidget(QWidget *parent = NULL);

    QByteArray data() const;
    void setData(const QByteArray &data);

    int extraAreaWidth() const;
    void extraAreaPaintEvent(QPaintEvent *event);

protected:
    void scrollContentsBy(int dx, int dy);
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);
    void timerEvent(QTimerEvent *event);

private:
    enum {
        BytesPerLine = 16,
        HexColumnsPerLine = BytesPerLine * 3 - 1 // Including the interior whitespace
    };

    enum MoveMode {
        KeepAnchor,
        MoveAnchor
    };

    void updateScrollBarRanges();
    void updateLines(int fromPosition, int toPosition);
    void updateCursorLine();
    void setBlinkingCursorEnabled(bool enable);
    int positionAt(const QPoint &position, bool *inHexSection) const;
    void setCursorPosition(int position, MoveMode moveMode);
    void ensureCursorVisible();

    BinaryEditorExtraArea *m_extraArea;
    QByteArray m_data;
    int m_lineCount;
    int m_lastVerticalScrollBarValue;
    bool m_cursorVisible;
    bool m_cursorInHexSection;
    int m_cursorPosition;
    int m_anchorPosition;
    QBasicTimer m_cursorBlinkTimer;
    int m_documentMargin; // FIXME: Replace with BinaryDocument::documentMargin
    bool m_highlightCurrentLine;
};

#endif // BINARYEDITORWIDGET_H
