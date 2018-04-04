//
// Zero Editor
// Copyright (C) 2015-2018 Matthias Bolte <matthias.bolte@googlemail.com>
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

#ifndef EDITOR_H
#define EDITOR_H

#include <QObject>

class Document;
class QWidget;

class Editor : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Editor)

public:
    enum Action {
        Undo,
        Redo,
        Cut,
        Copy,
        Paste,
        Delete,
        SelectAll,
        ToggleCase
    };

    enum Feature {
        WordWrapping
    };

    explicit Editor(QObject *parent = NULL);

    virtual Document *document() const = 0;
    virtual QWidget *widget() const = 0;

    virtual bool isActionAvailable(Action action) const { Q_UNUSED(action) return false; }
    virtual bool hasFeature(Feature feature) const { Q_UNUSED(feature) return false; }

    virtual bool isWordWrapping() const { return false; }

public slots:
    virtual void undo() { }
    virtual void redo() { }
    virtual void cut() { }
    virtual void copy() { }
    virtual void paste() { }
    virtual void delete_() { }
    virtual void selectAll() { }
    virtual void toggleCase() { }

    virtual void setWordWrapping(bool enable) { Q_UNUSED(enable) }

signals:
    void actionAvailabilityChanged(Action action, bool available);
};

#endif // EDITOR_H
