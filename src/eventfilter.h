//
// Zero Editor
// Copyright (C) 2018 Matthias Bolte <matthias.bolte@googlemail.com>
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

#ifndef EVENTFILTER_H
#define EVENTFILTER_H

#include <QObject>

class EventFilter : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(EventFilter)

public:
    explicit EventFilter(QObject *parent = NULL);
    ~EventFilter();

    static EventFilter *instance() { return s_instance; }

protected:
    bool eventFilter(QObject *object, QEvent *event);

private:
    static EventFilter *s_instance;
};

#endif // EVENTFILTER_H