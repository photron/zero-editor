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

#ifndef TEXTDOCUMENT_H
#define TEXTDOCUMENT_H

#include "document.h"

#include <QTextDocument>

class TextDocument : public Document
{
    Q_OBJECT
    Q_DISABLE_COPY(TextDocument)

public:
    explicit TextDocument(QObject *parent = NULL);
    ~TextDocument();

    bool open(const QString &filePath, QString *error);

    QTextDocument *document() const { return m_document; }

private:
    QTextDocument *m_document;
};

#endif // TEXTDOCUMENT_H
