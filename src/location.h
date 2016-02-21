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

#ifndef LOCATION_H
#define LOCATION_H

#include <QSharedData>

class LocationData : public QSharedData
{
public:
    LocationData(const QString &filePath);
    LocationData(const LocationData &other);

    QString internalFilePath;
    QString displayFilePath;
    QString displayDirectoryPath;
    QString displayFileName;
};

class Location
{
public:
    Location() : d(new LocationData("")) { }
    Location(const QString &filePath) : d(new LocationData(filePath)) { }

    bool isEmpty() const { return d->internalFilePath.isEmpty(); }

    QString internalFilePath() const { return d->internalFilePath; }
    QString displayFilePath() const { return d->displayFilePath; }
    QString displayDirectoryPath() const { return d->displayDirectoryPath; }
    QString displayFileName() const { return d->displayFileName; }

    bool operator==(const Location &other) const { return d == other.d || d->internalFilePath == other.d->internalFilePath; }
    bool operator!=(const Location &other) const { return !(*this == other); }

private:
    QSharedDataPointer<LocationData> d;
};

#endif // LOCATION_H
