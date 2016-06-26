//
// Zero Editor
// Copyright (C) 2015-2016 Matthias Bolte <matthias.bolte@googlemail.com>
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

#ifndef RECENTFILESWIDGET_H
#define RECENTFILESWIDGET_H

#include <QWidget>

class QActionGroup;

namespace Ui {
class RecentFilesWidget;
}

class RecentFilesWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(RecentFilesWidget)

public:
    explicit RecentFilesWidget(QWidget *parent = NULL);
    ~RecentFilesWidget();

private slots:
    void showOpenedFiles();
    void showViewedFiles();
    void showModifiedFiles();
    void showClosedFiles();
    void updateModeMenuAndTitle();

private:
    Ui::RecentFilesWidget *m_ui;

    QActionGroup *m_actionGroup;
    QAction *m_actionOpened;
    QAction *m_actionViewed;
    QAction *m_actionModified;
    QAction *m_actionClosed;
};

#endif // RECENTFILESWIDGET_H
