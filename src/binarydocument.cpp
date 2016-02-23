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

#include "binarydocument.h"

BinaryDocument::BinaryDocument(QObject *parent) :
    Document(Binary, parent)
{
}

bool BinaryDocument::load(const QByteArray &data, QString *error)
{
    Q_ASSERT(error != NULL);

    if (data.length() == 0) {
        *error = QString("Can not open empty file \"%1\" in binary mode").arg(location().filePath("unnamed"));

        return false;
    }

    m_data = data;

    return true;
}

bool BinaryDocument::save(QByteArray *data, QString *error)
{
    Q_ASSERT(data != NULL);
    Q_ASSERT(error != NULL);

    *data  = m_data;

    return true;
}

void BinaryDocument::setByteAt(int i, quint8 byte)
{
    if (m_data.at(i) != byte) {
        m_data[i] = byte;

        setModified(true);
    }
}
