//
// Zero Editor
// Copyright (C) 2016 Matthias Bolte <matthias.bolte@googlemail.com>
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

#ifndef FILESWIDGET_H
#define FILESWIDGET_H

#include <QHash>
#include <QRegularExpression>
#include <QStandardItemModel>
#include <QWidget>

class QTreeView;
class Document;

class FilesWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(FilesWidget)

public:
    explicit FilesWidget(QWidget *parent = NULL);

signals:
    void filterValidityChanged(bool valid);

public slots:
    void showModifiedDocumentsOnly(bool enable);
    void setFilterEnabled(bool enable);
    void setFilterPattern(const QString &pattern);

private slots:
    void addDocument(Document *document);
    void removeDocument(Document *document);
    void setCurrentDocument(const QModelIndex &index);
    void setCurrentChild(Document *document);
    void updateParentIndexMarkers(const QModelIndex &index);
    void updateParentItemMarkers(QStandardItem *item);
    void updateLocationOfSender();
    void updateModificationMarkerOfSender();

private:
    enum {
        DocumentPointerRole = Qt::UserRole,
        DirectoryPathRole, // for parents
        FileNameRole, // for children
        LowerCaseNameRole // for sorting
    };

    void updateModificationMarker(Document *document);
    void markItemAsCurrent(QStandardItem *item, bool mark) const;
    void markItemAsModified(QStandardItem *item, bool mark) const;
    void applyFilter();
    bool filterAcceptsChild(const QModelIndex &index) const;

    QStandardItemModel m_model;
    QHash<Document *, QStandardItem *> m_children; // values owned by QStandardItemModel
    QStandardItem *m_currentChild;
    QTreeView *m_treeFiles;

    bool m_showModifiedFilesOnly;

    bool m_filterEnabled;
    QRegularExpression m_filterRegularExpression;
};

#endif // FILESWIDGET_H
