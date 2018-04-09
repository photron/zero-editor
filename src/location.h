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

#ifndef LOCATION_H
#define LOCATION_H

#include <QDir>
#include <QFileInfo>
#include <QSharedData>

class LocationData : public QSharedData
{
public:
    LocationData(const QString &path);
    LocationData(const LocationData &other);

    bool isDirectory;
    QString path; // <directoryPath> + <fileName>
    QString directoryPath; // Guaranteed to have a trailing separator
    QString fileName;
};

class Location
{
public:
    Location() : d(new LocationData("")) { }
    Location(const char *path) : d(new LocationData(path)) { }
    Location(const QString &path) : d(new LocationData(path)) { }

    bool isEmpty() const { return d->path.isEmpty(); }
    bool isDirectory() const { return d->isDirectory; }
    bool isFile() const { return !isEmpty() && !isDirectory(); }

    QString path(const QString &empty = QString()) const { return isEmpty() ? empty : d->path; }
    QString directoryPath(const QString &empty = QString()) const { return isEmpty() ? empty : d->directoryPath; }
    QString fileName(const QString &empty = QString()) const { return isEmpty() ? empty : d->fileName; }

    bool exists() const { return QFileInfo::exists(d->path); }

    Location file(const QString &name) { return d->directoryPath + name; }

    // Use QFileInfo for path comparison to avoid treating paths as different if they only differ in case but are
    // located on a caseless file system and are actually identical. QFileInfo handles this case correctly.
    bool operator==(const Location &other) const { return d == other.d || QFileInfo(d->path) == QFileInfo(other.d->path); }
    bool operator!=(const Location &other) const { return !(*this == other); }

private:
    QSharedDataPointer<LocationData> d;
};

inline uint qHash(const Location &key, uint seed = 0)
{
    return qHash(key.path(), seed);
}

#endif // LOCATION_H
