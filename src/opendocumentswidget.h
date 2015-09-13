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

#ifndef OPENDOCUMENTSWIDGET_H
#define OPENDOCUMENTSWIDGET_H

#include <QWidget>

namespace Ui {
class OpenDocumentsWidget;
}

class OpenDocumentsWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(OpenDocumentsWidget)

public:
    explicit OpenDocumentsWidget(QWidget *parent = NULL);
    ~OpenDocumentsWidget();

    void installLineEditEventFilter(QObject *filter);

private:
    Ui::OpenDocumentsWidget *m_ui;
};

#endif // OPENDOCUMENTSWIDGET_H
