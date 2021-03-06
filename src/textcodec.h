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

#ifndef TEXTCODEC_H
#define TEXTCODEC_H

#include <QHash>
#include <QList>
#include <QMetaType>
#include <QTextCodec>

class TextCodec;

class TextCodecState
{
    Q_DISABLE_COPY(TextCodecState)

public:
    TextCodecState() : m_first(true) { }

    bool hasError() const { return m_state.invalidChars != 0 || m_state.remainingChars != 0; }

private:
    bool m_first;
    QTextCodec::ConverterState m_state;

    friend class TextCodec;
};

class TextCodec
{
    Q_DISABLE_COPY(TextCodec)

public:
    qint64 number() const { return mibToNumber(m_codec->mibEnum(), m_byteOrderMark); }
    QString name() const;
    QStringList aliases() const;

    QString decode(const char *input, int length, TextCodecState *state = NULL) const;
    QByteArray encode(const QChar *input, int length, TextCodecState *state) const;

    static void initialize();
    static QList<qint64> knownNumbers() { return s_codecs->keys(); }
    static TextCodec *fromNumber(qint64 number) { return s_codecs->value(number, NULL); }
    static TextCodec *fromName(const QString &name);
    static TextCodec *fromByteOrderMark(const QByteArray &data);

private:
    static qint64 mibToNumber(int mib, bool byteOrderMark);

    TextCodec(QTextCodec *codec, bool byteOrderMark);

    QTextCodec *m_codec;
    bool m_byteOrderMark;

    static QHash<qint64, TextCodec *> *s_codecs;
};

Q_DECLARE_METATYPE(TextCodec *)

#endif // TEXTCODEC_H
