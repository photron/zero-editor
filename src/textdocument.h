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

#ifndef TEXTDOCUMENT_H
#define TEXTDOCUMENT_H

#include "document.h"

class QTextDocument;

class SyntaxHighlighter;
class TextCodec;

class TextDocument : public Document
{
    Q_OBJECT
    Q_DISABLE_COPY(TextDocument)

public:
    explicit TextDocument(TextCodec *codec, QObject *parent = NULL);
    ~TextDocument();

    bool load(const QByteArray &data, QString *error);
    bool save(QByteArray *data, QString *error);

    QTextDocument *internalDocument() const { return m_internalDocument; }

    void setCodec(TextCodec *codec);
    TextCodec *codec() const { return m_codec; }

    bool hasDecodingError() const { return m_hasDecodingError; }

private slots:
    void setContentsModified(bool modified);

private:
    void setEncodingModified(bool modified);

    QTextDocument *m_internalDocument;
    bool m_isContentsModified;
    SyntaxHighlighter *m_syntaxHighlighter;

    TextCodec *m_codec;
    bool m_hasDecodingError;
    bool m_isEncodingModified;
};

#endif // TEXTDOCUMENT_H
