
#include "texteditorwidget.h"

#include <QCoreApplication>
#include <QPainter>
#include <QTextBlock>

class TextEditorExtraArea : public QWidget
{
public:
    TextEditorExtraArea(TextEditorWidget *editor) :
        QWidget(editor),
        editor(editor)
    {
        QPalette palette;

        palette.setColor(QPalette::HighlightedText, palette.color(QPalette::WindowText));
        palette.setColor(QPalette::WindowText, palette.color(QPalette::Dark));

        setPalette(palette);
        setAutoFillBackground(true);
        setFont(QFont("DejaVu Sans Mono", 10));
    }

    QSize sizeHint() const
    {
        return QSize(editor->extraAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event)
    {
        editor->extraAreaPaintEvent(event);
    }

    void wheelEvent(QWheelEvent *event)
    {
        QCoreApplication::sendEvent(editor->viewport(), event);
    }

private:
    TextEditorWidget *editor;
};

TextEditorWidget::TextEditorWidget(QWidget *parent) :
    QPlainTextEdit(parent),
    extraArea(new TextEditorExtraArea(this)),
    lastCursorBlockNumber(-1),
    lastCursorSelectionStart(-1)
{
    setFont(QFont("DejaVu Sans Mono", 10));

    // Ensure that the text is black
    QPalette palette;

    palette.setColor(QPalette::Text, Qt::black); // FIXME: Why is this necessary on Linux?

    setPalette(palette);

    // Calculate current line highlight color (formula taken from Qt Creator 3.5.0)
    QColor forground = palette.color(QPalette::Highlight);
    QColor background = palette.color(QPalette::Base);
    qreal smallRatio = 0.3; // 0.05 for search scope
    qreal largeRatio = 0.6; // 0.4 for search scope
    qreal ratio = ((palette.color(QPalette::Text).value() < 128) ^
                   (palette.color(QPalette::HighlightedText).value() < 128)) ? smallRatio : largeRatio;

    currentLineHighlightColor = QColor::fromRgbF(forground.redF()   * ratio + background.redF()   * (1.0 - ratio),
                                                 forground.greenF() * ratio + background.greenF() * (1.0 - ratio),
                                                 forground.blueF()  * ratio + background.blueF()  * (1.0 - ratio));
    currentLineHighlightColor.setAlpha(128);

    // Show tabs and spaces
    QTextOption option = document()->defaultTextOption();

    option.setFlags(option.flags() | QTextOption::ShowTabsAndSpaces);

    document()->setDefaultTextOption(option);

    connect(this, &QPlainTextEdit::updateRequest, this, &TextEditorWidget::updateExtraArea);
    connect(this, &QPlainTextEdit::blockCountChanged, this, &TextEditorWidget::updateExtraAreaWidth);
    connect(this, &QPlainTextEdit::selectionChanged, this, &TextEditorWidget::updateExtraAreaSelectionHighlight);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &TextEditorWidget::updateCurrentLineHighlight);

    updateExtraAreaWidth();
    updateCurrentLineHighlight();
}

void TextEditorWidget::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    QRect rect = contentsRect();

    rect.setWidth(extraAreaWidth());

    extraArea->setGeometry(rect);
}

int TextEditorWidget::extraAreaWidth() const
{
    int digits = 1;
    int maximum = blockCount();

    while (maximum >= 10) {
        maximum /= 10;
        ++digits;
    }

    return 6 + fontMetrics().width('9') * digits + 6;
}

void TextEditorWidget::extraAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(extraArea);
    int extraAreaWidth = extraArea->width();
    int selectionStart = textCursor().selectionStart();
    int selectionEnd = textCursor().selectionEnd();
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    qreal top = blockBoundingGeometry(block).translated(contentOffset()).top();
    qreal height = blockBoundingRect(block).height();
    qreal bottom = top + height;

    painter.setPen(extraArea->palette().color(QPalette::WindowText));

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            bool selected = (selectionStart < block.position() + block.length() && selectionEnd >= block.position()) ||
                            (selectionStart == selectionEnd && selectionStart == block.position());

            if (selected) {
                painter.setPen(extraArea->palette().color(QPalette::HighlightedText));
            }

            painter.drawText(QRectF(0, top, extraAreaWidth - 6, height),
                             Qt::AlignRight, QString::number(blockNumber + 1));

            if (selected) {
                painter.setPen(extraArea->palette().color(QPalette::WindowText));
            }
        }

        block = block.next();
        top = bottom;
        height = blockBoundingRect(block).height();
        bottom = top + height;
        ++blockNumber;
    }
}

// private slot
void TextEditorWidget::updateExtraArea(const QRect &rect, int dy)
{
    if (dy != 0) {
        extraArea->scroll(0, dy);
    } else {
        extraArea->update(0, rect.y(), extraArea->width(), rect.height());
    }

    if (rect.contains(viewport()->rect())) {
        updateExtraAreaWidth();
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
    // FIXME: This is only necessary if blocks can be wrapped
    //if (no block wrapping) {
    //    return;
    //}

    int cursorBlockNumber = textCursor().blockNumber();

    if (cursorBlockNumber != lastCursorBlockNumber) {
        QPointF offset = contentOffset();

        // Force an update on the entire block that currently contains the cursor to ensure that the line number
        // highlight is updated even if the cursor position is not in the first line of that block. Without this the
        // line number for a wrapped block stays non-highlight if the cursor/selection enters it without touching its
        // first line.
        QTextBlock block = document()->findBlockByNumber(cursorBlockNumber);

        if (block.isValid() && block.isVisible()) {
            extraArea->update(blockBoundingGeometry(block).translated(offset).toAlignedRect());
        }

        lastCursorBlockNumber = cursorBlockNumber;

        // Also force an update on the entire block that contained the previous selection start to ensure that the
        // line number highlight is updated even if the previous selection start was not in the first line of that
        // block. Without this the line number for a wrapped block stays highlight if the selection leaves it.
        int cursorSelectionStart = textCursor().selectionStart();

        if (cursorSelectionStart != lastCursorSelectionStart) {
            block = document()->findBlock(lastCursorSelectionStart);

            if (block.isValid()) {
                extraArea->update(blockBoundingGeometry(block).translated(offset).toAlignedRect());
            }

            lastCursorSelectionStart = cursorSelectionStart;
        }
    }
}

// private slot
void TextEditorWidget::updateCurrentLineHighlight()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        selection.format.setBackground(currentLineHighlightColor);
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
