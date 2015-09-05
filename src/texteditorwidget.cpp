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

#include "texteditorwidget.h"

#include "editorcolors.h"
#include "monospacefontmetrics.h"

#include <QCoreApplication>
#include <QPainter>
#include <QTextBlock>

class TextEditorExtraArea : public QWidget
{
public:
    TextEditorExtraArea(TextEditorWidget *editor) :
        QWidget(editor),
        m_editor(editor)
    {
        setFont(MonospaceFontMetrics::font());
        setPalette(EditorColors::basicPalette());
        setAutoFillBackground(true);
    }

    QSize sizeHint() const
    {
        return QSize(m_editor->extraAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event)
    {
        m_editor->extraAreaPaintEvent(event);
    }

    void wheelEvent(QWheelEvent *event)
    {
        QCoreApplication::sendEvent(m_editor->viewport(), event);
    }

private:
    TextEditorWidget *m_editor;
};

TextEditorWidget::TextEditorWidget(QWidget *parent) :
    QPlainTextEdit(parent),
    m_extraArea(new TextEditorExtraArea(this)),
    m_lastCursorBlockNumber(-1),
    m_lastCursorPositionInBlock(-1),
    m_lastCursorSelectionStart(-1),
    m_lastCursorSelectionEnd(-1),
    m_highlightCurrentLine(false)
{
    setFont(MonospaceFontMetrics::font());
    setPalette(EditorColors::basicPalette());
    setCursorWidth(2);

    // Show tabs and spaces and set tab size to 4 spaces
    QTextOption option = document()->defaultTextOption();

    option.setFlags(option.flags() | QTextOption::ShowTabsAndSpaces);
    option.setTabStop(MonospaceFontMetrics::charWidth() * 4);

    document()->setDefaultTextOption(option);

    connect(this, &QPlainTextEdit::updateRequest, this, &TextEditorWidget::redrawExtraAreaRect);
    connect(this, &QPlainTextEdit::blockCountChanged, this, &TextEditorWidget::updateExtraAreaWidth);
    connect(this, &QPlainTextEdit::selectionChanged, this, &TextEditorWidget::updateExtraAreaSelectionHighlight);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &TextEditorWidget::updateCurrentLineHighlight);

    updateExtraAreaWidth();
    updateCurrentLineHighlight();
}

int TextEditorWidget::extraAreaWidth() const
{
    int digits = 1;
    int maximum = blockCount();

    while (maximum >= 10) {
        maximum /= 10;
        ++digits;
    }

    return 8 + MonospaceFontMetrics::charWidth() * digits + 8;
}

void TextEditorWidget::extraAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_extraArea);
    int extraAreaWidth = m_extraArea->width();
    int selectionStart = textCursor().selectionStart();
    int selectionEnd = textCursor().selectionEnd();
    QTextBlock textCursorBlock = textCursor().block();
    QTextBlock block = firstVisibleBlock();
    qreal top = blockBoundingGeometry(block).translated(contentOffset()).top();
    qreal height = blockBoundingRect(block).height();
    qreal bottom = top + height;

    painter.setPen(m_extraArea->palette().color(QPalette::WindowText));

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            // Highlight the line containing the cursor
            if (m_highlightCurrentLine && block == textCursorBlock) {
                QRectF lineRect = block.layout()->lineForTextPosition(textCursor().positionInBlock()).rect();

                lineRect.moveTop(top + lineRect.top());
                lineRect.setLeft(0);
                lineRect.setRight(extraAreaWidth);

                painter.fillRect(lineRect, EditorColors::currentLineHighlightColor());
            }

            // Highlight selected line number
            bool selected = (selectionStart != selectionEnd) &&
                            (selectionStart < block.position() + block.length() && selectionEnd >= block.position());

            if (selected) {
                painter.setPen(m_extraArea->palette().color(QPalette::Highlight).darker(100));
            }

            // Draw line number
            painter.drawText(QRectF(0, top, extraAreaWidth - 8, height), Qt::AlignRight,
                             QString::number(block.blockNumber() + 1));

            // Draw dots for the additional lines in wrapped blocks
            int blockLineCount = block.lineCount();

            if (blockLineCount > 1) {
                qreal lineHeight = height / blockLineCount;

                for (int i = 1; i < blockLineCount; ++i) {
                    painter.drawText(QRectF(0, top + lineHeight * i, extraAreaWidth - 8, lineHeight),
                                     Qt::AlignRight, "\u00B7");
                }
            }

            // Reset text color
            if (selected) {
                painter.setPen(m_extraArea->palette().color(QPalette::WindowText));
            }
        }

        block = block.next();
        top = bottom;
        height = blockBoundingRect(block).height();
        bottom = top + height;
    }
}

// protected
void TextEditorWidget::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    QRect rect = contentsRect();

    rect.setWidth(extraAreaWidth());

    m_extraArea->setGeometry(rect);
}

// protected
void TextEditorWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());
    QPointF offset = contentOffset();
    QTextBlock textCursorBlock = textCursor().block();
    QRect er = event->rect();
    QRect viewportRect = viewport()->rect();
    bool editable = !isReadOnly();
    QTextBlock block = firstVisibleBlock();

    // Set a brush origin so that the WaveUnderline knows where the wave started
    painter.setBrushOrigin(offset);

    // Draw 80 column wrap marker
    int marker = MonospaceFontMetrics::charWidth() * 80 + offset.x() + document()->documentMargin();

    if (marker < viewportRect.width()) {
        painter.setPen(EditorColors::innerWrapMarkerColor());
        painter.drawLine(QPointF(marker, er.top()), QPointF(marker, er.bottom()));

        // Draw 120 column wrap marker
        marker = MonospaceFontMetrics::charWidth() * 120 + offset.x() + document()->documentMargin();

        if (marker < viewportRect.width()) {
            painter.setPen(EditorColors::outerWrapMarkerColor());
            painter.drawLine(QPointF(marker, er.top()), QPointF(marker, er.bottom()));
        }
    }

    QAbstractTextDocumentLayout::PaintContext context = getPaintContext();

    painter.setPen(palette().color(QPalette::Text));

    while (block.isValid()) {
        QRectF r = blockBoundingRect(block).translated(offset);
        QTextLayout *layout = block.layout();

        if (!block.isVisible()) {
            offset.ry() += r.height();
            block = block.next();
            continue;
        }

        if (r.bottom() >= er.top() && r.top() <= er.bottom()) {
            QVector<QTextLayout::FormatRange> selectionFormats;
            int blockPosition = block.position();
            int blockLength = block.length();

            for (int i = 0; i < context.selections.size(); ++i) {
                QAbstractTextDocumentLayout::Selection selection = context.selections.at(i);
                int selectionStart = selection.cursor.selectionStart() - blockPosition;
                int selectionEnd = selection.cursor.selectionEnd() - blockPosition;

                if (selectionStart < blockLength && selectionEnd > 0 && selectionEnd > selectionStart) {
                    QTextLayout::FormatRange selectionFormat;

                    selectionFormat.start = selectionStart;
                    selectionFormat.length = selectionEnd - selectionStart;
                    selectionFormat.format = selection.format;

                    selectionFormats.append(selectionFormat);
                }
            }

            if (m_highlightCurrentLine && block == textCursorBlock) {
                QRectF lineRect = layout->lineForTextPosition(textCursor().positionInBlock()).rect();

                lineRect.moveTop(r.top() + lineRect.top());
                lineRect.setLeft(0);
                lineRect.setRight(viewportRect.width() - offset.x());

                if (!editable && !er.contains(lineRect.toRect())) {
                    QRect updateRect = er;
                    updateRect.setLeft(0);
                    updateRect.setRight(viewportRect.width() - offset.x());
                    viewport()->update(updateRect);
                }

                painter.fillRect(lineRect, EditorColors::currentLineHighlightColor());
            }

            bool drawCursor = ((editable || (textInteractionFlags() & Qt::TextSelectableByKeyboard))
                               && context.cursorPosition >= blockPosition
                               && context.cursorPosition < blockPosition + blockLength);

            layout->draw(&painter, offset, selectionFormats, er);

            if (drawCursor
                    || (editable && context.cursorPosition < -1
                        && !layout->preeditAreaText().isEmpty())) {
                int cursorPosition = context.cursorPosition;

                if (cursorPosition < -1) {
                    cursorPosition = layout->preeditAreaPosition() - (cursorPosition + 2);
                } else {
                    cursorPosition -= blockPosition;
                }

                layout->drawCursor(&painter, offset, cursorPosition, cursorWidth());
            }
        }

        offset.ry() += r.height();

        if (offset.y() > viewportRect.height()) {
            break;
        }

        block = block.next();
    }
}

// protected
void TextEditorWidget::focusInEvent(QFocusEvent *event)
{
    m_highlightCurrentLine = true;

    // Redraw cursor line highlight rect to show it
    redrawLineInBlock(textCursor().blockNumber(), textCursor().positionInBlock());

    QPlainTextEdit::focusInEvent(event);
}

// protected
void TextEditorWidget::focusOutEvent(QFocusEvent *event)
{
    m_highlightCurrentLine = false;

    // Redraw cursor line highlight rect to clear it
    redrawLineInBlock(textCursor().blockNumber(), textCursor().positionInBlock());

    QPlainTextEdit::focusOutEvent(event);
}

// private slot
void TextEditorWidget::redrawExtraAreaRect(const QRect &rect, int dy)
{
    if (dy != 0) {
        m_extraArea->scroll(0, dy);
    } else {
        // Project QPlainTextEdit::updateRequest() calls onto the full extra area width
        m_extraArea->update(0, rect.y(), m_extraArea->width(), rect.height());
    }
}

// private slot
void TextEditorWidget::updateExtraAreaWidth()
{
    setViewportMargins(extraAreaWidth(), 0, 0, 0);
}

// private slot
void TextEditorWidget::updateExtraAreaSelectionHighlight()
{
    // Redraw the now deselected lines to clear them and redraw the now selected lines to show them
    int selectionStart = textCursor().selectionStart();
    int selectionEnd = textCursor().selectionEnd();

    // Calculate (de-)selected section before the unchanged selection section
    int beforeStart = qMin(m_lastCursorSelectionStart, selectionStart);
    int beforeEnd;

    if (m_lastCursorSelectionStart <= selectionStart) {
        beforeEnd = qMin(m_lastCursorSelectionEnd, selectionStart);
    } else {
        beforeEnd = qMin(m_lastCursorSelectionStart, selectionEnd);
    }

    // Calculate (de-)selected section after the unchanged selection section
    int afterStart;

    if (m_lastCursorSelectionEnd <= selectionEnd) {
        afterStart = qMax(m_lastCursorSelectionEnd, selectionStart);
    } else {
        afterStart = qMax(m_lastCursorSelectionStart, selectionEnd);
    }

    int afterEnd = qMax(m_lastCursorSelectionEnd, selectionEnd);

    // Redraw non-empty ranges
    if (beforeStart != beforeEnd) {
        redrawExtraAreaBlockRange(beforeStart, beforeEnd);
    }

    if (afterStart != afterEnd) {
        redrawExtraAreaBlockRange(afterStart, afterEnd);
    }

    m_lastCursorSelectionStart = selectionStart;
    m_lastCursorSelectionEnd = selectionEnd;
}

// private slot
void TextEditorWidget::updateCurrentLineHighlight()
{
    // Redraw last cursor line highlight rect to clear it
    if (m_lastCursorBlockNumber >= 0 && m_lastCursorPositionInBlock >= 0) {
        redrawLineInBlock(m_lastCursorBlockNumber, m_lastCursorPositionInBlock);
    }

    // Redraw current cursor line highlight rect to show it
    int blockNumber = textCursor().blockNumber();
    int positionInBlock = textCursor().positionInBlock();

    redrawLineInBlock(blockNumber, positionInBlock);

    m_lastCursorBlockNumber = blockNumber;
    m_lastCursorPositionInBlock = positionInBlock;
}

// private
void TextEditorWidget::redrawLineInBlock(int blockNumber, int positionInBlock)
{
    QTextBlock block = document()->findBlockByNumber(blockNumber);

    if (!block.isValid()) {
        return;
    }

    // The block bounding geometry is in content coordinates. After translating it with the content offset the block
    // rect is in viewport coordinates.
    QRect blockRect = blockBoundingGeometry(block).translated(contentOffset()).toAlignedRect();

    // The line rect is relative to the containing block
    QRect lineRect = block.layout()->lineForTextPosition(positionInBlock).rect().toAlignedRect();

    // Translate the line rect to viewport coordinates and make it full width
    lineRect.moveTop(blockRect.top() + lineRect.top());
    lineRect.setLeft(0);
    lineRect.setRight(INT_MAX);

    viewport()->update(lineRect);
    m_extraArea->update(lineRect);
}

// private
void TextEditorWidget::redrawExtraAreaBlockRange(int fromPosition, int toPosition)
{
    QPointF offset = contentOffset();
    QTextBlock block = document()->findBlock(fromPosition);
    QTextBlock lastBlock = document()->findBlock(toPosition);

    while (block.isValid()) {
        m_extraArea->update(blockBoundingGeometry(block).translated(offset).toAlignedRect());

        if (block == lastBlock) {
            break;
        }

        block = block.next();
    }
}
