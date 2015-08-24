
#include "texteditorwidget.h"

#include <QPainter>
#include <QTextBlock>

class TextEditorExtraArea : public QWidget
{
public:
    TextEditorExtraArea(TextEditorWidget *editor) :
        QWidget(editor),
        editor(editor)
    {
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

private:
    TextEditorWidget *editor;
};

TextEditorWidget::TextEditorWidget(QWidget *parent) :
    QPlainTextEdit(parent),
    extraArea(new TextEditorExtraArea(this))
{
    setFont(QFont("DejaVu Sans Mono", 10));

    // Ensure that the text is black
    QPalette p = palette();

    p.setColor(QPalette::Text, Qt::black);
    setPalette(p);

    connect(this, &QPlainTextEdit::blockCountChanged, this, &TextEditorWidget::updateExtraAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &TextEditorWidget::updateExtraArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &TextEditorWidget::highlightCurrentLine);

    updateExtraAreaWidth();
    highlightCurrentLine();
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
    int maximum = qMax(1, blockCount());

    while (maximum >= 10) {
        maximum /= 10;
        ++digits;
    }

    return 3 + fontMetrics().width('9') * digits + 3;
}

void TextEditorWidget::extraAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(extraArea);

    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int)blockBoundingRect(block).height();

    painter.setPen(Qt::black);

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            painter.drawText(0, top, extraArea->width() - 3, fontMetrics().height(),
                             Qt::AlignRight, QString::number(blockNumber + 1));
        }

        block = block.next();
        top = bottom;
        bottom = top + (int)blockBoundingRect(block).height();
        ++blockNumber;
    }
}

// private slot
void TextEditorWidget::updateExtraAreaWidth()
{
    setViewportMargins(extraAreaWidth(), 0, 0, 0);
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
void TextEditorWidget::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor(Qt::yellow).lighter(160);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();

        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}
