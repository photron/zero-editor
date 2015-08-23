
#include "recentfileswidget.h"
#include "ui_recentfileswidget.h"

#include <QMenu>

RecentFilesWidget::RecentFilesWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RecentFilesWidget)
{
    ui->setupUi(this);

    QMenu *menuMode = new QMenu();

    menuMode->addAction("Recently Viewed Files");
    menuMode->addAction("Recently Opened Files");
    menuMode->addAction("Recently Closed Files");

    ui->buttonMode->setMenu(menuMode);
}

RecentFilesWidget::~RecentFilesWidget()
{
    delete ui;
}
