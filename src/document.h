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

#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "location.h"

#include <QObject>

class Document : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Document)

public:
    enum Type {
        Text,
        Binary
    };

    explicit Document(Type type, QObject *parent = NULL);

    Type type() const { return m_type; }

    virtual bool load(const QByteArray &data, QString *error) = 0;
    virtual bool save(QByteArray *data, QString *error) = 0;

    Location location() const { return m_location; }
    void setLocation(const Location &location);

    bool isModified() const { return m_modified; }

signals:
    void locationChanged(const Location &location);
    void modificationChanged(bool modified);

public slots:
    void setModified(bool modified);

private:
    Type m_type;
    Location m_location;
    bool m_modified;
};

Q_DECLARE_METATYPE(Document::Type)

#endif // DOCUMENT_H
