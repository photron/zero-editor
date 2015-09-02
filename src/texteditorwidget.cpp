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
    m_lastCursorSelectionStart(-1)
{
    setFont(MonospaceFontMetrics::font());
    setPalette(EditorColors::basicPalette());
    setCursorWidth(2);

    // Show tabs and spaces and set tab size to 4 spaces
    QTextOption option = document()->defaultTextOption();

    option.setFlags(option.flags() | QTextOption::ShowTabsAndSpaces);
    option.setTabStop(MonospaceFontMetrics::charWidth() * 4);

    document()->setDefaultTextOption(option);

    connect(this, &QPlainTextEdit::updateRequest, this, &TextEditorWidget::updateExtraArea);
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
            if (block == textCursorBlock) {
                QRectF textCursorLine = block.layout()->lineForTextPosition(textCursor().positionInBlock()).rect();

                textCursorLine.translate(0, top);
                textCursorLine.setLeft(0);
                textCursorLine.setRight(extraAreaWidth);

                painter.fillRect(textCursorLine, EditorColors::currentLineHighlightColor());
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

            if (block == textCursorBlock) {
                QRectF rr = layout->lineForTextPosition(textCursor().positionInBlock()).rect();

                rr.translate(0, r.top());
                rr.setLeft(0);
                rr.setRight(viewportRect.width() - offset.x());

                if (!editable && !er.contains(rr.toRect())) {
                    QRect updateRect = er;
                    updateRect.setLeft(0);
                    updateRect.setRight(viewportRect.width() - offset.x());
                    viewport()->update(updateRect);
                }

                painter.fillRect(rr, EditorColors::currentLineHighlightColor());
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

// private slot
void TextEditorWidget::updateExtraArea(const QRect &rect, int dy)
{
    if (dy != 0) {
        m_extraArea->scroll(0, dy);
    } else {
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
    // The following logic is only necessary if blocks can be wrapped
    if (wordWrapMode() == QTextOption::NoWrap) {
        return;
    }

    int cursorBlockNumber = textCursor().blockNumber();

    if (cursorBlockNumber != m_lastCursorBlockNumber) {
        QPointF offset = contentOffset();

        // Force an update on the entire block that currently contains the cursor to ensure that the line number
        // highlight is updated even if the cursor position is not in the first line of that block. Without this the
        // line number for a wrapped block stays non-highlight if the cursor/selection enters it without touching its
        // first line.
        QTextBlock block = document()->findBlockByNumber(cursorBlockNumber);

        if (block.isValid() && block.isVisible()) {
            m_extraArea->update(blockBoundingGeometry(block).translated(offset).toAlignedRect());
        }

        m_lastCursorBlockNumber = cursorBlockNumber;

        // Also force an update on the entire block that contained the previous selection start to ensure that the
        // line number highlight is updated even if the previous selection start was not in the first line of that
        // block. Without this the line number for a wrapped block stays highlight if the selection leaves it.
        int cursorSelectionStart = textCursor().selectionStart();

        if (cursorSelectionStart != m_lastCursorSelectionStart) {
            block = document()->findBlock(m_lastCursorSelectionStart);

            if (block.isValid()) {
                m_extraArea->update(blockBoundingGeometry(block).translated(offset).toAlignedRect());
            }

            m_lastCursorSelectionStart = cursorSelectionStart;
        }
    }
}

// private slot
void TextEditorWidget::updateCurrentLineHighlight()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        selection.format.setBackground(EditorColors::currentLineHighlightColor());
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();

        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);

    // Update the extra area selection highlight to ensure that the line number highlight is updated even if the
    // previous cursor position was not in the first line of that block. Without this the line number for a wrapped
    // block stays highlight if the cursor leaves it.
    updateExtraAreaSelectionHighlight();
}
