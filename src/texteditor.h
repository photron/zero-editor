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

#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include "editor.h"
#include "textdocument.h"
#include "texteditorwidget.h"

#include <QPointer>

class TextEditor : public Editor
{
    Q_OBJECT
    Q_DISABLE_COPY(TextEditor)

public:
    explicit TextEditor(TextDocument *document, QObject *parent = NULL);
    ~TextEditor();

    Document *document() const { return m_document; }
    QWidget *widget() const { return m_widget; }

    bool isActionAvailable(Action action) const;

    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void delete_();
    void selectAll();
    void toggleCase();

    bool hasFeature(Feature feature) const;

    bool isWordWrapping() const;
    void setWordWrapping(bool enable);

private slots:
    void updateUndoActionAvailability(bool available);
    void updateRedoActionAvailability(bool available);
    void updateSelectionActionsAvailability();
    void updatePasteActionAvailability();
    void updateSelectAllActionAvailability();

private:
    QPointer<TextDocument> m_document; // owned by DocumentManager
    QPointer<TextEditorWidget> m_widget; // owned by its parent widget if any

    bool m_pasteAvailable;
    bool m_selectAllAvailable;
};

#endif // TEXTEDITOR_H
