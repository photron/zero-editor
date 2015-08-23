#ifndef FINDANDREPLACEWIDGET_H
#define FINDANDREPLACEWIDGET_H

#include <QWidget>

namespace Ui {
class FindAndReplaceWidget;
}

class FindAndReplaceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FindAndReplaceWidget(QWidget *parent = 0);
    ~FindAndReplaceWidget();

    void installLineEditEventFilter(QObject *filter);

    void prepareForShow();

signals:
    void hideClicked();

private:
    Ui::FindAndReplaceWidget *ui;
};

#endif // FINDANDREPLACEWIDGET_H
