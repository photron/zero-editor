#ifndef OPENFILESWIDGET_H
#define OPENFILESWIDGET_H

#include <QWidget>

namespace Ui {
class OpenFilesWidget;
}

class OpenFilesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OpenFilesWidget(QWidget *parent = 0);
    ~OpenFilesWidget();

    void installLineEditEventFilter(QObject *filter);

private:
    Ui::OpenFilesWidget *ui;
};

#endif // OPENFILESWIDGET_H
