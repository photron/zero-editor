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

#include "textdocument.h"

#include "monospacefontmetrics.h"

#include <QFile>
#include <QDebug>
#include <QDir>
#include <QPlainTextDocumentLayout>
#include <QTextCodec>
#include <QTextDocument>

TextDocument::TextDocument(QObject *parent) :
    Document(Text, parent),
    m_internalDocument(new QTextDocument),
    m_contentsModified(false),
    m_codec(QTextCodec::codecForName("UTF-8")),
    m_byteOrderMark(false),
    m_hasDecodingError(false),
    m_encodingModified(false)
{
    m_internalDocument->setDocumentLayout(new QPlainTextDocumentLayout(m_internalDocument));

    // Show tabs and spaces and set tab size to 4 spaces
    QTextOption option = m_internalDocument->defaultTextOption();

    // FIXME: trailing spaces in a wrapped line are not shown
    option.setFlags(option.flags() | QTextOption::ShowTabsAndSpaces);
    option.setTabStop(MonospaceFontMetrics::charWidth() * 4);

    m_internalDocument->setDefaultTextOption(option);

    connect(m_internalDocument, &QTextDocument::modificationChanged, this, &TextDocument::setContentsModified);
}

TextDocument::~TextDocument()
{
    delete m_internalDocument;
}

// Returns true if the given file was successfully opened. If codec is non-null then it is used to decode the document
// data, otherwise the encoding of the document is guessed based on the presence of a Unicode BOM. Returns false if an
// error occurred while opening the file and sets error to a non-null QString describing the error.
bool TextDocument::open(const QString &filePath, QTextCodec *codec, QString *error)
{
    Q_ASSERT(!filePath.isEmpty());
    Q_ASSERT(error != NULL);

    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly)) {
        *error = QString("Could not open '%1' for reading: %2").arg(QDir::toNativeSeparators(filePath),
                                                                    file.errorString());

        return false;
    }

    // FIXME: do this in chunks to avoid blocking the UI if the file is big
    QByteArray data = file.readAll();

    if (file.error() != QFile::NoError) {
        *error = QString("Could not read '%1': %2").arg(QDir::toNativeSeparators(filePath), file.errorString());

        return false;
    }

    if (codec != NULL) {
        m_codec = codec;
        m_byteOrderMark = false;
    } else {
        m_codec = QTextCodec::codecForUtfText(data, NULL);
        m_byteOrderMark = false;

        if (m_codec == NULL) {
            m_codec = QTextCodec::codecForName("UTF-8");
        } else {
            // QTextCodec::codecForUtfText() detects the encoding based on the Unicode BOM. If it can detect the
            // encoding then a Unicode BOM is present.
            m_byteOrderMark = true;
        }
    }

    // FIXME: do this in chunks to avoid blocking the UI if the file is big
    QTextCodec::ConverterState state;
    QString text = m_codec->toUnicode(data.constData(), data.length(), &state);

    m_hasDecodingError = state.invalidChars != 0 || state.remainingChars != 0;

    disconnect(m_internalDocument, &QTextDocument::modificationChanged, this, &TextDocument::setContentsModified);

    m_internalDocument->setPlainText(text);
    m_internalDocument->setModified(false);

    connect(m_internalDocument, &QTextDocument::modificationChanged, this, &TextDocument::setContentsModified);

    setFilePath(filePath);

    return true;
}

void TextDocument::setCodec(QTextCodec *codec)
{
    Q_ASSERT(codec != NULL);

    if (m_codec != codec) {
        m_codec = codec;

        setEncodingModified(true);
    }
}

void TextDocument::setByteOrderMark(bool enable)
{
    if (m_byteOrderMark != enable) {
        m_byteOrderMark = enable;

        setEncodingModified(true);
    }
}

// private slot
void TextDocument::setContentsModified(bool modified)
{
    if (m_contentsModified != modified) {
        m_contentsModified = modified;

        setModified(m_contentsModified || m_encodingModified);
    }
}

// private
void TextDocument::setEncodingModified(bool modified)
{
    if (m_encodingModified != modified) {
        m_encodingModified = modified;

        setModified(m_contentsModified || m_encodingModified);
    }
}
