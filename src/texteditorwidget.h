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

#ifndef TEXTEDITORWIDGET_H
#define TEXTEDITORWIDGET_H

#include <QPlainTextEdit>

class TextEditorExtraArea;

class TextEditorWidget : public QPlainTextEdit
{
    Q_OBJECT

public:
    TextEditorWidget(QWidget *parent = NULL);

    int extraAreaWidth() const;
    void extraAreaPaintEvent(QPaintEvent *event);

protected:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);

private slots:
    void redrawExtraAreaRect(const QRect &rect, int dy);
    void updateExtraAreaWidth();
    void updateExtraAreaSelectionHighlight();
    void updateCurrentLineHighlight();

private:
    void redrawLineInBlock(int blockNumber, int positionInBlock);
    void redrawExtraAreaBlockRange(int fromPosition, int toPosition);

    TextEditorExtraArea *m_extraArea;
    int m_lastCursorBlockNumber;
    int m_lastCursorPositionInBlock;
    int m_lastCursorSelectionStart;
    int m_lastCursorSelectionEnd;
    bool m_highlightCurrentLine;
};

#endif // TEXTEDITORWIDGET_H
