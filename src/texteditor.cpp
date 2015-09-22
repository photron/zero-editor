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

#include <QApplication>
#include <QClipboard>

TextEditor::TextEditor(TextDocument *document, QObject *parent) :
    Editor(parent),
    m_document(document),
    m_widget(new TextEditorWidget(document))
{
    m_pasteAvailable = m_widget->canPaste();
    m_selectAllAvailable = !m_document->document()->isEmpty();

    connect(m_document->document(), &QTextDocument::undoAvailable, this, &TextEditor::updateUndoActionAvailability);
    connect(m_document->document(), &QTextDocument::redoAvailable, this, &TextEditor::updateRedoActionAvailability);
    connect(m_widget, &QPlainTextEdit::selectionChanged, this, &TextEditor::updateSelectionActionsAvailability);
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &TextEditor::updatePasteActionAvailability);
    connect(m_document->document(), &QTextDocument::contentsChanged, this, &TextEditor::updateSelectAllActionAvailability);
}

TextEditor::~TextEditor()
{
    delete m_widget;
}

bool TextEditor::isActionAvailable(Action action) const
{
    switch (action) {
    case Undo:
        return m_document->document()->isUndoAvailable();

    case Redo:
        return m_document->document()->isRedoAvailable();

    case Cut:
        return m_widget->textCursor().hasSelection();

    case Copy:
        return m_widget->textCursor().hasSelection();

    case Paste:
        return m_widget->canPaste();

    case Delete:
        return m_widget->textCursor().hasSelection();

    case SelectAll:
        return !m_document->document()->isEmpty();

    case ToggleCase:
        return m_widget->textCursor().hasSelection();
    }

    Q_ASSERT(false);

    return false;
}

void TextEditor::undo()
{
    m_document->document()->undo();
}

void TextEditor::redo()
{
    m_document->document()->redo();
}

void TextEditor::cut()
{
    m_widget->cut();
}

void TextEditor::copy()
{
    m_widget->copy();
}

void TextEditor::paste()
{
    m_widget->paste();
}

void TextEditor::delete_()
{
    m_widget->textCursor().removeSelectedText();
}

void TextEditor::selectAll()
{
    m_widget->selectAll();
}

void TextEditor::toggleCase()
{
    QTextCursor textCursor = m_widget->textCursor();

    Q_ASSERT(textCursor.hasSelection());

    int position = textCursor.position();
    int anchor = textCursor.anchor();
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

bool TextEditor::hasFeature(Feature feature) const
{
    return feature == WordWrapping;
}

bool TextEditor::isWordWrapping() const
{
    return m_widget->wordWrapMode() != QTextOption::NoWrap;
}

void TextEditor::setWordWrapping(bool enable)
{
    m_widget->setWordWrapMode(enable ? QTextOption::WrapAtWordBoundaryOrAnywhere : QTextOption::NoWrap);
}

// private slot
void TextEditor::updateUndoActionAvailability(bool available)
{
    emit actionAvailabilityChanged(Undo, available);
}

// private slot
void TextEditor::updateRedoActionAvailability(bool available)
{
    emit actionAvailabilityChanged(Redo, available);
}

// private slot
void TextEditor::updateSelectionActionsAvailability()
{
    bool available = m_widget->textCursor().hasSelection();

    emit actionAvailabilityChanged(Cut, available);
    emit actionAvailabilityChanged(Copy, available);
    emit actionAvailabilityChanged(Delete, available);
    emit actionAvailabilityChanged(ToggleCase, available);
}

// private slot
void TextEditor::updatePasteActionAvailability()
{
    bool available = isActionAvailable(Paste);

    if (m_pasteAvailable != available) {
        m_pasteAvailable = available;

        emit actionAvailabilityChanged(Paste, m_pasteAvailable);
    }
}

// private slot
void TextEditor::updateSelectAllActionAvailability()
{
    bool available = isActionAvailable(SelectAll);

    if (m_selectAllAvailable != available) {
        m_selectAllAvailable = available;

        emit actionAvailabilityChanged(SelectAll, m_selectAllAvailable);
    }
}
