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

#include "location.h"

#include <QFlags>
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
    enum Option {
        KeepAfterClose = (1 << 0)
    };

    Q_DECLARE_FLAGS(Options, Option)

    enum Marker {
        CurrentMarker = (1 << 0),
        ModifiedMarker = (1 << 1),
        ClosedMarker = (1 << 2)
    };

    Q_DECLARE_FLAGS(Markers, Marker)

    explicit FilesWidget(QWidget *parent = NULL);

    void setOptions(const Options &options) { m_options = options; }
    void setOption(Option option, bool enable) { m_options.setFlag(option, enable); }

    Options options() const { return m_options; }
    bool hasOption(Option option) const { return m_options.testFlag(option); }

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
    void updateParentItemMarkers(QStandardItem *parent);
    void updateLocationOfSender();
    void updateModifiedMarkerOfSender();

private:
    enum {
        DocumentRole = Qt::UserRole,
        FilePathRole, // for reopening closed documents
        DocumentTypeRole, // for reopening closed documents
        TextCodecRole, // for reopening closed documents
        DirectoryPathRole, // for parents
        FileNameRole, // for children
        LowerCaseNameRole, // for sorting
        MarkersRole
    };

    void updateModifiedMarker(Document *document);
    void setMarker(QStandardItem *item, Marker marker, bool enable) const;
    void applyFilter();
    bool filterAcceptsChild(const QModelIndex &index) const;
    QStandardItem *findOrCreateParent(const Location &location);
    void takeChildFromParent(QStandardItem *child);

    Options m_options;

    QStandardItemModel m_model;
    QHash<Document *, QStandardItem *> m_openChildren; // values owned by QStandardItemModel
    QHash<Location, QStandardItem *> m_closedChildren; // values owned by QStandardItemModel
    QStandardItem *m_currentChild;
    QTreeView *m_treeFiles;

    bool m_showModifiedFilesOnly;

    bool m_filterEnabled;
    QRegularExpression m_filterRegularExpression;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(FilesWidget::Options)
Q_DECLARE_OPERATORS_FOR_FLAGS(FilesWidget::Markers)
Q_DECLARE_METATYPE(FilesWidget::Markers)

#endif // FILESWIDGET_H
