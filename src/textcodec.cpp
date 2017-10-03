//
// Zero Editor
// Copyright (C) 2016-2017 Matthias Bolte <matthias.bolte@googlemail.com>
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

#include "textcodec.h"

// Use a QHash pointer here to avoid potential static initialization order problems
QHash<qint64, TextCodec *> *TextCodec::s_codecs = NULL;

namespace IANA {

// http://www.iana.org/assignments/character-sets/character-sets.xml
enum {
    UTF8 = 106,
    UTF16BE = 1013,
    UTF16LE = 1014,
    UTF16 = 1015,
    UTF32BE = 1018,
    UTF32LE = 1019,
    UTF32 = 1017,
};

}

QByteArray TextCodec::name() const
{
    QByteArray name = m_codec->name();

    if (m_byteOrderMark) {
        name += "-BOM";
    }

    return name;
}

QString TextCodec::decode(const char *input, int length, TextCodecState *state) const
{
    return m_codec->toUnicode(input, length, state != NULL ? &state->m_state : NULL);
}

QByteArray TextCodec::encode(const QChar *input, int length, TextCodecState *state) const
{
    if (state->m_first && !m_byteOrderMark) {
        state->m_state.flags |= QTextCodec::IgnoreHeader;
    }

    return m_codec->fromUnicode(input, length, state != NULL ? &state->m_state : NULL);
}

// static
void TextCodec::initialize()
{
    s_codecs = new QHash<qint64, TextCodec *>; // FIXME: This leaks memory

    foreach (int mib, QTextCodec::availableMibs()) {
        QTextCodec *codec = QTextCodec::codecForMib(mib);

        s_codecs->insert(mibToNumber(mib, false), new TextCodec(codec, false));

        if (codec->name().startsWith("UTF")) {
            s_codecs->insert(mibToNumber(mib, true), new TextCodec(codec, true));
        }
    }
}

// static
TextCodec *TextCodec::fromName(const QByteArray &name)
{
    bool byteOrderMark = name.startsWith("UTF") && name.endsWith("BOM");
    QTextCodec *codec = QTextCodec::codecForName(name.mid(0, name.length() - (byteOrderMark ? 3 : 0)));

    if (codec == NULL) {
        return NULL;
    }

    return fromNumber(mibToNumber(codec->mibEnum(), byteOrderMark));
}

// static
TextCodec *TextCodec::fromByteOrderMark(const QByteArray &data)
{
    int size = data.size();
    const uchar *bytes = (const uchar *)data.constData();

    if (size >= 4) {
        if (bytes[0] == 0x00 && bytes[1] == 0x00 && bytes[2] == 0xFE && bytes[3] == 0xFF) {
            return fromNumber(mibToNumber(IANA::UTF32BE, true)); // UTF-32BE-BOM
        } else if (bytes[0] == 0xFF && bytes[1] == 0xFE && bytes[2] == 0x00 && bytes[3] == 0x00) {
            return fromNumber(mibToNumber(IANA::UTF32LE, true)); // UTF-32LE-BOM
        }
    }

    if (size >= 2) {
        if (bytes[0] == 0xFE && bytes[1] == 0xFF) {
            return fromNumber(mibToNumber(IANA::UTF16BE, true)); // UTF-16BE-BOM
        } else if (bytes[0] == 0xFF && bytes[1] == 0xFE) {
            return fromNumber(mibToNumber(IANA::UTF16LE, true)); // UTF-16LE-BOM
        }
    }

    if (size >= 3) {
        if (bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF) {
            return fromNumber(mibToNumber(IANA::UTF8, true)); // UTF-8-BOM
        }
    }

    return NULL;
}

// static
qint64 TextCodec::mibToNumber(int mib, bool byteOrderMark)
{
    return mib * 2 + (byteOrderMark ? 1 : 0);
}

// private
TextCodec::TextCodec(QTextCodec *codec, bool byteOrderMark) :
    m_codec(codec),
    m_byteOrderMark(byteOrderMark)
{
}
