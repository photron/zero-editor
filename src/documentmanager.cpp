//
// Zero Editor
// Copyright (C) 2015-2016 Matthias Bolte <matthias.bolte@googlemail.com>
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
#include <QFileInfo>

DocumentManager *DocumentManager::s_instance = NULL;

DocumentManager::DocumentManager(QObject *parent) :
    QObject(parent),
    m_current(NULL)
{
    s_instance = this;
}

DocumentManager::~DocumentManager()
{
    foreach (Document *document, m_documents) {
        delete s_instance->m_editors.take(document);
    }

    s_instance = NULL;
}

// static
void DocumentManager::create()
{
    TextDocument *document = new TextDocument;
    TextEditor *editor = new TextEditor(document);

    s_instance->m_documents.append(document);
    s_instance->m_editors.insert(document, editor);

    connect(document, &Document::modificationChanged, s_instance, &DocumentManager::updateModificationCount);

    emit s_instance->opened(document);

    setCurrent(document);
}

// Returns true if the given file was successfully opened. Returns false if an error occurred while opening the file
// and sets error to a non-null QString describing the error.
//
// static
bool DocumentManager::open(const QString &filePath, Document::Type type, QTextCodec *codec, QString *error)
{
    Q_ASSERT(!filePath.isEmpty());
    Q_ASSERT(type == Document::Text || codec == NULL);
    Q_ASSERT(error != NULL);

    // Check if a document for the given file is already open with the requested type and codec. If yes make it the
    // current document, otherwise (re-)open it with the requested type and codec. Use QFileInfo for file path
    // comparison to avoid treating file paths as different if they only differ in case but are located on a caseless
    // file system are actually identical. QFileInfo handles this case correctly.
    QFileInfo fileInfo(filePath);

    foreach (Document *other, s_instance->m_documents) {
        if (QFileInfo(other->filePath()) == fileInfo) {
            // Reopen document, if type doesn't match
            if (other->type() != type) {
                close(other); // FIXME: need to preserve document content, if modified

                break;
            }

            // Reopen text document, if codec is specified, but doesn't match
            if (other->type() == Document::Text && codec != NULL) {
                TextDocument *document = qobject_cast<TextDocument *>(other);

                Q_ASSERT(document != NULL);

                if (document->codec() != codec) {
                    close(other); // FIXME: need to preserve document content, if modified

                    break;
                }
            }

            setCurrent(other);

            return true;
        }
    }

    Document *document;
    Editor *editor;

    if (type == Document::Text) {
        TextDocument *textDocument = new TextDocument;

        if (!textDocument->open(filePath, codec, error)) {
            delete textDocument;

            return false;
        }

        document = textDocument;
        editor = new TextEditor(textDocument);
    } else if (type == Document::Binary) {
        BinaryDocument *binaryDocument = new BinaryDocument;

        if (!binaryDocument->open(filePath, error)) {
            delete binaryDocument;

            return false;
        }

        document = binaryDocument;
        editor = new BinaryEditor(binaryDocument);
    } else {
        Q_ASSERT(false);
    }

    s_instance->m_documents.append(document);
    s_instance->m_editors.insert(document, editor);

    connect(document, &Document::modificationChanged, s_instance, &DocumentManager::updateModificationCount);

    emit s_instance->opened(document);

    setCurrent(document);

    return true;
}

// static
void DocumentManager::close(Document *document)
{
    Q_ASSERT(document != NULL);
    Q_ASSERT(s_instance->m_documents.contains(document));

    if (s_instance->m_current == document) {
        foreach (Document *other, s_instance->m_documents) {
            if (other != document) {
                setCurrent(other);
                break;
            }
        }

        if (s_instance->m_current == document) {
            setCurrent(NULL);
        }
    }

    emit s_instance->aboutToBeClosed(document);

    s_instance->m_documents.removeAll(document);

    // Need to delete-later the editor and the document here to avoid deleting them too early in the middle of a reopen
    // cycle, which in turn would trigger a segfault.
    s_instance->m_editors.take(document)->deleteLater();

    s_instance->updateModificationCount();
}

// static
Editor *DocumentManager::editor(Document *document)
{
    Q_ASSERT(document != NULL);
    Q_ASSERT(s_instance->m_documents.contains(document));

    return s_instance->m_editors.value(document, NULL);
}

// static
void DocumentManager::setCurrent(Document *document)
{
    Q_ASSERT(document == NULL || s_instance->m_documents.contains(document));

    if (document != s_instance->m_current) {
        s_instance->m_current = document;

        emit s_instance->currentChanged(s_instance->m_current);
    }
}

// private slot
void DocumentManager::updateModificationCount()
{
    int modificationCount = 0;

    foreach (Document *document, m_documents) {
        if (document->isModified()) {
            ++modificationCount;
        }
    }

    if (m_modificationCount != modificationCount) {
        m_modificationCount = modificationCount;

        emit modificationCountChanged(m_modificationCount);
    }
}
