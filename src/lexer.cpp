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

#include "lexer.h"

#include <QDebug>

// Assumes that the input consists of one or more complete lines of text including their line ending ("\n" only)
Lexer::Lexer(const QString &input, State state) :
    m_input(input),
    m_state(state),
    m_options(LastOption),
    m_hasAnyIntegerLiteralTokenOption(false),
    m_hasAnyFloatLiteralTokenOption(false),
    m_hasAnyNumericLiteralTokenOption(false),
    m_offset(0),
    m_char('\0'),
    m_charLength(0),
    m_nextCPlusPlusAlternativeChar('\0'),
    m_nextCPlusPlusAlternativeCharLength(0)
{
}

void Lexer::setOption(Option option, bool enable)
{
    if (enable) {
        m_options.setBit(option);
    } else {
        m_options.clearBit(option);
    }

    m_hasAnyIntegerLiteralTokenOption = false;

    for (int i = HexadecimalIntegerLiteralToken; i <= BinaryIntegerLiteralToken; ++i) {
        if (m_options.testBit(i)) {
            m_hasAnyIntegerLiteralTokenOption = true;

            break;
        }
    }

    m_hasAnyFloatLiteralTokenOption = false;

    for (int i = HexadecimalFloatLiteralToken; i <= DecimalFloatLiteralToken; ++i) {
        if (m_options.testBit(i)) {
            m_hasAnyFloatLiteralTokenOption = true;

            break;
        }
    }

    m_hasAnyNumericLiteralTokenOption = m_hasAnyIntegerLiteralTokenOption || m_hasAnyFloatLiteralTokenOption;
}

/* C99 keywords
auto
break
case
char
const
continue
default
do
double
else
enum
extern
float
for
goto
if
inline
int
long
register
restrict
return
short
signed
sizeof
static
struct
switch
typedef
union
unsigned
void
volatile
while
_Bool
_Complex
_Imaginary
*/

void Lexer::scan(Token *token)
{
    Q_ASSERT(token != NULL);

    token->kind = Token::Unknown;
    token->offset = m_offset;
    token->length = 0;

    current();

    if (m_char == '\0') {
        token->kind = Token::EndOfInput;

        return;
    }

    uint c;

    switch (m_state) {
    case InNothing:
        break;

    case InCStringLiteralToken:
        c = m_char;

        while (m_char != '\0' && (c == '\\' || m_char != '"')) {
            c = m_char;

            next();
        }

        if (c != '\\' && m_char == '"') {
            m_state = InNothing;

            next();
        }

        token->kind = Token::CStringLiteral;

        break;

    case InPascalStringLiteralToken:
        break; // FIXME

    case InCCommentToken:
        c = m_char;

        while (m_char != '\0' && (c != '*' || m_char != '/')) {
            c = m_char;

            next();
        }

        if (c == '*' && m_char == '/') {
            m_state = InNothing;

            next();
        }

        token->kind = Token::CComment;

        goto exit;

    case InCPlusPlusCommentToken:
        while (m_char != '\0' && m_char != '\n') {
            next();
        }

        if (m_char == '\n') {
            m_state = InNothing;
        }

        token->kind = Token::CPlusPlusComment;

        goto exit;

    case InPascalCommentToken:
        while (m_char != '\0' && m_char != '}') {
            next();
        }

        if (m_char == '}') {
            m_state = InNothing;

            next();
        }

        token->kind = Token::PascalComment;

        goto exit;
    }

    switch (m_char) {
    case '(':
        next();

        if (hasOption(LeftParenthesisToken)) {
            token->kind = Token::LeftParenthesis;
        }

        break;

    case ')':
        next();

        if (hasOption(RightParenthesisToken)) {
            token->kind = Token::RightParenthesis;
        }

        break;

    case '{':
        next();

        if (hasOption(PascalCommentToken)) {
            m_state = InPascalCommentToken;

            next();

            while (m_char != '\0' && m_char != '}') {
                next();
            }

            if (m_char == '}') {
                m_state = InNothing;

                next();
            }

            token->kind = Token::PascalComment;
        } else if (hasOption(LeftBraceToken)) {
            token->kind = Token::LeftBrace;
        }

        break;

    case '}':
        next();

        if (hasOption(RightBraceToken)) {
            token->kind = Token::RightBrace;
        }

        break;

    case '[':
        next();

        if (hasOption(LeftBracketToken)) {
            token->kind = Token::LeftBracket;
        }

        break;

    case ']':
        next();

        if (hasOption(RightBracketToken)) {
            token->kind = Token::RightBracket;
        }

        break;

    case ';':
        next();

        if (hasOption(SemicolonToken)) {
            token->kind = Token::Semicolon;
        }

        break;

    case ':':
        next();

        if (hasOption(ColonToken)) {
            token->kind = Token::Colon;
        }

        break;

    case ',':
        next();

        if (hasOption(CommaToken)) {
           token->kind = Token::Comma;
        }

        break;

    case '.':
        next();

        if (isDecimalDigit(m_char) && hasOption(DecimalFloatLiteralToken)) {
            if (skipDecimalFloatLiteralFraction(false) != FullySkipped) {
                token->kind = Token::Error;

                break;
            }

            if (hasOption(CNumericLiteralTypeSuffix) && skipCFloatLiteralTypeSuffix() == PartiallySkipped) {
                token->kind = Token::Error;

                break;
            }

            token->kind = Token::DecimalFloatLiteral;
        } else if (hasOption(DotToken)) {
            token->kind = Token::Dot;
        }

        break;

    case '+':
        next();

        if (m_char == '+' && hasOption(PlusPlusToken)) {
            next();

            token->kind = Token::PlusPlus;
        } else if (m_char == '=' && hasOption(PlusEqualToken)) {
            next();

            token->kind = Token::PlusEqual;
        } else if (hasOption(PlusToken)) {
            token->kind = Token::Plus;
        }

        break;

    case '-':
        next();

        if (m_char == '-' && hasOption(MinusMinusToken)) {
            next();

            token->kind = Token::MinusMinus;
        } else if (m_char == '=' && hasOption(MinusEqualToken)) {
            next();

            token->kind = Token::MinusEqual;
        } else if (hasOption(MinusToken)) {
            token->kind = Token::Minus;
        }

        break;

    case '*':
        next();

        if (m_char == '*' && hasOption(StarStarToken)) {
            next();

            token->kind = Token::StarStar;
        } else if (m_char == '=' && hasOption(StarEqualToken)) {
            next();

            token->kind = Token::StarEqual;
        } else if (hasOption(StarToken)) {
            token->kind = Token::Star;
        }

        break;

    case '/':
        next();

        if (m_char == '/') {
            if (hasOption(CPlusPlusCommentToken)) {
                m_state = InCPlusPlusCommentToken;

                next();

                while (m_char != '\0' && m_char != '\n') {
                    next();
                }

                if (m_char == '\n') {
                    m_state = InNothing;

                    next();
                }

                token->kind = Token::CPlusPlusComment;
            } else if (hasOption(SlashSlashToken)) {
                next();

                token->kind = Token::SlashSlash;
            } else if (hasOption(SlashToken)) {
                token->kind = Token::Slash;
            }
        } else if (m_char == '*' && hasOption(CCommentToken)) {
            m_state = InCCommentToken;

            next();

            c = m_char;

            while (m_char != '\0' && (c != '*' || m_char != '/')) {
                c = m_char;

                next();
            }

            if (c == '*' && m_char == '/') {
                m_state = InNothing;

                next();
            }

            token->kind = Token::CComment;
        } else if (m_char == '=' && hasOption(SlashEqualToken)) {
            next();

            token->kind = Token::SlashEqual;
        } else if (hasOption(SlashToken)){
            token->kind = Token::Slash;
        }

        break;

    case '%':
        next();

        if (m_char == '=' && hasOption(PercentEqualToken)) {
            next();

            token->kind = Token::PercentEqual;
        } else if (hasOption(PercentToken)) {
            token->kind = Token::Percent;
        }

        break;

    case '&':
        next();

        if (m_char == '&' && hasOption(AmpersandAmpersandToken)) {
            next();

            token->kind = Token::AmpersandAmpersand;
        } else if (m_char == '=' && hasOption(AmpersandEqualToken)) {
            next();

            token->kind = Token::AmpersandEqual;
        } else if (hasOption(AmpersandToken)) {
            token->kind = Token::Ampersand;
        }

        break;

    case '|':
        next();

        if (m_char == '|' && hasOption(PipePipeToken)) {
            next();

            token->kind = Token::PipePipe;
        } else if (m_char == '=' && hasOption(PipeEqualToken)) {
            next();

            token->kind = Token::PipeEqual;
        } else if (hasOption(PipeToken)) {
            token->kind = Token::Pipe;
        }

        break;

    case '^':
        next();

        if (m_char == '=' && hasOption(CaretEqualToken)) {
            next();

            token->kind = Token::CaretEqual;
        } else if (hasOption(CaretToken)) {
            token->kind = Token::Caret;
        }

        break;

    case '~':
        next();

        if (m_char == '=' && hasOption(TildeEqualToken)) {
            next();

            token->kind = Token::TildeEqual;
        } else if (hasOption(TildeToken)) {
            token->kind = Token::Tilde;
        }

        break;

    case '#':
        next();

        if (hasOption(ScriptCommentToken)) {
            while (m_char != '\0' && m_char != '\n') {
                next();
            }

            if (m_char == '\n') {
                next();
            }

            token->kind = Token::ScriptComment;
        } else if (hasOption(HashToken)) {
            token->kind = Token::Hash;
        }

        break;

    case '?':
        next();

        if (hasOption(QuestionToken)) {
            token->kind = Token::Question;
        }

        break;

    case '!':
        next();

        if (m_char == '=' && hasOption(ExclamationEqualToken)) {
            next();

            token->kind = Token::ExclamationEqual;
        } else if (hasOption(ExclamationToken)) {
            token->kind = Token::Exclamation;
        }

        break;

    case '=':
        next();

        if (m_char == '=' && hasOption(EqualEqualToken)) {
            next();

            token->kind = Token::EqualEqual;
        } else if (hasOption(EqualToken)) {
            token->kind = Token::Equal;
        }

        break;

    case '<':
        next();

        if (m_char == '<' && (hasOption(LessLessToken) || hasOption(LessLessEqualToken))) {
            next();

            if (m_char == '=' && hasOption(LessLessEqualToken)) {
                next();

                token->kind = Token::LessLessEqual;
            } else if (hasOption(LessLessToken)) {
                token->kind = Token::LessLess;
            }
        } else if (hasOption(LessToken)) {
            token->kind = Token::Less;
        }

        break;

    case '>':
        next();

        if (m_char == '>' && (hasOption(GreaterGreaterToken) || hasOption(GreaterGreaterEqualToken))) {
            next();

            if (m_char == '=' && hasOption(GreaterGreaterEqualToken)) {
                next();

                token->kind = Token::GreaterGreaterEqual;
            } else if (hasOption(GreaterGreaterToken)) {
                token->kind = Token::GreaterGreater;
            }
        } else if (hasOption(GreaterToken)) {
            token->kind = Token::Greater;
        }

        break;

    default:
        if (isWhitespace(m_char) && hasOption(WhitespaceToken)) {
            next();

            while (isWhitespace(m_char)) {
                next();
            }

            token->kind = Token::Whitespace;
        } else if ((isLetter(m_char) || m_char == '_') && hasOption(IdentifierToken)) {
            next();

            while (isLetter(m_char) || isDecimalDigit(m_char) || m_char == '_') {
                next();
            }

            token->kind = Token::Identifier;
        } else if (isDecimalDigit(m_char) && m_hasAnyNumericLiteralTokenOption) {
            if (m_char != 0) {
                next();

                while (isDecimalDigit(m_char)) {
                    next();
                }

                if (m_char == '.' && hasOption(DecimalFloatLiteralToken)) {
                    next();

                    while (isDecimalDigit(m_char)) {
                        next();
                    }

                    if (skipDecimalFloatLiteralExponent() == PartiallySkipped) {
                        token->kind = Token::Error;

                        break;
                    }

                    if (hasOption(CNumericLiteralTypeSuffix) &&
                            skipCFloatLiteralTypeSuffix() == PartiallySkipped) {
                        token->kind = Token::Error;

                        break;
                    }

                    token->kind = Token::DecimalFloatLiteral;
                } else if ((m_char == 'e' || m_char == 'E') && hasOption(DecimalFloatLiteralToken)) {
                    if (skipDecimalFloatLiteralExponent() != FullySkipped) {
                        token->kind = Token::Error;

                        break;
                    }

                    if (hasOption(CNumericLiteralTypeSuffix) &&
                            skipCFloatLiteralTypeSuffix() == PartiallySkipped) {
                        token->kind = Token::Error;

                        break;
                    }

                    token->kind = Token::DecimalFloatLiteral;
                } else if (hasOption(DecimalIntegerLiteralToken)) {
                    if (hasOption(CNumericLiteralTypeSuffix) &&
                            skipCIntegerLiteralTypeSuffix() == PartiallySkipped) {
                        token->kind = Token::Error;

                        break;
                    }

                    token->kind = Token::DecimalIntegerLiteral;
                }
            } else {
                next();

                if ((m_char == 'x' || m_char == 'X') &&
                        (hasOption(HexadecimalIntegerLiteralToken) ||
                         hasOption(HexadecimalFloatLiteralToken))) {
                    next();

                    bool hasLeadingDigits = false;

                    while (isHexadecimalDigit(m_char)) {
                        next();

                        hasLeadingDigits = true;
                    }

                    if (m_char == '.' && hasOption(HexadecimalFloatLiteralToken)) {
                        next();

                        if (!hasLeadingDigits && !isHexadecimalDigit(m_char)) {
                            // There must be either leading or trailing digits
                            token->kind = Token::Error;

                            break;
                        }

                        next();

                        while (isHexadecimalDigit(m_char)) {
                            next();
                        }

                        if (skipHexadecimalFloatLiteralExponent() != FullySkipped) {
                            // Exponent is mandatory for hexadecimal float literal
                            token->kind = Token::Error;

                            break;
                        }

                        if (hasOption(CNumericLiteralTypeSuffix) &&
                                skipCFloatLiteralTypeSuffix() == PartiallySkipped) {
                            token->kind = Token::Error;

                            break;
                        }

                        token->kind = Token::HexadecimalFloatLiteral;
                    } else if (hasLeadingDigits &&
                               (m_char == 'p' || m_char == 'P') &&
                               hasOption(HexadecimalFloatLiteralToken)) {
                        if (skipHexadecimalFloatLiteralExponent() != FullySkipped) {
                            // Exponent is mandatory for hexadecimal float literal
                            token->kind = Token::Error;

                            break;
                        }

                        if (hasOption(CNumericLiteralTypeSuffix) &&
                                skipCFloatLiteralTypeSuffix() == PartiallySkipped) {
                            token->kind = Token::Error;

                            break;
                        }

                        token->kind = Token::HexadecimalFloatLiteral;
                    } else if (hasLeadingDigits && hasOption(HexadecimalIntegerLiteralToken)) {
                        if (hasOption(CNumericLiteralTypeSuffix) &&
                                skipCIntegerLiteralTypeSuffix() == PartiallySkipped) {
                            token->kind = Token::Error;

                            break;
                        }

                        token->kind = Token::HexadecimalIntegerLiteral;
                    }
                } else if ((m_char == 'o' || m_char == 'O') &&
                           hasOption(OctalIntegerLiteralToken) &&
                           hasOption(PythonAlternativeOctalIntegerLiteralNotation)) {
                    next();

                    if (!isOctalDigit(m_char)) {
                        token->kind = Token::Error;

                        break;
                    }

                    next();

                    while (isOctalDigit(m_char)) {
                        next();
                    }

                    if (hasOption(CNumericLiteralTypeSuffix) &&
                            skipCIntegerLiteralTypeSuffix() == PartiallySkipped) {
                        token->kind = Token::Error;

                        break;
                    }

                    token->kind = Token::OctalIntegerLiteral;
                } else if ((m_char == 'b' || m_char == 'B') &&
                            hasOption(BinaryIntegerLiteralToken)) {
                    next();

                    if (!isBinaryDigit(m_char)) {
                        token->kind = Token::Error;

                        break;
                    }

                    while (isBinaryDigit(m_char)) {
                        next();
                    }

                    if (hasOption(CNumericLiteralTypeSuffix) &&
                            skipCIntegerLiteralTypeSuffix() == PartiallySkipped) {
                        token->kind = Token::Error;

                        break;
                    }

                    token->kind = Token::BinaryIntegerLiteral;
                } else if (isOctalDigit(m_char) && hasOption(OctalIntegerLiteralToken)) {
                    next();

                    // FIXME: this culs also be the start of a DecimalFloatLiteralToken as it can have leading zeros
                    //        in the integer part. but this cannot be distinguished in this streaming lexer mechanic
                    //        that doesn't allow for infinite lookahead to find the potential dot

                    while (isOctalDigit(m_char)) {
                        next();
                    }

                    if (hasOption(CNumericLiteralTypeSuffix) &&
                            skipCIntegerLiteralTypeSuffix() == PartiallySkipped) {
                        token->kind = Token::Error;

                        break;
                    }

                    token->kind = Token::OctalIntegerLiteral;
                } else if (m_char == '.' && hasOption(DecimalFloatLiteralToken)) {
                    next();

                    if (skipDecimalFloatLiteralFraction(true) == PartiallySkipped) {
                        token->kind = Token::Error;

                        break;
                    }

                    if (hasOption(CNumericLiteralTypeSuffix) && skipCFloatLiteralTypeSuffix() == PartiallySkipped) {
                        token->kind = Token::Error;

                        break;
                    }

                    token->kind = Token::DecimalFloatLiteral;
                } else if (hasOption(DecimalIntegerLiteralToken)) {
                    if (hasOption(CNumericLiteralTypeSuffix) &&
                            skipCIntegerLiteralTypeSuffix() == PartiallySkipped) {
                        token->kind = Token::Error;

                        break;
                    }

                    token->kind = Token::DecimalIntegerLiteral;
                } else if (hasOption(OctalIntegerLiteralToken)) {
                    if (hasOption(CNumericLiteralTypeSuffix) &&
                            skipCIntegerLiteralTypeSuffix() == PartiallySkipped) {
                        token->kind = Token::Error;

                        break;
                    }

                    token->kind = Token::OctalIntegerLiteral;
                }
            }
        } else if (m_char == '"' && hasOption(CStringLiteralToken)) {
            m_state = InCStringLiteralToken;

            next();

            c = m_char;

            while (m_char != '\0' && (c == '\\' || m_char != '"')) {
                c = m_char;

                next();
            }

            if (c != '\\' && m_char == '"') {
                m_state = InNothing;

                next();
            }

            token->kind = Token::CStringLiteral;
        //} else if (m_char == '\'' && hasOption(CCharacterLiteralToken)) {
        } else {
            next();
        }

        break;
    }

exit:
    token->length = m_offset - token->offset;
}

// private
void Lexer::current()
{
    m_charLength = 0;

    if (m_offset >= m_input.length()) {
        m_char = '\0';

        return;
    }

    forever {
        // FIXME: this ignores surrogates and other Unicode fun: http://www.utf8everywhere.org/
        m_char = m_input.at(m_offset + m_charLength).unicode();
        m_charLength += 1;

        // C trigraphs are handled in translation phase 1 by the C preprocessor. Therefore the C compiler never seems
        // them and they have to be replaced here always.
        if (m_char == '?' &&
                hasOption(CTrigraph) &&
                m_offset + m_charLength + 1 < m_input.length() &&
                m_input.at(m_offset + m_charLength).unicode() == '?') {
            // "??=" -> "#"
            // "??<" -> "{"
            // "??>" -> "}"
            // "??(" -> "["
            // "??)" -> "]"
            // "??/" -> "\"
            // "??'" -> "^"
            // "??!" -> "|"
            // "??-" -> "~"
            switch (m_input.at(m_offset + m_charLength + 1).unicode()) {
            case '=':
                m_char = '#';

                break;

            case '<':
                m_char = '{';

                break;

            case '>':
                m_char = '}';

                break;

            case '(':
                m_char = ']';

                break;

            case ')':
                m_char = ']';

                break;

            case '/':
                m_char = '\\';

                break;

            case '\'':
                m_char = '^';

                break;

            case '!':
                m_char = '|';

                break;

            case '-':
                m_char = '~';

                break;
            }

            if (m_char != '?') {
                m_charLength += 2;
            }
        }

        // C digraphs are only replaced if they are not part of a string literal
        else if ((m_char == '<' || m_char == ':' || m_char == '%') &&
                 hasOption(CDigraph) &&
                 m_state != InCStringLiteralToken &&
                 m_offset + m_charLength < m_input.length()) {
            // "<:" -> "["
            // ":>" -> "]"
            // "<%" -> "{"
            // "%>" -> "}"
            // "%:" -> "#"
            switch (m_input.at(m_offset + m_charLength).unicode()) {
            case ':':
                if (m_char == '<') {
                    m_char = '[';
                } else if (m_char == '%') {
                    m_char = '#';
                }

                break;

            case '>':
                if (m_char == ':') {
                    m_char = ']';
                } else if (m_char == '%') {
                    m_char = '}';
                }

                break;

            case '%':
                if (m_char == '<') {
                    m_char = '{';
                }

                break;
            }

            if (m_char != '<' && m_char != ':' && m_char != '%') {
                m_charLength += 1;
            }
        }

        // C++ alternative tokens are only replaced if they are not part of a string literal
        else if ((m_char == 'a' || m_char == 'b' || m_char == 'c' || m_char == 'o' || m_char == 'n' || m_char == 'x') &&
                 hasOption(CPlusPlusAlternativeNotation) &&
                 m_state != InCStringLiteralToken &&
                 m_offset + m_charLength < m_input.length()) { // Shortest token is 2 chars long
            const QStringRef &digraph = m_input.midRef(m_offset + m_charLength - 1, 6); // Longest token is 6 chars long

            // "and"    -> "&&"
            // "and_eq" -> "&="
            // "bitand" -> "&"
            // "or"     -> "||"
            // "or_eq"  -> "|="
            // "bitor"  -> "|"
            // "xor"    -> "^"
            // "xor_eq" -> "^="
            // "not"    -> "!"
            // "not_eq" -> "!="
            // "compl"  -> "~"
            if (digraph == "and") {
                m_char = '&';
                m_nextCPlusPlusAlternativeChar = '&';
                m_nextCPlusPlusAlternativeCharLength = 2;
            } else if (digraph == "and_eq") {
                m_char = '&';
                m_charLength += 2;
                m_nextCPlusPlusAlternativeChar = '=';
                m_nextCPlusPlusAlternativeCharLength = 3;
            } else if (digraph == "bitand") {
                m_char = '&';
                m_charLength += 5;
            } else if (digraph == "or") {
                m_char = '|';
                m_nextCPlusPlusAlternativeChar = '|';
                m_nextCPlusPlusAlternativeCharLength = 1;
            } else if (digraph == "or_eq") {
                m_char = '|';
                m_charLength += 1;
                m_nextCPlusPlusAlternativeChar = '=';
                m_nextCPlusPlusAlternativeCharLength = 3;
            } else if (digraph == "bitor") {
                m_char = '|';
                m_charLength += 4;
            } else if (digraph == "xor") {
                m_char = '^';
                m_charLength += 2;
            } else if (digraph == "xor_eq") {
                m_char = '^';
                m_charLength += 2;
                m_nextCPlusPlusAlternativeChar = '=';
                m_nextCPlusPlusAlternativeCharLength = 3;
            } else if (digraph == "not") {
                m_char = '!';
                m_charLength += 2;
            } else if (digraph == "not_eq") {
                m_char = '!';
                m_charLength += 2;
                m_nextCPlusPlusAlternativeChar = '=';
                m_nextCPlusPlusAlternativeCharLength = 3;
            } else if (digraph == "compl") {
                m_char = '~';
                m_charLength += 4;
            }
        }

        // Pascal digraphs are only replaced if they are not part of a string literal // FIXME: is this correct?
        else if ((m_char == '(' || m_char == '.' || m_char == '*') &&
                 hasOption(PascalDigraph) &&
                 m_state != InPascalStringLiteralToken &&
                 m_offset + m_charLength < m_input.length()) {
            // "(." -> '['
            // ".)" -> ']'
            // "(*" -> '{'
            // "*)" -> '}'
            switch (m_input.at(m_offset + m_charLength).unicode()) {
            case '.':
                if (m_char == '(') {
                    m_char = '[';
                }

                break;

            case '*':
                if (m_char == '(') {
                    m_char = '{';
                }

                break;

            case ')':
                if (m_char == '.') {
                    m_char = ']';
                } else if (m_char == '*') {
                    m_char = '}';
                }

                break;
            }

            if (m_char != '(' && m_char != '.' && m_char != '*') {
                m_charLength += 1;
            }
        }

        if (m_char == '\\' &&
                hasOption(CLineContinuation) &&
                m_offset + m_charLength < m_input.length() &&
                m_input.at(m_offset + m_charLength).unicode() == '\n') {
            m_charLength += 1;

            continue;
        }

        return;
    }
}

// private
void Lexer::next()
{
    if (m_nextCPlusPlusAlternativeChar != '\0') {
        m_char = m_nextCPlusPlusAlternativeChar;
        m_offset += m_nextCPlusPlusAlternativeCharLength;

        m_nextCPlusPlusAlternativeChar = '\0';
        m_nextCPlusPlusAlternativeCharLength = 0;
    } else {
        m_offset += m_charLength;

        current();
    }
}

// private
Lexer::SkipResult Lexer::skipDecimalFloatLiteralFraction(bool hasLeadingDigits)
{
    Q_ASSERT(hasOption(DecimalFloatLiteralToken));

    if (!hasLeadingDigits) {
        if (!isDecimalDigit(m_char)) {
            return NothingSkipped;
        }

        next();
    }

    while (isDecimalDigit(m_char)) {
        next();
    }

    if (skipDecimalFloatLiteralExponent() == PartiallySkipped) {
        return PartiallySkipped;
    }

    return FullySkipped;
}

// private
Lexer::SkipResult Lexer::skipHexadecimalFloatLiteralExponent()
{
    Q_ASSERT(hasOption(HexadecimalFloatLiteralToken));

    return skipFloatLiteralExponent('p', 'P');
}

// private
Lexer::SkipResult Lexer::skipDecimalFloatLiteralExponent()
{
    Q_ASSERT(hasOption(DecimalFloatLiteralToken));

    return skipFloatLiteralExponent('e', 'E');
}

// private
Lexer::SkipResult Lexer::skipFloatLiteralExponent(uint lower, uint upper) // [<lower><upper>][+-]?[0-9]+
{
    Q_ASSERT(hasOption(HexadecimalFloatLiteralToken) || hasOption(DecimalFloatLiteralToken));

    if (m_char != lower && m_char != upper) {
        return NothingSkipped;
    }

    next();

    if (m_char == '+' || m_char == '-') {
        next();
    }

    if (!isDecimalDigit(m_char)) {
        return PartiallySkipped;
    }

    while (isDecimalDigit(m_char)) {
        next();
    }

    return FullySkipped;
}

// private
Lexer::SkipResult Lexer::skipCIntegerLiteralTypeSuffix()
{
    Q_ASSERT(hasOption(CNumericLiteralTypeSuffix));

    if (m_char == 'u' || m_char == 'U') {
        next();

        if (m_char == 'l') {
            next();

            if (m_char == 'l') {
                next();
            }
        } else if (m_char == 'L') {
            next();

            if (m_char == 'L') {
                next();
            }
        }

        return FullySkipped;
    } else if (m_char == 'l') {
        next();

        if (m_char == 'l') {
            next();

            if (m_char == 'u' || m_char == 'U') {
                next();
            }
        }

        return FullySkipped;
    } else if (m_char == 'L') {
        next();

        if (m_char == 'L') {
            next();

            if (m_char == 'u' || m_char == 'U') {
                next();
            }
        }

        return FullySkipped;
    }

    return NothingSkipped;
}

// private
Lexer::SkipResult Lexer::skipCFloatLiteralTypeSuffix()
{
    Q_ASSERT(hasOption(CNumericLiteralTypeSuffix));

    if (m_char == 'f' || m_char == 'F' || m_char == 'l' || m_char == 'L') {
        next();

        return FullySkipped;
    }

    return NothingSkipped;
}
