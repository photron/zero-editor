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

#include "textdocument.h"

#include "monospacefontmetrics.h"

#include <QFile>
#include <QDir>
#include <QPlainTextDocumentLayout>

TextDocument::TextDocument(QObject *parent) :
    Document(parent),
    m_document(new QTextDocument)
{
    m_document->setDocumentLayout(new QPlainTextDocumentLayout(m_document));

    // Show tabs and spaces and set tab size to 4 spaces
    QTextOption option = m_document->defaultTextOption();

    // FIXME: trailing spaces in a wrapped line are not shown
    option.setFlags(option.flags() | QTextOption::ShowTabsAndSpaces);
    option.setTabStop(MonospaceFontMetrics::charWidth() * 4);

    m_document->setDefaultTextOption(option);
}

TextDocument::~TextDocument()
{
    delete m_document;
}

// Returns true if the given file was successfully opened. Returns false if an error occurred while opening the file
// and sets error to a non-null QString describing the error. Also returns false if the file contains an unsupported
// document type and sets error to a null QString.
bool TextDocument::open(const QString &filePath, QString *error)
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

    if (data.contains('\0')) {
        *error = QString();

        return false;
    }

    // FIXME: check for proper encoding and not containing undecodable data

    // Setup internal QTextDocument before exposing it to the outside
    m_document->setPlainText(data);
    m_document->setModified(false);

    setFilePath(filePath);

    connect(m_document, &QTextDocument::modificationChanged, this, &TextDocument::setModified);

    return true;
}
