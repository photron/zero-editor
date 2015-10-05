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
#include <QDebug>
#include <QMenu>

TextEditor::TextEditor(TextDocument *document, QObject *parent) :
    Editor(parent),
    m_document(document),
    m_widget(new TextEditorWidget(document))
{
    m_pasteAvailable = m_widget->canPaste();
    m_selectAllAvailable = !m_document->document()->isEmpty();

    connect(m_document->document(), &QTextDocument::undoAvailable, this, &TextEditor::updateUndoActionAvailability);
    connect(m_document->document(), &QTextDocument::redoAvailable, this, &TextEditor::updateRedoActionAvailability);
    connect(m_widget.data(), &QPlainTextEdit::selectionChanged, this, &TextEditor::updateSelectionActionsAvailability);
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &TextEditor::updatePasteActionAvailability);
    connect(m_document->document(), &QTextDocument::contentsChanged, this, &TextEditor::updateSelectAllActionAvailability);

    m_widget->viewport()->installEventFilter(this);
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

bool TextEditor::hasFeature(Feature feature) const
{
    return feature == WordWrapping;
}

bool TextEditor::isWordWrapping() const
{
    return m_widget->wordWrapMode() != QTextOption::NoWrap;
}

// slot
void TextEditor::undo()
{
    m_document->document()->undo();
}

// slot
void TextEditor::redo()
{
    m_document->document()->redo();
}

// slot
void TextEditor::cut()
{
    m_widget->cut();
}

// slot
void TextEditor::copy()
{
    m_widget->copy();
}

// slot
void TextEditor::paste()
{
    m_widget->paste();
}

// slot
void TextEditor::delete_()
{
    m_widget->textCursor().removeSelectedText();
}

// slot
void TextEditor::selectAll()
{
    m_widget->selectAll();
}

// slot
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

// slot
void TextEditor::setWordWrapping(bool enable)
{
    m_widget->setWordWrapMode(enable ? QTextOption::WrapAtWordBoundaryOrAnywhere : QTextOption::NoWrap);
}

// protected
bool TextEditor::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::ContextMenu) {
        TextEditorWidget *editor = qobject_cast<TextEditorWidget *>(object->parent());

        if (editor != NULL) {
            QContextMenuEvent *contextMenuEvent = static_cast<QContextMenuEvent *>(event);
            QMenu *menu = editor->createStandardContextMenu();
            QList<QAction *> actions = menu->actions();

            for (int i = actions.length() - 1; i >= 0; --i) {
                QAction *action = actions.at(i);
                QString text(action->text().split('\t').first());

                // Qt only sets shortcuts for the QPlainTextEdit context menu QActions if there isn't another active
                // shortcut with the same key sequence. But because as such shortcuts still work if the QPlainTextEdit
                // has focus there is no reason to not have them set for the context menu QActions.
                if (text == "&Undo") {
                    action->setShortcut(QKeySequence::Undo);
                } else if (text == "&Redo") {
                    action->setShortcut(QKeySequence::Redo);
                } else if (text == "Cu&t") {
                    action->setShortcut(QKeySequence::Cut);
                } else if (text == "&Copy") {
                    action->setShortcut(QKeySequence::Copy);
                } else if (text == "&Paste") {
                    action->setShortcut(QKeySequence::Paste);
                } else if (text == "Select All") {
                    action->setShortcut(QKeySequence::SelectAll);

                    // Qt doesn't add an icon for the select all action. So it's set here.
                    action->setIcon(QIcon(":/icons/16x16/select-all.png"));
                }
            }

            menu->addSeparator();

            QAction *actionToggleCase = menu->addAction(QIcon(":/icons/16x16/toggle-case.png"), "Toggle Case");

            actionToggleCase->setShortcut(QKeySequence("Ctrl+U"));
            actionToggleCase->setEnabled(isActionAvailable(ToggleCase));

            connect(actionToggleCase, &QAction::triggered, this, &TextEditor::toggleCase);

            menu->setAttribute(Qt::WA_DeleteOnClose);
            menu->popup(contextMenuEvent->globalPos());

            return true;
        }
    }

    return Editor::eventFilter(object, event);
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
