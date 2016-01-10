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

#ifndef BINARYDOCUMENT_H
#define BINARYDOCUMENT_H

#include "document.h"

#include <QByteArray>

class BinaryDocument : public Document
{
    Q_OBJECT
    Q_DISABLE_COPY(BinaryDocument)

public:
    explicit BinaryDocument(QObject *parent = NULL);

    bool open(const QString &filePath, QString *error);

    int length() const { return m_data.length(); }

    quint8 byteAt(int i) const { return m_data.at(i); }
    void setByteAt(int i, quint8 byte);

    QByteArray slice(int i, int length = -1) { return m_data.mid(i, length); }

private:
    QByteArray m_data;
};

#endif // BINARYDOCUMENT_H
