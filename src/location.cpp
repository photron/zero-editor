//
// Zero Editor
// Copyright (C) 2016-2017 Matthias Bolte <matthias.bolte@googlemail.com>
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

#include "location.h"

#include <QDir>
#include <QFileInfo>

LocationData::LocationData(const QString &filePath_)
{
    if (!filePath_.isEmpty()) {
        QFileInfo fileInfo(filePath_);

        // Canonical getters for QFileInfo only work with existing file paths
        if (fileInfo.exists()) {
            filePath = QDir::toNativeSeparators(fileInfo.canonicalFilePath());
            directoryPath = QDir::toNativeSeparators(fileInfo.canonicalPath());
        } else {
            filePath = QDir::toNativeSeparators(fileInfo.absoluteFilePath());
            directoryPath = QDir::toNativeSeparators(fileInfo.absolutePath());
        }

        fileName = fileInfo.fileName();
    }
}

LocationData::LocationData(const LocationData &other) :
    QSharedData(other),
    filePath(other.filePath),
    directoryPath(other.directoryPath),
    fileName(other.fileName)
{
}
