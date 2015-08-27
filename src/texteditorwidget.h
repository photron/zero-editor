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

private slots:
    void updateExtraArea(const QRect &rect, int dy);
    void updateExtraAreaWidth();
    void updateExtraAreaSelectionHighlight();
    void updateCurrentLineHighlight();

private:
    TextEditorExtraArea *extraArea;
    int lastCursorBlockNumber;
    int lastCursorSelectionStart;
    QColor currentLineHighlightColor;
};

#endif // TEXTEDITORWIDGET_H
