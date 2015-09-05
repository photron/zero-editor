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

#ifndef GITDIFFWIDGET_H
#define GITDIFFWIDGET_H

#include <QWidget>

namespace Ui {
class GitDiffWidget;
}

class GitDiffWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GitDiffWidget(QWidget *parent = NULL);
    ~GitDiffWidget();

signals:
    void hideClicked();

private:
    Ui::GitDiffWidget *m_ui;
};

#endif // GITDIFFWIDGET_H
