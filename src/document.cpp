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

#include "document.h"

#include <QDebug>

Document::Document(QObject *parent) :
    QObject(parent),
    m_modified(false)
{
}

// protected
void Document::setFilePath(const QString &filePath)
{
    m_filePath = filePath;
}

// protected slot
void Document::setModified(bool modified)
{
    if (m_modified != modified) {
        m_modified = modified;

        emit modificationChanged(m_modified);
    }
}
