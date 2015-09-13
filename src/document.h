//
// Zero Editor
// Copyright (C) 2015 Matthias Bolte <matthias.bolte@googlemail.com>
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

#include <QObject>

class Document : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Document)

public:
    explicit Document(QObject *parent = NULL);

    // Return true if the given file was successfully opened. Return false if an error occurred while opening the
    // file and set error to a non-null QString describing the error. Also returns false if the file contains an
    // unsupported document type and set error to a null QString.
    virtual bool open(const QString &filePath, QString *error) = 0;

    QString filePath() const { return m_filePath; }
    bool isModified() const { return m_modified; }

signals:
    void modificationChanged(bool modified);

protected:
    void setFilePath(const QString &filePath);

protected slots:
    void setModified(bool modified);

private:
    QString m_filePath;
    bool m_modified;
};

#endif // DOCUMENT_H
