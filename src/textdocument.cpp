//
// Zero Editor
// Copyright (C) 2015-2018 Matthias Bolte <matthias.bolte@googlemail.com>
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
#include "syntaxhighlighter.h"
#include "textcodec.h"

#include <QFile>
#include <QDebug>
#include <QDir>
#include <QPlainTextDocumentLayout>
#include <QTextDocument>

TextDocument::TextDocument(TextCodec *codec, QObject *parent) :
    Document(Text, parent),
    m_internalDocument(new QTextDocument),
    m_isContentsModified(false),
    m_syntaxHighlighter(NULL),
    m_codec(codec),
    m_hasDecodingError(false),
    m_isEncodingModified(false)
{
    m_internalDocument->setDocumentLayout(new QPlainTextDocumentLayout(m_internalDocument));

    // Show tabs and spaces and set tab size to 4 spaces
    QTextOption option = m_internalDocument->defaultTextOption();

    // FIXME: trailing spaces in a wrapped line are not shown
    option.setFlags(option.flags() | QTextOption::ShowTabsAndSpaces);
    option.setTabStop(MonospaceFontMetrics::charWidth() * 4);

    m_internalDocument->setDefaultTextOption(option);

    connect(m_internalDocument, &QTextDocument::modificationChanged, this, &TextDocument::setContentsModified);

    m_syntaxHighlighter = new SyntaxHighlighter(m_internalDocument);
}

TextDocument::~TextDocument()
{
    // Disconnect all signals before deleting the syntax highlighter. Otherwise the syntax highlighter might trigger
    // a QTextDocument::contentsChanged signal emission that makes the TextEditor access this TextDocument object while
    // it is being deleted, resulting in a segfault.
    m_internalDocument->disconnect();

    delete m_syntaxHighlighter;
    delete m_internalDocument;
}

bool TextDocument::load(const QByteArray &data, QString *error)
{
    Q_ASSERT(error != NULL);

    if (m_codec == NULL) {
        m_codec = TextCodec::fromByteOrderMark(data);

        if (m_codec == NULL) {
            m_codec = TextCodec::fromName("UTF-8");
        }
    }

    // FIXME: do this in chunks to avoid blocking the UI if the file is big
    TextCodecState state;
    const QString &text = m_codec->decode(data.constData(), data.length(), &state);

    m_hasDecodingError = state.hasError();

    // FIXME: detect line ending

    disconnect(m_internalDocument, &QTextDocument::modificationChanged, this, &TextDocument::setContentsModified);

    // QTextDocument::setPlainText replaces "\r" and "\r\n" with "\n"
    m_internalDocument->setPlainText(text);
    m_internalDocument->setModified(false);

    connect(m_internalDocument, &QTextDocument::modificationChanged, this, &TextDocument::setContentsModified);

    return true;
}

bool TextDocument::save(QByteArray *data, QString *error)
{
    Q_ASSERT(data != NULL);
    Q_ASSERT(error != NULL);
    Q_ASSERT(m_codec != NULL);

    // QTextDocument::toPlainText only outputs "\n" as line ending
    const QString &text = m_internalDocument->toPlainText();

    // FIXME: replace "\n" with actually selected line ending for this document

    TextCodecState state;

    // FIXME: do this in chunks to avoid blocking the UI if the file is big
    *data = m_codec->encode(text.constData(), text.length(), &state);

    if (state.hasError()) {
        *error = QString("Can not encode text for file \"%1\" as %2").arg(location().filePath("unnamed"),
                                                                          QString(m_codec->name()));

        return false;
    }

    return true;
}

// FIXME: this needs to be recorded as part of the undo history
void TextDocument::setCodec(TextCodec *codec)
{
    Q_ASSERT(codec != NULL);

    if (m_codec != codec) {
        m_codec = codec;

        setEncodingModified(true);
    }
}

// private slot
void TextDocument::setContentsModified(bool modified)
{
    if (m_isContentsModified != modified) {
        m_isContentsModified = modified;

        setModified(m_isContentsModified || m_isEncodingModified);
    }
}

// private
void TextDocument::setEncodingModified(bool modified)
{
    if (m_isEncodingModified != modified) {
        m_isEncodingModified = modified;

        setModified(m_isContentsModified || m_isEncodingModified);
    }
}
