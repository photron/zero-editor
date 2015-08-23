#ifndef FINDINFILESWIDGET_H
#define FINDINFILESWIDGET_H

#include <QWidget>

namespace Ui {
class FindInFilesWidget;
}

class FindInFilesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FindInFilesWidget(QWidget *parent = NULL);
    ~FindInFilesWidget();

    void installLineEditEventFilter(QObject *filter);

    void prepareForShow();

signals:
    void hideClicked();

private:
    Ui::FindInFilesWidget *ui;
};

#endif // FINDINFILESWIDGET_H
