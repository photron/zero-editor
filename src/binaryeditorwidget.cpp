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

#include "binaryeditorwidget.h"

#include "editorcolors.h"
#include "monospacefontmetrics.h"

#include <QtMath>
#include <QApplication>
#include <QClipboard>
#include <QPainter>
#include <QPaintEvent>
#include <QScrollBar>
#include <QTextDocument>
#include <QWheelEvent>

class BinaryEditorExtraArea : public QWidget
{
public:
    BinaryEditorExtraArea(BinaryEditorWidget *editor) :
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
    BinaryEditorWidget *m_editor;
};

BinaryEditorWidget::BinaryEditorWidget(QWidget *parent) :
    QAbstractScrollArea(parent),
    m_extraArea(new BinaryEditorExtraArea(this)),
    m_lineCount(1),
    m_cursorVisible(false),
    m_cursorInHexSection(true),
    m_cursorAtLowNibble(false),
    m_cursorPosition(0),
    m_anchorPosition(0),
    m_lastVerticalScrollBarValue(-1),
    m_documentMargin(QTextDocument(this).documentMargin()),
    m_highlightCurrentLine(false)
{
    setFont(MonospaceFontMetrics::font());
    setPalette(EditorColors::basicPalette());
    updateScrollBarRanges();
    setViewportMargins(extraAreaWidth(), 0, 0, 0);

    QByteArray d("cons\0t int topLine = verticalScrollBar()->value();   const int xoffset = horizontalScrollBar()->value();            const int x1 = -xoffset + m_margin + m_labelWidth - m_charWidth/2 - 1;            const int x2 = -xoffset + m_margin + m_labelWidth + m_bytesPerLine * m_columnWidth + m_charWidth/2;            painter.drawLine(x1, 0, x1, viewport()->height());            painter.drawLine(x2, 0, x2, viewport()->height());", 0x165);

    setData(d); // FIXME: deal with special case of empty data
}

QByteArray BinaryEditorWidget::data() const
{
    return m_data;
}

void BinaryEditorWidget::setData(const QByteArray &data)
{
    m_data = data;
    m_lineCount = m_data.length() / BytesPerLine + 1;

    updateScrollBarRanges();
}

void BinaryEditorWidget::undo()
{

}

void BinaryEditorWidget::redo()
{

}

void BinaryEditorWidget::copy()
{
    int selectionStart = qMin(m_anchorPosition, m_cursorPosition);
    int selectionEnd = qMax(m_anchorPosition, m_cursorPosition);
    int selectionLength = selectionEnd - selectionStart + 1;
    QByteArray selectedData = m_data.mid(selectionStart, selectionLength);

    if (m_cursorInHexSection) {
        const char *hexDigits= "0123456789ABCDEF";
        QString hexString;

        hexString.reserve(3 * selectedData.size());

        for (int i = 0; i < selectedData.size(); ++i) {
            quint8 byte = selectedData.at(i);

            hexString += hexDigits[(byte >> 4) & 0x0F];
            hexString += hexDigits[byte & 0x0F];
            hexString += ' ';
        }

        hexString.chop(1);

        QApplication::clipboard()->setText(hexString);
    } else {
        QString printableString;

        printableString.reserve(selectedData.size());

        for (int i = 0; i < selectedData.size(); ++i) {
            quint8 byte = selectedData.at(i);

            if (byte >= 1 && byte <= 126) {
                printableString += (char)byte;
            } else {
                printableString += ' ';
            }
        }

        QApplication::clipboard()->setText(printableString);
    }
}

void BinaryEditorWidget::selectAll()
{
    setCursorPosition(0, MoveAnchor);
    setCursorPosition(m_data.length() - 1, KeepAnchor);
}

int BinaryEditorWidget::extraAreaWidth() const
{
    int digits = 8;

    return 8 + MonospaceFontMetrics::charWidth() * digits + 8;
}

void BinaryEditorWidget::extraAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_extraArea);
    int extraAreaWidth = m_extraArea->width();
    int selectionStart;
    int selectionEnd;

    if (m_cursorPosition >= m_anchorPosition) {
        selectionStart = m_anchorPosition;
        selectionEnd = m_cursorPosition;
    } else {
        selectionStart = m_cursorPosition;
        selectionEnd = m_anchorPosition;
    }

    int line = verticalScrollBar()->value();
    int lineHeight = MonospaceFontMetrics::lineHeight();
    int top = 0;

    // If the first line is visible then offset it by the document margin to mimic the QPlainTextEdit margin behavior.
    if (line == 0) {
        top += m_documentMargin;
    }

    int bottom = top + lineHeight;

    painter.setPen(m_extraArea->palette().color(QPalette::WindowText));

    while (line < m_lineCount && top <= event->rect().bottom()) {
        if (bottom >= event->rect().top()) {
            int linePosition = BytesPerLine * line;

            // Highlight the line containing the cursor
            if (m_highlightCurrentLine && m_cursorPosition >= linePosition
                    && m_cursorPosition < linePosition + BytesPerLine) {
                painter.fillRect(QRect(0, top, extraAreaWidth, lineHeight), EditorColors::currentLineHighlightColor());
            }

            // Highlight selected line number
            bool selected = selectionStart < linePosition + BytesPerLine && selectionEnd >= linePosition;

            if (selected) {
                painter.setPen(m_extraArea->palette().color(QPalette::Highlight).darker(100));
            }

            // Draw line number
            painter.drawText(QRect(0, top, extraAreaWidth - 8, lineHeight), Qt::AlignRight,
                             QString::asprintf("%08X", line * BytesPerLine));

            // Reset text color
            if (selected) {
                painter.setPen(m_extraArea->palette().color(QPalette::WindowText));
            }
        }

        ++line;
        top = bottom;
        bottom = top + lineHeight;
    }
}

// protected
void BinaryEditorWidget::scrollContentsBy(int dx, int dy)
{
    Q_UNUSED(dx)

    // The vertical scroll bar operates in lines, but the scroll function operates in pixels. Multiply dy by the line
    // spacing to convert between the two.
    dy *= MonospaceFontMetrics::lineHeight();

    // Check if the scroll operation resulted in showing/hiding the first line. If that is the case then an extra top
    // margin of 4px (see QTextDocument::documentMargin) is added/subtracted to mimic the document margin of the
    // QPlainTextEdit.
    int verticalScrollBarValue = verticalScrollBar()->value();

    if (verticalScrollBarValue == 0 && m_lastVerticalScrollBarValue != 0) {
        dy += m_documentMargin;
    } else if (verticalScrollBarValue != 0 && m_lastVerticalScrollBarValue == 0) {
        dy -= m_documentMargin;
    }

    m_lastVerticalScrollBarValue = verticalScrollBarValue;

    viewport()->scroll(dx, dy);
    m_extraArea->scroll(0, dy);
}

// protected
bool BinaryEditorWidget::event(QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent == QKeySequence::Undo
                || keyEvent == QKeySequence::Redo
                || keyEvent == QKeySequence::Copy
                || keyEvent == QKeySequence::SelectAll) {
            keyEvent->accept();
        }
    }

    return QAbstractScrollArea::event(event);
}

// protected
void BinaryEditorWidget::resizeEvent(QResizeEvent *event)
{
    QAbstractScrollArea::resizeEvent(event);

    QRect rect = contentsRect();

    rect.setWidth(extraAreaWidth());

    m_extraArea->setGeometry(rect);

    updateScrollBarRanges();
}

// protected
void BinaryEditorWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());
    int charWidth = MonospaceFontMetrics::charWidth();
    int lineHeight = MonospaceFontMetrics::lineHeight();
    int selectionStart;
    int selectionEnd;

    if (m_cursorPosition >= m_anchorPosition) {
        selectionStart = m_anchorPosition;
        selectionEnd = m_cursorPosition;
    } else {
        selectionStart = m_cursorPosition;
        selectionEnd = m_anchorPosition;
    }

    int leftHex = m_documentMargin - horizontalScrollBar()->value();
    int leftPrintable = leftHex + (HexColumnsPerLine + 1) * charWidth + 1 + charWidth;
    int right = viewport()->width() + horizontalScrollBar()->value() - m_documentMargin;
    int line = verticalScrollBar()->value();
    int top = 0;

    // If the first line is visible then offset it by the document margin to mimic the QPlainTextEdit margin behavior.
    if (line == 0) {
        top += m_documentMargin;
    }

    int bottom = top + lineHeight;
    QString hexString(HexColumnsPerLine, ' ');
    QChar *hexChars = hexString.data();
    const char *hexDigits= "0123456789ABCDEF";
    QString printableString(BytesPerLine, '_');
    QChar *printableChars = printableString.data();

    // Draw divider line between hex amd printable section
    int divider = leftHex + (HexColumnsPerLine + 1) * charWidth;

    if (divider < viewport()->width()) {
        painter.setPen(palette().color(QPalette::Midlight));
        painter.drawLine(divider, event->rect().top(), divider, event->rect().bottom());
    }

    painter.setPen(palette().color(QPalette::Text));

    while (line < m_lineCount && top <= event->rect().bottom()) {
        if (bottom >= event->rect().top()) {
            int linePosition = BytesPerLine * line;
            bool cursorInLine = m_cursorPosition >= linePosition && m_cursorPosition < linePosition + BytesPerLine;
            QRect hexRect(leftHex, top, HexColumnsPerLine * charWidth, lineHeight);
            QRect printableRect(leftPrintable, top, BytesPerLine * charWidth, lineHeight);

            // Highlight the line containing the cursor
            if (m_highlightCurrentLine && cursorInLine) {
                painter.fillRect(QRect(0, top, right + m_documentMargin, lineHeight),
                                 EditorColors::currentLineHighlightColor());
            }

            // Highlight selected bytes
            int fullWidthSelection = 0;
            int hexSelectionLeft = -1;
            int hexSelectionRight = -1;
            int printableSelectionLeft = -1;
            int printableSelectionRight = -1;

            if (selectionStart < linePosition) {
                // Selection starts before this line
                hexSelectionLeft = hexRect.left();
                printableSelectionLeft = printableRect.left();
                ++fullWidthSelection;
            } else if (selectionStart >= linePosition && selectionStart < linePosition + BytesPerLine) {
                // Selection starts in this line
                int offset = (selectionStart % BytesPerLine) * charWidth;

                hexSelectionLeft = hexRect.left() + offset * 3;
                printableSelectionLeft = printableRect.left() + offset;
            }

            if (selectionEnd >= linePosition + BytesPerLine) {
                // Selection ends after this line, +1 because right() returns the last position INSIDE the QRect
                hexSelectionRight = hexRect.right() + 1;
                printableSelectionRight = printableRect.right() + 1;
                ++fullWidthSelection;
            } else if (selectionEnd >= linePosition && selectionEnd < linePosition + BytesPerLine) {
                // Selection ends in this line
                int offset = ((selectionEnd % BytesPerLine) + 1) * charWidth;

                hexSelectionRight = hexRect.left() + qMax(offset * 3 - charWidth, 0);
                printableSelectionRight = printableRect.left() + offset;
            }

            // Prepare hex nibbles and printable text
            for (int i = 0; i < BytesPerLine; ++i) {
                int offset = line * BytesPerLine + i;

                if (offset < m_data.length()) {
                    quint8 byte = m_data.at(offset);

                    hexChars[i * 3] = hexDigits[(byte >> 4) & 0x0F];
                    hexChars[i * 3 + 1] = hexDigits[byte & 0x0F];

                    if (byte >= 32 && byte <= 126) {
                        printableChars[i] = (char)byte;
                    } else {
                        printableChars[i] = 0x00B7;
                    }
                } else {
                    hexChars[i * 3] = ' ';
                    hexChars[i * 3 + 1] = ' ';
                    printableChars[i] = ' ';
                }
            }

            // Draw non-fully selected lines
            if (fullWidthSelection < 2) {
                painter.drawText(hexRect, hexString);
                painter.drawText(printableRect, printableString);
            }

            // Draw selected lines
            if (hexSelectionLeft >= 0 && hexSelectionRight >= 0) {
                QRect selectionRect(hexSelectionLeft, top, hexSelectionRight - hexSelectionLeft, lineHeight);

                painter.save();
                painter.fillRect(selectionRect, palette().color(QPalette::Highlight));
                painter.setPen(palette().color(QPalette::HighlightedText));
                painter.setClipRect(selectionRect);
                painter.drawText(hexRect, hexString);
                painter.restore();
            }

            if (printableSelectionLeft >= 0 && printableSelectionRight >= 0) {
                QRect selectionRect(printableSelectionLeft, top,
                                    printableSelectionRight - printableSelectionLeft, lineHeight);

                painter.save();
                painter.fillRect(selectionRect, palette().color(QPalette::Highlight));
                painter.setPen(palette().color(QPalette::HighlightedText));
                painter.setClipRect(selectionRect);
                painter.drawText(printableRect, printableString);
                painter.restore();
            }

            if (m_cursorVisible && cursorInLine) {
                int offset = (m_cursorPosition % BytesPerLine) * charWidth;

                // Draw hex cursor
                QRect hexCursorRect;

                if (m_cursorAtLowNibble) {
                    hexCursorRect = QRect(hexRect.left() + offset * 3 + charWidth, top, charWidth, lineHeight);
                } else {
                    hexCursorRect = QRect(hexRect.left() + offset * 3, top, charWidth * 2, lineHeight);
                }

                painter.save();

                if (m_cursorInHexSection) {
                    painter.fillRect(hexCursorRect, Qt::black);
                    painter.setPen(palette().color(QPalette::HighlightedText));
                    painter.setClipRect(hexCursorRect);
                    painter.drawText(hexRect, hexString);
                } else {
                    painter.setPen(Qt::black);
                    painter.drawRect(hexCursorRect.adjusted(0, 0, -1, -1));
                }

                painter.restore();

                // Draw printable cursor
                QRect printableCursorRect(printableRect.left() + offset, top, charWidth, lineHeight);

                painter.save();

                if (!m_cursorInHexSection) {
                    painter.fillRect(printableCursorRect, Qt::black);
                    painter.setPen(palette().color(QPalette::HighlightedText));
                    painter.setClipRect(printableCursorRect);
                    painter.drawText(printableRect, printableString);
                } else {
                    painter.setPen(Qt::black);
                    painter.drawRect(printableCursorRect.adjusted(0, 0, -1, -1));
                }

                painter.restore();
            }
        }

        ++line;
        top = bottom;
        bottom = top + lineHeight;
    }
}

// protected
void BinaryEditorWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        return;
    }

    setCursorPosition(positionAt(event->pos(), &m_cursorInHexSection), MoveAnchor);
}

// protected
void BinaryEditorWidget::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) == 0) {
        return;
    }

    setCursorPosition(positionAt(event->pos(), &m_cursorInHexSection), KeepAnchor);
}

// protected
void BinaryEditorWidget::mouseReleaseEvent(QMouseEvent *event)
{
}

// protected
void BinaryEditorWidget::keyPressEvent(QKeyEvent *event)
{
    if (event == QKeySequence::Undo) {
        event->accept();
        undo();

        return;
    } else if (event == QKeySequence::Redo) {
        event->accept();
        redo();

        return;
    } else if (event == QKeySequence::Copy) {
        event->accept();
        copy();

        return;
    } else if (event == QKeySequence::SelectAll) {
        event->accept();
        selectAll();

        return;
    }

    MoveMode moveMode = event->modifiers() & Qt::ShiftModifier ? KeepAnchor : MoveAnchor;
    bool ctrlPressed = event->modifiers() & Qt::ControlModifier;
    int line;
    int position;

    switch (event->key()) {
    case Qt::Key_Up:
        if (ctrlPressed) {
            verticalScrollBar()->triggerAction(QScrollBar::SliderSingleStepSub);
        } else if (m_cursorPosition - BytesPerLine >= 0) {
            setCursorPosition(m_cursorPosition - BytesPerLine, moveMode);
        }

        break;

    case Qt::Key_Down:
        if (ctrlPressed) {
            verticalScrollBar()->triggerAction(QScrollBar::SliderSingleStepAdd);
        } else if (m_cursorPosition + BytesPerLine < m_data.length()) {
            setCursorPosition(m_cursorPosition + BytesPerLine, moveMode);
        }

        break;

    case Qt::Key_Right:
        setCursorPosition(m_cursorPosition + 1, moveMode);
        break;

    case Qt::Key_Left:
        setCursorPosition(m_cursorPosition - 1, moveMode);
        break;

    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
        // FIXME: does not jet jump to the start and end of the document
        line = qMax(m_cursorPosition / BytesPerLine - verticalScrollBar()->value(), 0);

        verticalScrollBar()->triggerAction(event->key() == Qt::Key_PageUp ? QScrollBar::SliderPageStepSub
                                                                          : QScrollBar::SliderPageStepAdd);

        if (!ctrlPressed) {
            setCursorPosition((verticalScrollBar()->value() + line) * BytesPerLine + m_cursorPosition % BytesPerLine,
                              moveMode);
        }

        break;

    case Qt::Key_Home:
        if (ctrlPressed) {
            position = 0;
        } else {
            position = m_cursorPosition - (m_cursorPosition % BytesPerLine);
        }

        setCursorPosition(position, moveMode);

        break;

    case Qt::Key_End:
        if (ctrlPressed) {
            position = m_data.length() - 1;
        } else {
            position = m_cursorPosition / BytesPerLine * BytesPerLine + (BytesPerLine - 1);
        }

        setCursorPosition(position, moveMode);

        break;

    default:
        QString text = event->text();

        for (int i = 0; i < text.length(); ++i) {
            QChar c = text.at(i);

            if (m_cursorInHexSection) {
                c = c.toLower();

                int nibble = -1;

                if (c.unicode() >= 'a' && c.unicode() <= 'f') {
                    nibble = c.unicode() - 'a' + 10;
                } else if (c.unicode() >= '0' && c.unicode() <= '9') {
                    nibble = c.unicode() - '0';
                }

                if (nibble < 0) {
                    continue;
                }

                if (m_cursorAtLowNibble) {
                    m_data[m_cursorPosition] = nibble + (m_data.at(m_cursorPosition) & 0xF0);
                    m_cursorAtLowNibble = false;

                    setCursorPosition(m_cursorPosition + 1, MoveAnchor);
                } else {
                    m_data[m_cursorPosition] = (nibble << 4) + (m_data.at(m_cursorPosition) & 0x0F);
                    m_cursorAtLowNibble = true;

                    redrawCursorLine();
                }
            } else {
                if (c.unicode() >= 128 || !c.isPrint()) {
                    continue;
                }

                m_data[m_cursorPosition] = c.unicode();

                setCursorPosition(m_cursorPosition + 1, MoveAnchor);
            }
        }
    }

    event->accept();
}

// protected
void BinaryEditorWidget::focusInEvent(QFocusEvent *event)
{
    setBlinkingCursorEnabled(true);

    m_highlightCurrentLine = true;

    redrawCursorLine();

    QAbstractScrollArea::focusInEvent(event);
}

// protected
void BinaryEditorWidget::focusOutEvent(QFocusEvent *event)
{
    setBlinkingCursorEnabled(false);

    m_highlightCurrentLine = false;

    redrawCursorLine();

    QAbstractScrollArea::focusOutEvent(event);
}

// protected
void BinaryEditorWidget::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_cursorBlinkTimer.timerId()) {
        m_cursorVisible = !m_cursorVisible;

        redrawCursorLine();
    }

    QAbstractScrollArea::timerEvent(event);
}

// private
void BinaryEditorWidget::updateScrollBarRanges()
{
    int charWidth = MonospaceFontMetrics::charWidth();
    int lineHeight = MonospaceFontMetrics::lineHeight();

    // Mimic the logic QPlainTextWidget uses to calculate the visible line count. QPlainTextWidget basically takes the
    // viewport height, subtracts the document top/bottom margins (defaults to 4px, see QTextDocument::documentMargin),
    // sutracts 1px (to avoid that the last line could touch the widget bottom) and then calcualtes how many lines fit
    // into the remaining height.
    int visibleLineCount = qMax(viewport()->height() - m_documentMargin * 2 - 1, 0) / lineHeight;
    int contentWidth = (HexColumnsPerLine + 1) * charWidth + 1 + (1 + BytesPerLine) * charWidth;

    horizontalScrollBar()->setRange(0, contentWidth + m_documentMargin * 2 - viewport()->width());
    horizontalScrollBar()->setPageStep(viewport()->width());

    verticalScrollBar()->setRange(0, m_lineCount - visibleLineCount);
    verticalScrollBar()->setPageStep(visibleLineCount);
    //ensureCursorVisible(); // FIXME

    m_lastVerticalScrollBarValue = verticalScrollBar()->value();
}

// private
void BinaryEditorWidget::redrawLines(int fromPosition, int toPosition)
{
    int lineHeight = MonospaceFontMetrics::lineHeight();
    int line = verticalScrollBar()->value();
    int firstLine = qMin(fromPosition, toPosition) / BytesPerLine;
    int lastLine = qMax(fromPosition, toPosition) / BytesPerLine;
    int y = (firstLine - line) * lineHeight;
    int height = (lastLine - firstLine + 1) * lineHeight;

    // If the first line is visible then offset it by the document margin to mimic the QPlainTextEdit margin behavior.
    if (line == 0) {
        y += m_documentMargin;
    }

    viewport()->update(0, y, viewport()->width(), height);
    m_extraArea->update(0, y, extraAreaWidth(), height);
}

// private
void BinaryEditorWidget::redrawCursorLine()
{
    redrawLines(m_cursorPosition, m_cursorPosition);
}

// private
void BinaryEditorWidget::setBlinkingCursorEnabled(bool enable)
{
    if (enable && QApplication::cursorFlashTime() > 0) {
        m_cursorBlinkTimer.start(QApplication::cursorFlashTime() / 2, this);
    } else {
        m_cursorBlinkTimer.stop();
    }

    m_cursorVisible = enable;

    redrawCursorLine();
}

// private
int BinaryEditorWidget::positionAt(const QPoint &position, bool *inHexSection) const
{
    int charWidth = MonospaceFontMetrics::charWidth();
    int lineHeight = MonospaceFontMetrics::lineHeight();
    int maxPosition = m_data.length() - 1;

    // Calculate x relative to the left edge of the first hex column
    int x = position.x() + horizontalScrollBar()->value() - m_documentMargin;

    // Check if x is in the hex section
    *inHexSection = x < (HexColumnsPerLine + 1) * charWidth;

    // Calculate line relative to the top edge of the first hex line. Use qFloor, because truncation would round
    // towards zero which would produce a wrong result if the position is in the line immediatly above the first line.
    int line = verticalScrollBar()->value() + qFloor((position.y() - m_documentMargin) / (float)lineHeight);

    // Check if position is before the first or after the last line
    if (line < 0) {
        return 0;
    } else if (line >= m_lineCount) {
        return maxPosition;
    }

    if (*inHexSection) {
        // Use qFloor, because truncation would round towards zero which would produce a wrong result if the position
        // is in the column immediatly left to the first hex column.
        int hexColumn = qBound(0, qFloor((x + (float)charWidth / 2) / (charWidth * 3)), BytesPerLine - 1);

        return qMin(BytesPerLine * line + hexColumn, maxPosition);
    } else {
        // Shift x to calculate printable column relative to the left edge of the first printable column
        x -= BytesPerLine * 3 * charWidth + 1 + charWidth;

        // Use qFloor, because truncation would round towards zero which would produce a wrong result if the position
        // is in the column immediatly left to the first printable column.
        int printableColumn = qBound(0, qFloor(x / (float)charWidth), BytesPerLine - 1);

        return qMin(BytesPerLine * line + printableColumn, maxPosition);
    }
}

// private
void BinaryEditorWidget::setCursorPosition(int position, MoveMode moveMode)
{
    int lastCursorPosition = m_cursorPosition;

    m_cursorAtLowNibble = false;
    m_cursorPosition = qBound(0, position, m_data.length() - 1);

    if (moveMode == MoveAnchor) {
        redrawLines(m_anchorPosition, lastCursorPosition);

        m_anchorPosition = m_cursorPosition;
    }

    redrawLines(lastCursorPosition, m_cursorPosition);
    ensureCursorVisible();
}

// private
void BinaryEditorWidget::ensureCursorVisible()
{
    int charWidth = MonospaceFontMetrics::charWidth();
    int line = verticalScrollBar()->value();
    int lineHeight = MonospaceFontMetrics::lineHeight();
    int y = (m_cursorPosition / BytesPerLine - line) * lineHeight;
    int leftHex = m_documentMargin - horizontalScrollBar()->value();
    int leftPrintable = leftHex + (HexColumnsPerLine + 1) * charWidth + 1 + charWidth;
    int offset = (m_cursorPosition % BytesPerLine) * charWidth;
    int visibleLineCount = qMax(viewport()->height() - m_documentMargin * 2 - 1, 0) / lineHeight;
    QRect cursorRect;

    // If the first line is visible then offset it by the document margin to mimic the QPlainTextEdit margin behavior.
    if (line == 0) {
        y += m_documentMargin;
    }

    if (m_cursorInHexSection) {
        cursorRect = QRect(leftHex + offset * 3, y, charWidth * 2, lineHeight);
    } else {
        cursorRect = QRect(leftPrintable + offset, y, charWidth, lineHeight);
    }

    QRect viewportRect = viewport()->rect();

    // FIXME: need to handle horizontal scrolling
    if (!viewportRect.contains(cursorRect)) {
        if (cursorRect.top() < viewportRect.top()) {
            verticalScrollBar()->setValue(m_cursorPosition / BytesPerLine);
        } else if (cursorRect.bottom() > viewportRect.bottom()) {
            verticalScrollBar()->setValue(m_cursorPosition / BytesPerLine - visibleLineCount + 1);
        }
    }
}
