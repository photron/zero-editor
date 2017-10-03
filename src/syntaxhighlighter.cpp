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

#include "syntaxhighlighter.h"

#include "lexer.h"

#include <QFont>

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent) :
    QSyntaxHighlighter(parent)
{
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    QStringList keywords;

    keywords << "int";
    keywords << "void";
    keywords << "struct";
    keywords << "char";

    QTextCharFormat keywordFormat;

    keywordFormat.setFontWeight(QFont::Bold);
    keywordFormat.setForeground(Qt::darkMagenta);

    QTextCharFormat commentFormat;

    commentFormat.setForeground(Qt::red);

    QTextCharFormat whitespaceFormat;

    whitespaceFormat.setForeground(Qt::lightGray);

    setCurrentBlockState(Lexer::InNothing);

    int state = previousBlockState();

    Lexer lexer(text, state >= 0 ? (Lexer::State)state : Lexer::InNothing);

    lexer.setOption(Lexer::WhitespaceToken, true);
    lexer.setOption(Lexer::IdentifierToken, true);
    lexer.setOption(Lexer::CCommentToken, true);

    Token token;

    lexer.scan(&token);

    while (token.kind != Token::EndOfInput) {
        if (token.kind == Token::Identifier) {
            const QString &identifier = text.mid(token.offset, token.length);

            if (keywords.contains(identifier)) {
                setFormat(token.offset, token.length, keywordFormat);
            }
        } else if (token.kind == Token::CComment) {
            setFormat(token.offset, token.length, commentFormat);
        } else if (token.kind == Token::Whitespace) {
            setFormat(token.offset, token.length, whitespaceFormat);
        }

        lexer.scan(&token);
    }

    setCurrentBlockState(lexer.state());
}
