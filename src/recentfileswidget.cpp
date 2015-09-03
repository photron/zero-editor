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

#include "recentfileswidget.h"
#include "ui_recentfileswidget.h"

#include <QMenu>

RecentFilesWidget::RecentFilesWidget(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::RecentFilesWidget)
{
    m_ui->setupUi(this);

    QMenu *menuMode = new QMenu();

    menuMode->addAction("Recently Viewed Files");
    menuMode->addAction("Recently Opened Files");
    menuMode->addAction("Recently Closed Files");

    m_ui->buttonMode->setMenu(menuMode);
}

RecentFilesWidget::~RecentFilesWidget()
{
    delete m_ui;
}
