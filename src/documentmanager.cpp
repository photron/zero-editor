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

#include "documentmanager.h"

#include "binarydocument.h"
#include "binaryeditor.h"
#include "textdocument.h"
#include "texteditor.h"

#include <QDebug>

DocumentManager *DocumentManager::s_instance = NULL;

DocumentManager::DocumentManager(QObject *parent) :
    QObject(parent),
    m_currentDocument(NULL)
{
    s_instance = this;
}

DocumentManager::~DocumentManager()
{
    foreach (Document *document, m_documents) {
        delete document;
    }

    // FIXME: delete editors

    s_instance = NULL;
}

// static
bool DocumentManager::open(const QString &filePath, QString *error)
{
    Q_ASSERT(!filePath.isEmpty());
    Q_ASSERT(error != NULL);

    // FIXME: check if file path is already open. if yes make it the current document

    Document *document = new TextDocument;
    QString localError;

    if (document->open(filePath, &localError)) {
        Editor *editor = new TextEditor(static_cast<TextDocument *>(document));

        s_instance->m_documents.append(document);
        s_instance->m_editors.insert(document, editor);

        emit s_instance->documentOpened(document);

        setCurrentDocument(document);

        return true;
    } else if (!localError.isNull()) {
        delete document;

        *error = localError;

        return false;
    } else {
        document = new BinaryDocument;

        if (document->open(filePath, &localError)) {
            Editor *editor = new BinaryEditor(static_cast<BinaryDocument *>(document));

            s_instance->m_documents.append(document);
            s_instance->m_editors.insert(document, editor);

            emit s_instance->documentOpened(document);

            setCurrentDocument(document);

            return true;
        } else if (!localError.isNull()) {
            delete document;

            *error = localError;

            return false;
        } else {
            *error = QString("Could not open '%1': Unsupported document type").arg(filePath);

            return false;
        }
    }
}

// static
Editor *DocumentManager::editorForDocument(Document *document)
{
    Q_ASSERT(document != NULL);
    Q_ASSERT(s_instance->m_documents.contains(document));

    return s_instance->m_editors.value(document, NULL);
}

// static
void DocumentManager::setCurrentDocument(Document *document)
{
    Q_ASSERT(document != NULL);
    Q_ASSERT(s_instance->m_documents.contains(document));

    if (document != s_instance->m_currentDocument) {
        s_instance->m_currentDocument = document;

        emit s_instance->currentDocumentChanged(s_instance->m_currentDocument);
    }
}
