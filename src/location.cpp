//
// Zero Editor
// Copyright (C) 2016 Matthias Bolte <matthias.bolte@googlemail.com>
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

LocationData::LocationData(const QString &filePath)
{
    if (filePath.isEmpty()) {
        internalFilePath = "";
        displayFilePath = "unnamed";
        displayDirectoryPath = "unnamed";
        displayFileName = "unnamed";
    } else {
        QFileInfo fileInfo(filePath);

        internalFilePath = fileInfo.canonicalFilePath();
        displayFilePath = QDir::toNativeSeparators(internalFilePath);
        displayDirectoryPath = QDir::toNativeSeparators(fileInfo.canonicalPath());
        displayFileName = fileInfo.fileName();
    }
}

LocationData::LocationData(const LocationData &other) :
    QSharedData(other),
    internalFilePath(other.internalFilePath),
    displayFilePath(other.displayFilePath),
    displayDirectoryPath(other.displayDirectoryPath),
    displayFileName(other.displayFileName)
{
}
