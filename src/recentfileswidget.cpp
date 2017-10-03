//
// Zero Editor
// Copyright (C) 2015-2017 Matthias Bolte <matthias.bolte@googlemail.com>
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

#include "recentfileswidget.h"
#include "ui_recentfileswidget.h"

#include <QActionGroup>
#include <QMenu>

RecentFilesWidget::RecentFilesWidget(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::RecentFilesWidget)
{
    m_ui->setupUi(this);

    m_ui->widgetOpenedFiles->setOption(FilesWidget::KeepAfterClose, true);
    m_ui->widgetViewedFiles->setOption(FilesWidget::KeepAfterClose, true);
    m_ui->widgetModifiedFiles->setOption(FilesWidget::KeepAfterClose, true);
    m_ui->widgetClosedFiles->setOption(FilesWidget::KeepAfterClose, true);

    QMenu *menuMode = new QMenu;

    m_actionOpened = menuMode->addAction("Recently Opened Files");
    m_actionViewed = menuMode->addAction("Recently Viewed Files");
    m_actionModified = menuMode->addAction("Recently Modified Files");
    m_actionClosed = menuMode->addAction("Recently Closed Files");

    m_actionOpened->setCheckable(true);
    m_actionViewed->setCheckable(true);
    m_actionModified->setCheckable(true);
    m_actionClosed->setCheckable(true);

    connect(m_actionOpened, &QAction::triggered, this, &RecentFilesWidget::showOpenedFiles);
    connect(m_actionViewed, &QAction::triggered, this, &RecentFilesWidget::showViewedFiles);
    connect(m_actionModified, &QAction::triggered, this, &RecentFilesWidget::showModifiedFiles);
    connect(m_actionClosed, &QAction::triggered, this, &RecentFilesWidget::showClosedFiles);

    m_ui->buttonMode->setMenu(menuMode);

    m_actionGroup = new QActionGroup(this);

    m_actionGroup->addAction(m_actionOpened);
    m_actionGroup->addAction(m_actionViewed);
    m_actionGroup->addAction(m_actionModified);
    m_actionGroup->addAction(m_actionClosed);

    connect(m_ui->stackedWidget, &QStackedWidget::currentChanged, this, &RecentFilesWidget::updateModeMenuAndTitle);

    showViewedFiles();

    // FIXME: the recently viewed mode always shows the currently viewed document at the top. dbl clicking an item
    // in the list makes it the currently view document and moves the item to the top of the list. the go to previous
    // file action makes the second item in the list the currently viewed document and moves it to the top of the list.
    // this makes the go to previous file action toggle between the two most recently viewed documents.
    //
    // the recently opened mode shows files in the order they were opened. closing and reopening a file moves it to the
    // top of the list
    //
    // the recently closed mode shows files in the order they were closed. reopening and closing a file moves it to the
    // top of the list
    //
    // all three list have a maximum length of 100 items
}

RecentFilesWidget::~RecentFilesWidget()
{
    delete m_ui;
}

// private slot
void RecentFilesWidget::showOpenedFiles()
{
    m_ui->stackedWidget->setCurrentWidget(m_ui->widgetOpenedFiles);
}

// private slot
void RecentFilesWidget::showViewedFiles()
{
    m_ui->stackedWidget->setCurrentWidget(m_ui->widgetViewedFiles);
}

// private slot
void RecentFilesWidget::showModifiedFiles()
{
    m_ui->stackedWidget->setCurrentWidget(m_ui->widgetModifiedFiles);
}

// private slot
void RecentFilesWidget::showClosedFiles()
{
    m_ui->stackedWidget->setCurrentWidget(m_ui->widgetClosedFiles);
}

// private slot
void RecentFilesWidget::updateModeMenuAndTitle()
{
    QWidget *currentWidget = m_ui->stackedWidget->currentWidget();

    if (currentWidget == m_ui->widgetOpenedFiles) {
        m_actionOpened->setChecked(true);
        m_ui->labelTitle->setText(m_actionOpened->text());
    } else if (currentWidget == m_ui->widgetViewedFiles) {
        m_actionViewed->setChecked(true);
        m_ui->labelTitle->setText(m_actionViewed->text());
    } else if (currentWidget == m_ui->widgetModifiedFiles) {
        m_actionModified->setChecked(true);
        m_ui->labelTitle->setText(m_actionModified->text());
    } else if (currentWidget == m_ui->widgetClosedFiles) {
        m_actionClosed->setChecked(true);
        m_ui->labelTitle->setText(m_actionClosed->text());
    }
}
