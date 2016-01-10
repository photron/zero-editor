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

#ifndef TEXTDOCUMENT_H
#define TEXTDOCUMENT_H

#include "document.h"

class QTextCodec;
class QTextDocument;

class TextDocument : public Document
{
    Q_OBJECT
    Q_DISABLE_COPY(TextDocument)

public:
    explicit TextDocument(QObject *parent = NULL);
    ~TextDocument();

    bool open(const QString &filePath, QTextCodec *codec, QString *error);

    QTextDocument *internalDocument() const { return m_internalDocument; }

    void setCodec(QTextCodec *codec);
    QTextCodec *codec() const { return m_codec; }

    void setByteOrderMark(bool enable);
    bool byteOrderMark() const { return m_byteOrderMark; }

    bool hasDecodingError() const { return m_hasDecodingError; }

private slots:
    void setContentsModified(bool modified);

private:
    void setEncodingModified(bool modified);

    QTextDocument *m_internalDocument;
    bool m_contentsModified;

    QTextCodec *m_codec;
    bool m_byteOrderMark; // applies to Unicode codecs only
    bool m_hasDecodingError;
    bool m_encodingModified;
};

#endif // TEXTDOCUMENT_H
