//
// Zero Editor
// Copyright (C) 2015-2018 Matthias Bolte <matthias.bolte@googlemail.com>
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
#include "encodingdialog.h"
#include "filedialog.h"
#include "mainwindow.h"
#include "textdocument.h"
#include "textcodec.h"
#include "texteditor.h"

#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

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
    TextDocument *document = new TextDocument(TextCodec::fromName("UTF-8"));
    TextEditor *editor = new TextEditor(document);

    s_instance->m_documents.append(document);
    s_instance->m_editors.insert(document, editor);

    connect(document, &Document::modificationChanged, s_instance, &DocumentManager::updateModificationCount);

    emit s_instance->opened(document);

    setCurrent(document);
}

// static
Document *DocumentManager::open(const Location &location, Document::Type type, TextCodec *codec)
{
    QString error;
    Document *document = open(location, type, codec, &error);

    if (document == NULL) {
        if (error.isEmpty()) {
            error = QString("Could not open \"%1\": Unknown error.").arg(location.path());
        }

        QMessageBox::critical(MainWindow::instance(), "Open File Error", error);
    }

    return document;
}

// static
Document *DocumentManager::open(const Location &location, Document::Type type, TextCodec *codec, QString *error)
{
    Q_ASSERT(!location.isEmpty());
    Q_ASSERT(type == Document::Text || codec == NULL);
    Q_ASSERT(error != NULL);

    if (find(location) != NULL) {
        *error = QString("File \"%1\" is already open.").arg(location.path());

        return NULL;
    }

    QFile file(location.path());

    if (!file.open(QIODevice::ReadOnly)) {
        *error = QString("Could not open \"%1\" for reading: %2").arg(location.path(), file.errorString());

        return NULL;
    }

    // FIXME: do this in chunks to avoid blocking the UI if the file is big
    const QByteArray &data = file.readAll();

    if (file.error() != QFile::NoError) {
        *error = QString("Could not read from \"%1\": %2").arg(location.path(), file.errorString());

        return NULL;
    }

    return load(location, type, data, codec, error);
}

// static
Document *DocumentManager::load(const Location &location, Document::Type type, const QByteArray &data, TextCodec *codec,
                                QString *error)
{
    Q_ASSERT(type == Document::Text || codec == NULL);
    Q_ASSERT(error != NULL);

    if (!location.isEmpty() && find(location) != NULL) {
        *error = QString("\"%1\" is already open.").arg(location.path());

        return NULL;
    }

    Document *document;
    Editor *editor;

    if (type == Document::Text) {
        TextDocument *textDocument = new TextDocument(codec, s_instance);

        textDocument->setLocation(location);

        if (!textDocument->load(data, error)) {
            delete textDocument;

            return NULL;
        }

        document = textDocument;
        editor = new TextEditor(textDocument);
    } else if (type == Document::Binary) {
        BinaryDocument *binaryDocument = new BinaryDocument(s_instance);

        binaryDocument->setLocation(location);

        if (!binaryDocument->load(data, error)) {
            delete binaryDocument;

            return NULL;
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

    return document;
}

// static
void DocumentManager::save(Document *document)
{
    Q_ASSERT(document != NULL);

    if (!document->isModified()) {
        return;
    }

    if (document->location().isEmpty()) {
        showSaveAsDialog(document);
    } else {
        saveAs(document, document->location());
    }
}

// static
void DocumentManager::saveAs(Document *document, const Location &location)
{
    Q_ASSERT(document != NULL);
    Q_ASSERT(!location.isEmpty());

    // FIXME: Need to deal with saving A as B while B already exists and is already open

    QFile file(location.path());
    QString error;

    if (!file.open(QIODevice::WriteOnly)) {
        error = QString("Could not open \"%1\" for writing: %2").arg(location.path(), file.errorString());

        QMessageBox::critical(MainWindow::instance(), "Save File Error", error);

        return;
    }

    QByteArray data;

    if (!document->save(&data, &error)) {
        if (error.isEmpty()) {
            error = QString("Could not save \"%1\": Unknown error.").arg(location.path());
        }

        QMessageBox::critical(MainWindow::instance(), "Save File Error", error);

        return;
    }

    if (file.write(data) < data.length()) {
        error = QString("Short write to \"%1\" occurred.").arg(location.path());

        QMessageBox::critical(MainWindow::instance(), "Save File Error", error);

        return;
    }

    document->setModified(false);
    document->setLocation(location);
}

// static
void DocumentManager::saveAll()
{
    foreach (Document *document, s_instance->m_documents) {
        save(document);
    }
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
Document *DocumentManager::find(const Location &location)
{
    Q_ASSERT(!location.isEmpty());

    foreach (Document *document, s_instance->m_documents) {
        if (document->location() == location) {
            return document;
        }
    }

    return NULL;
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

// static
void DocumentManager::showOpenDialog()
{
    const Location& suggestion = s_instance->m_current != NULL ? s_instance->m_current->location() : Location();
    const QList<Location> &locations = FileDialog::getOpenLocations(MainWindow::instance(), suggestion);

    foreach (const Location &location, locations) {
        Document *document = find(location);

        if (document != NULL) {
            setCurrent(document);

            continue;
        }

        open(location, Document::Text, NULL);
    }
}

// static
void DocumentManager::showSaveAsDialog(Document *document)
{
    Q_ASSERT(document != NULL);

    // Ensure that the open files widget shows this document as current, so that the user knows for which document the
    // "Save File" dialog showed up. Because this method might be called for a non-current document, for example, it
    // can be called as part of the saveAll method.
    setCurrent(document);

    const Location &oldLocation = document->location();
    Location suggestion;

    if (oldLocation.isEmpty()) {
        suggestion = Location::home().file("unnamed");
    } else {
        suggestion = oldLocation.path();
    }

    const Location &newLocation = FileDialog::getSaveLocation(MainWindow::instance(), suggestion);

    if (!newLocation.isEmpty()) {
        saveAs(document, newLocation);
    }
}

// static
void DocumentManager::showEncodingDialog(Document *document)
{
    Q_ASSERT(document != NULL);

    TextCodec *codec = NULL;
    bool hasDecodingError = false;

    if (document->type() == Document::Text) {
        TextDocument *textDocument = static_cast<TextDocument *>(document);

        codec = textDocument->codec();
        hasDecodingError = textDocument->hasDecodingError();
    }

    EncodingDialog dialog(codec, MainWindow::instance());

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    codec = dialog.codec();

    if (hasDecodingError) {
        // If the TextDocument has a decoding error then it was never correctly loaded in the first place and doesn't
        // contain the actual text from the underlying file in a reusable way. In this case reopen the file with the
        // selected codec. There are no unsaved changes to be lost.
        const Location &location = document->location();
        Document::Type type = codec != NULL ? Document::Text : Document::Binary;
        QString error;

        close(document);

        if (open(location, type, codec, &error) == NULL) {
            if (error.isEmpty()) {
                error = QString("Could not change encoding for \"%1\": Unknown error.").arg(location.path());
            }

            QMessageBox::critical(MainWindow::instance(), "Encoding Change Error", error);
        }
    } else if (document->type() == Document::Text) {
        TextDocument *textDocument = static_cast<TextDocument *>(document);

        if (codec != NULL) {
            // The codec for TextDocuments can be changed on-the-fly. If the codec cannot encode the contained text
            // then this will be detected the next time the document is saved and an error will be reported.
            textDocument->setCodec(codec);
        } else {
            const Location &location = document->location();
            bool modified = textDocument->isModified();
            QByteArray data;
            QString error;

            if (!document->save(&data, &error)) {
                if (error.isEmpty()) {
                    error = QString("Could not change encoding for \"%1\": Unknown error.").arg(location.path());
                }

                QMessageBox::critical(MainWindow::instance(), "Encoding Change Error", error);

                return;
            }

            close(document);

            error = QString();
            document = load(location, Document::Binary, data, NULL, &error);

            if (document == NULL) {
                if (error.isEmpty()) {
                    error = QString("Could not change encoding for \"%1\": Unknown error.").arg(location.path());
                }

                QMessageBox::critical(MainWindow::instance(), "Encoding Change Error", error);
            } else if (modified) {
                document->setModified(true);
            }
        }
    } else if (document->type() == Document::Binary) {
        if (codec != NULL) {
            const Location &location = document->location();
            bool modified = document->isModified();
            QByteArray data;
            QString error;

            if (!document->save(&data, &error)) {
                if (error.isEmpty()) {
                    error = QString("Could not change encoding for \"%1\": Unknown error.").arg(location.path());
                }

                QMessageBox::critical(MainWindow::instance(), "Encoding Change Error", error);

                return;
            }

            close(document);

            document = load(location, Document::Text, data, codec, &error);

            if (document == NULL) {
                if (error.isEmpty()) {
                    error = QString("Could not change encoding for \"%1\": Unknown error.").arg(location.path());
                }

                QMessageBox::critical(MainWindow::instance(), "Encoding Change Error", error);
            } else if (modified) {
                document->setModified(true);
            }
        }
    } else {
        Q_ASSERT(false);
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
