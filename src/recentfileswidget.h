#ifndef RECENTFILESWIDGET_H
#define RECENTFILESWIDGET_H

#include <QWidget>

namespace Ui {
class RecentFilesWidget;
}

class RecentFilesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RecentFilesWidget(QWidget *parent = 0);
    ~RecentFilesWidget();

private:
    Ui::RecentFilesWidget *ui;
};

#endif // RECENTFILESWIDGET_H
