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

#include <QFile>
#include <QDir>

BinaryDocument::BinaryDocument(QObject *parent) :
    Document(Binary, parent)
{
}

// Returns true if the given file was successfully opened. Returns false if an error occurred while opening the file
// and sets error to a non-null QString describing the error.
bool BinaryDocument::open(const QString &filePath, QString *error)
{
    Q_ASSERT(!filePath.isEmpty());
    Q_ASSERT(error != NULL);

    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly)) {
        *error = QString("Could not open '%1' for reading: %2").arg(QDir::toNativeSeparators(filePath),
                                                                    file.errorString());

        return false;
    }

    // FIXME: do this in a way that doesn't block the UI if the file is big
    QByteArray data = file.readAll();

    if (file.error() != QFile::NoError) {
        *error = QString("Could not read '%1': %2").arg(QDir::toNativeSeparators(filePath), file.errorString());

        return false;
    }

    if (data.length() == 0) {
        *error = QString("Can not open empty file '%1' in binary mode.").arg(QDir::toNativeSeparators(filePath));

        return false;
    }

    m_data = data;

    setFilePath(filePath);

    return true;
}

void BinaryDocument::setByteAt(int i, quint8 byte)
{
    if (m_data.at(i) != byte) {
        m_data[i] = byte;

        setModified(true);
    }
}
