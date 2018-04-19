//
// Zero Editor
// Copyright (C) 2016-2018 Matthias Bolte <matthias.bolte@googlemail.com>
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
#include <QRegularExpression>

LocationData::LocationData(const QString &path_)
{
    if (!path_.isEmpty()) {
        Q_ASSERT(QDir::isAbsolutePath(path_));

        isDirectory = path_.endsWith("/") || path_.endsWith("\\");

        // Qt's QDir and QFileInfo are full of inconsistencies. For example, QDir::cleanPath() strips trailing
        // separators (except from root directories), but QFileInfo relies on the trailing separator to tell apart a
        // file path from a directory path. Also, the QFileInfo absolute and canonical path methods are inconsistent
        // in the way they split a path into the directory path and the file name part. Therefore, only use
        // QDir::cleanPath() do resolve dots and double-dots in the file path and do the rest by hand.
        path = QDir::toNativeSeparators(QDir::cleanPath(path_));

#ifdef Q_OS_WIN
        // On Windows C:Blubb (drive relative) is not the same as C:\Blubb (absolute). But drive relative are not widely
        // known and difficult to use. Therefore, just pretend that drive relative paths are typos and just inject the
        // separator after the colon to force an absolute path. Also Qt doesn't understand drive relative paths and
        // treats them as absolute paths anyway.
        path.replace(QRegularExpression("^([A-Za-z]:)([^\\\\])"), "\\1\\\\2");
#endif

        if (isDirectory) {
            // QDir::cleanPath() strips trailing separators from all paths, except from root directories
            if (!path.endsWith(QDir::separator())) {
                path += QDir::separator(); // Add trailing separator to indicate directory
            }

            directoryPath = path;
            fileName = "";
        } else {
            int index = path.lastIndexOf(QDir::separator());

            Q_ASSERT(index >= 0);

            directoryPath = path.left(index + 1);
            fileName = path.mid(index + 1);
        }

        directoryName = directoryPath.split(QDir::separator(), QString::SkipEmptyParts).last();
    }
}

LocationData::LocationData(const LocationData &other) :
    QSharedData(other),
    isDirectory(other.isDirectory),
    path(other.path),
    directoryPath(other.directoryPath),
    directoryName(other.directoryName),
    fileName(other.fileName)
{
}
