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

#include "bookmarkswidget.h"
#include "ui_bookmarkswidget.h"

BookmarksWidget::BookmarksWidget(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::BookmarksWidget)
{
    m_ui->setupUi(this);

    // FIXME: this widget will only be visible if there are any bookmarks. the remove all button will remove all
    // bookmarks and therefore hide the widget as well
}

BookmarksWidget::~BookmarksWidget()
{
    delete m_ui;
}
