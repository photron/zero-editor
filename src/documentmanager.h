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

#ifndef DOCUMENTMANAGER_H
#define DOCUMENTMANAGER_H

#include <QHash>
#include <QObject>

#include "document.h"

class Editor;

class DocumentManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DocumentManager)

public:
    explicit DocumentManager(QObject *parent = NULL);
    ~DocumentManager();

    static DocumentManager *instance() { return s_instance; }

    static void create();
    static bool open(const QString &filePath, Document::Type type, QTextCodec *codec, QString *error);
    static void close(Document *document);

    static Editor *editor(Document *document);

    static Document *current() { return s_instance->m_current; }
    static void setCurrent(Document *document);

    static int modificationCount() { return s_instance->m_modificationCount; }

signals:
    void opened(Document *document);
    void aboutToBeClosed(Document *document);
    void currentChanged(Document *document);
    void modificationCountChanged(int count);

private slots:
    void updateModificationCount();

private:
    static DocumentManager *s_instance;

    QList<Document *> m_documents;
    QHash<Document *, Editor *> m_editors;
    Document *m_current; // can be NULL if there is no current document
    int m_modificationCount;
};

#endif // DOCUMENTMANAGER_H
