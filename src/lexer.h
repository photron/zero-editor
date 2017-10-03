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

#ifndef LEXER_H
#define LEXER_H

#include <QBitArray>
#include <QString>

struct Token
{
    enum Kind {
        EndOfInput,
        Unknown,
        Error,

        Whitespace,
        Identifier,

        HexadecimalIntegerLiteral,
        DecimalIntegerLiteral,
        OctalIntegerLiteral,
        BinaryIntegerLiteral,

        HexadecimalFloatLiteral,
        DecimalFloatLiteral,

        CStringLiteral,
        CCharacterLiteral,
        //PascalStringLiteral,

        CComment,
        CPlusPlusComment,
        ScriptComment,
        PascalComment,

        LeftParenthesis,
        RightParenthesis,
        LeftBrace, // shadowed by PascalComment token, if enabled
        RightBrace, // shadowed by PascalComment token, if enabled
        LeftBracket,
        RightBracket,

        Semicolon,
        Colon,
        Comma,
        Dot,

        Plus,
        Minus,
        Star,
        Slash,
        Percent,
        Ampersand,
        Pipe,
        Caret,
        Tilde,
        Hash,
        Question,
        Exclamation,
        Equal,
        Less,
        Greater,

        PlusEqual,
        MinusEqual,
        StarEqual,
        SlashEqual,
        PercentEqual,
        AmpersandEqual,
        PipeEqual,
        CaretEqual,
        TildeEqual,
        ExclamationEqual,
        LessEqual,
        GreaterEqual,

        PlusPlus,
        MinusMinus,
        StarStar,
        SlashSlash, // shadowed by CPlusPlusComment token, if enabled
        AmpersandAmpersand,
        PipePipe,
        EqualEqual,
        LessLess,
        GreaterGreater,

        LessLessEqual,
        GreaterGreaterEqual
    };

    Kind kind;
    int offset;
    int length;
};

class Lexer
{
    Q_DISABLE_COPY(Lexer)

public:
    enum Option {
        CTrigraph,
        CDigraph,
        CPlusPlusAlternativeNotation, // does not include the common C trigraphs and digraphs
        PascalDigraph,

        CLineContinuation, // "\\\n" -> ""
        CNumericLiteralTypeSuffix,
        PythonAlternativeOctalIntegerLiteralNotation, // "0o1234" and "01234"

        WhitespaceToken,
        IdentifierToken,

        HexadecimalIntegerLiteralToken, // "0x1234"
        DecimalIntegerLiteralToken, // "1234"
        OctalIntegerLiteralToken, // "01234"
        BinaryIntegerLiteralToken, // "0b1101"

        HexadecimalFloatLiteralToken, // "0x1234.56p78"
        DecimalFloatLiteralToken, // "1234.56e78"

        CStringLiteralToken,
        CCharacterLiteralToken,

        CCommentToken,
        CPlusPlusCommentToken,
        ScriptCommentToken,
        PascalCommentToken,

        LeftParenthesisToken,
        RightParenthesisToken,
        LeftBraceToken, // shadowed by PascalCommentToken option, if enabled
        RightBraceToken, // shadowed by PascalCommentToken option, if enabled
        LeftBracketToken,
        RightBracketToken,

        SemicolonToken,
        ColonToken,
        CommaToken,
        DotToken,

        PlusToken,
        MinusToken,
        StarToken,
        SlashToken,
        PercentToken,
        AmpersandToken,
        PipeToken,
        CaretToken,
        TildeToken,
        HashToken,
        QuestionToken,
        ExclamationToken,
        EqualToken,
        LessToken,
        GreaterToken,

        PlusEqualToken,
        MinusEqualToken,
        StarEqualToken,
        SlashEqualToken,
        PercentEqualToken,
        AmpersandEqualToken,
        PipeEqualToken,
        CaretEqualToken,
        TildeEqualToken,
        ExclamationEqualToken,
        LessEqualToken,
        GreaterEqualToken,

        PlusPlusToken,
        MinusMinusToken,
        StarStarToken,
        SlashSlashToken, // shadowed by CPlusPlusCommentToken option, if enabled
        AmpersandAmpersandToken,
        PipePipeToken,
        EqualEqualToken,
        LessLessToken,
        GreaterGreaterToken,

        LessLessEqualToken,
        GreaterGreaterEqualToken,

        LastOption
    };

    enum State {
        InNothing,
        InCStringLiteralToken, // can be multiline due to C line continuation
        InPascalStringLiteralToken, // FIXME: can be multiline?
        InCCommentToken, // can inherently be multiline
        InCPlusPlusCommentToken, // can be multiline due to C line continuation
        InPascalCommentToken // can inherently be multiline
    };

    explicit Lexer(const QString &input, State state = InNothing);

    QString input() const { return m_input; }
    State state() const { return m_state; }

    void setOption(Option option, bool enable);
    bool hasOption(Option option) const { return m_options.testBit(option); }

    void scan(Token *token);

private:
    enum SkipResult {
        NothingSkipped,
        PartiallySkipped,
        FullySkipped
    };

    static bool isWhitespace(uint c) { return c == ' ' || (c >= '\t' && c <= '\r'); }
    static bool isHexadecimalDigit(uint c) { return isDecimalDigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'); }
    static bool isDecimalDigit(uint c) { return c >= '0' && c <= '9'; }
    static bool isOctalDigit(uint c) { return c >= '0' && c <= '7'; }
    static bool isBinaryDigit(uint c) { return c >= '0' && c <= '1'; }
    static bool isLetter(uint c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }

    void current();
    void next();

    SkipResult skipDecimalFloatLiteralFraction(bool hasLeadingDigits);
    SkipResult skipHexadecimalFloatLiteralExponent();
    SkipResult skipDecimalFloatLiteralExponent();
    SkipResult skipFloatLiteralExponent(uint lower, uint upper);
    SkipResult skipCIntegerLiteralTypeSuffix();
    SkipResult skipCFloatLiteralTypeSuffix();

    QString m_input;
    State m_state;
    QBitArray m_options;

    bool m_hasAnyIntegerLiteralTokenOption;
    bool m_hasAnyFloatLiteralTokenOption;
    bool m_hasAnyNumericLiteralTokenOption;

    int m_offset;
    uint m_char; // FIXME: this variable name is wrong, it's actually a code point, http://www.utf8everywhere.org/
    int m_charLength;
    uint m_nextCPlusPlusAlternativeChar;
    int m_nextCPlusPlusAlternativeCharLength;

};

class CLexer : public Lexer
{
public:
    explicit CLexer(const QString &input, State state = InNothing) :
        Lexer(input, state)
    {
        // FIXME: incomplete
        setOption(QuestionToken, true);
        setOption(PlusPlusToken, true);
        setOption(MinusMinusToken, true);
        setOption(AmpersandAmpersandToken, true);
        setOption(PipePipeToken, true);
        setOption(CCommentToken, true);
        setOption(CPlusPlusCommentToken, true);
        setOption(CLineContinuation, true);
        setOption(CNumericLiteralTypeSuffix, true);
    }
};

class CPlusPlusLexer : public CLexer
{
public:
    explicit CPlusPlusLexer(const QString &input, State state = InNothing) :
        CLexer(input, state)
    {
        // FIXME: incomplete
        setOption(CPlusPlusCommentToken, true);
        setOption(CPlusPlusAlternativeNotation, true);
    }
};
/*
class PythonLexer : public Lexer
{

};*/

#endif // LEXER_H
