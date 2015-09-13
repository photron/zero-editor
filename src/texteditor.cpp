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

#include "texteditor.h"

TextEditor::TextEditor(TextDocument *document, QObject *parent) :
    Editor(parent),
    m_document(document),
    m_widget(new TextEditorWidget(document))
{
}

TextEditor::~TextEditor()
{
    delete m_widget;
}

bool TextEditor::hasFeature(Feature feature) const
{
    return feature == WordWrapping || feature == ToggleCase;
}

bool TextEditor::isWordWrapping() const
{
    return m_widget->wordWrapMode() != QTextOption::NoWrap;
}

void TextEditor::setWordWrapping(bool enable)
{
    m_widget->setWordWrapMode(enable ? QTextOption::WrapAtWordBoundaryOrAnywhere : QTextOption::NoWrap);
}

void TextEditor::toggleCase()
{
    QTextCursor textCursor = m_widget->textCursor();
    int position = textCursor.position();
    int anchor = textCursor.anchor();

    if (!textCursor.hasSelection()) {
        textCursor.select(QTextCursor::WordUnderCursor);
    }

    QString original = textCursor.selectedText();
    QString upper = original.toUpper();
    QString toggled;

    if (original == upper) {
        toggled = original.toLower();
    } else {
        toggled = upper;
    }

    if (toggled == original) {
        return;
    }

    textCursor.insertText(toggled);
    textCursor.setPosition(anchor);
    textCursor.setPosition(position, QTextCursor::KeepAnchor);

    m_widget->setTextCursor(textCursor);
}
