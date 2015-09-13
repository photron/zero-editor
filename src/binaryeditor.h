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

#ifndef BINARYEDITOR_H
#define BINARYEDITOR_H

#include "editor.h"
#include "binarydocument.h"
#include "binaryeditorwidget.h"

#include <QPointer>

class BinaryEditor : public Editor
{
    Q_OBJECT
    Q_DISABLE_COPY(BinaryEditor)

public:
    explicit BinaryEditor(BinaryDocument *document, QObject *parent = NULL);
    ~BinaryEditor();

    Document *document() const { return m_document; }
    QWidget *widget() const { return m_widget; }

    bool hasFeature(Feature feature) const { Q_UNUSED(feature) return false; }

    bool isWordWrapping() const { return false; }
    void setWordWrapping(bool enable) { Q_UNUSED(enable) }

    void toggleCase() { }

private:
    QPointer<BinaryDocument> m_document; // owned by DocumentManager
    QPointer<BinaryEditorWidget> m_widget; // owned by its parent widget if any
};

#endif // BINARYEDITOR_H