//
// Zero Editor
// Copyright (C) 2016-2018 Matthias Bolte <matthias.bolte@googlemail.com>
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

#include "fileswidget.h"

#include "document.h"
#include "documentmanager.h"
#include "textcodec.h"
#include "textdocument.h"
#include "utils.h"

#include <QTreeView>
#include <QVBoxLayout>

FilesWidget::FilesWidget(QWidget *parent) :
    QWidget(parent),
    m_currentChild(NULL),
    m_treeFiles(new QTreeView(this)),
    m_showModifiedFilesOnly(false),
    m_filterEnabled(false)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_treeFiles);

    m_model.setSortRole(LowerCaseNameRole);

    // FIXME: enable edit triggers to allow renaming files on disk from the open documents tree?

    m_treeFiles->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_treeFiles->setEditTriggers(QTreeView::NoEditTriggers);
    m_treeFiles->setTextElideMode(Qt::ElideLeft);
    m_treeFiles->setHeaderHidden(true);
    m_treeFiles->setModel(&m_model);

    connect(m_treeFiles, &QTreeView::activated, this, &FilesWidget::setCurrentDocument);
    connect(m_treeFiles, &QTreeView::expanded, this, &FilesWidget::updateParentIndexMarkers);
    connect(m_treeFiles, &QTreeView::collapsed, this, &FilesWidget::updateParentIndexMarkers);

    connect(DocumentManager::instance(), &DocumentManager::opened, this, &FilesWidget::addDocument);
    connect(DocumentManager::instance(), &DocumentManager::aboutToBeClosed, this, &FilesWidget::removeDocument);
    connect(DocumentManager::instance(), &DocumentManager::currentChanged, this, &FilesWidget::setCurrentChild);
}

// slot
void FilesWidget::showModifiedDocumentsOnly(bool enable)
{
    if (m_showModifiedFilesOnly != enable) {
        m_showModifiedFilesOnly = enable;

        applyFilter();
    }
}

// slot
void FilesWidget::setFilterEnabled(bool enable)
{
    if (m_filterEnabled != enable) {
        m_filterEnabled = enable;

        applyFilter();
    }
}

// slot
void FilesWidget::setFilterPattern(const QString &pattern)
{
    if (m_filterRegularExpression.pattern() != pattern) {
        bool wasValid = m_filterRegularExpression.isValid();

        m_filterRegularExpression = QRegularExpression(pattern);

        bool isValid = m_filterRegularExpression.isValid();

        if (isValid != wasValid) {
            emit filterValidityChanged(isValid);
        }

        applyFilter();
    }
}

// private slot
void FilesWidget::addDocument(Document *document)
{
    Q_ASSERT(document != NULL);
    Q_ASSERT(document != DocumentManager::current());

    const Location &location = document->location();
    QStandardItem *child = m_closedChildren.value(location, NULL);

    Q_ASSERT(hasOption(KeepAfterClose) || child == NULL);

    QStandardItem *parent;

    if (child != NULL) {
        // Reopen closed child item
        parent = child->parent();

        child->setData(QVariant::fromValue(document), DocumentRole);
        child->setData(QVariant(), PathRole);
        child->setData(QVariant(), DocumentTypeRole);
        child->setData(QVariant(), TextCodecRole);

        m_closedChildren.remove(location);
        m_openChildren.insert(document, child);

        setMarker(child, ClosedMarker, false);
    } else {
        // Find or create parent item
        parent = findOrCreateParent(location);

        // Create child item
        const QString &fileName = location.fileName("unnamed");

        child = new QStandardItem(QIcon(":/icons/16x16/file.png"), fileName);

        child->setToolTip(location.path("unnamed"));
        child->setData(QVariant::fromValue(document), DocumentRole);
        child->setData(fileName, FileNameRole);
        child->setData(location.isEmpty() ? "" : fileName.toLower(), LowerCaseNameRole);
        child->setData(QVariant::fromValue(Markers()), MarkersRole);

        m_openChildren.insert(document, child);

        parent->appendRow(child);
        parent->sortChildren(0);
    }

    connect(document, &Document::locationChanged, this, &FilesWidget::updateLocationOfSender);
    connect(document, &Document::modificationChanged, this, &FilesWidget::updateModifiedMarkerOfSender);

    // Apply current filter
    if (!filterAcceptsChild(child->index())) {
        m_treeFiles->setRowHidden(child->row(), parent->index(), true);

        if (parent->rowCount() == 1) {
            m_treeFiles->setRowHidden(parent->row(), m_model.invisibleRootItem()->index(), true);
        }
    }

    // Show modification marker, if necessary
    updateModifiedMarker(document);
    updateParentItemMarkers(parent);
}

// private slot
void FilesWidget::removeDocument(Document *document)
{
    Q_ASSERT(document != NULL);
    Q_ASSERT(document != DocumentManager::current());

    disconnect(document, &Document::modificationChanged, this, &FilesWidget::updateModifiedMarkerOfSender);
    disconnect(document, &Document::locationChanged, this, &FilesWidget::updateLocationOfSender);

    QStandardItem *child = m_openChildren.value(document, NULL);

    Q_ASSERT(child != NULL);
    Q_ASSERT(child != m_currentChild);

    const Location &location = document->location();

    if (hasOption(KeepAfterClose) && !location.isEmpty()) {
        child->setData(QVariant::fromValue(NULL), DocumentRole);
        child->setData(location.path(), PathRole);
        child->setData(QVariant::fromValue(document->type()), DocumentTypeRole);

        if (document->type() == Document::Text) {
            child->setData(QVariant::fromValue(static_cast<TextDocument *>(document)->codec()), TextCodecRole);
        } else {
            child->setData(QVariant::fromValue(NULL), TextCodecRole);
        }

        m_openChildren.remove(document);
        m_closedChildren.insert(location, child);

        setMarker(child, ModifiedMarker, false);
        setMarker(child, ClosedMarker, true);
        updateParentItemMarkers(child->parent());
    } else {
        takeChildFromParent(child);

        m_openChildren.remove(document);

        delete child;
    }
}

// private slot
void FilesWidget::setCurrentDocument(const QModelIndex &index)
{
    Q_ASSERT(index.isValid());

    if (!index.parent().isValid()) {
        return; // ignore activation of parent item
    }

    Document *document = index.data(DocumentRole).value<Document *>();

    if (hasOption(KeepAfterClose)) {
        if (document != NULL) {
            DocumentManager::setCurrent(document);
        } else {
            const QString &filePath = index.data(PathRole).value<QString>();
            Document::Type type = index.data(DocumentTypeRole).value<Document::Type>();
            TextCodec *codec = index.data(TextCodecRole).value<TextCodec *>();

            Q_ASSERT(type != Document::Text || codec != NULL);

            DocumentManager::open(filePath, type, codec);
        }
    } else {
        Q_ASSERT(document != NULL);

        DocumentManager::setCurrent(document);
    }
}

// private slot
void FilesWidget::setCurrentChild(Document *document)
{
    // Set current child and parent back to normal
    if (m_currentChild != NULL) {
        setMarker(m_currentChild, CurrentMarker, false);

        QStandardItem *parent = m_currentChild->parent();

        m_treeFiles->scrollTo(parent->index());
        m_treeFiles->selectionModel()->select(parent->index(), QItemSelectionModel::ClearAndSelect);

        m_currentChild = NULL;

        updateParentItemMarkers(parent);
    }

    // Mark new current child as current
    if (document != NULL) {
        QStandardItem *child = m_openChildren.value(document, NULL);

        Q_ASSERT(child != NULL);

        QStandardItem *parent = child->parent();

        Q_ASSERT(parent != NULL);

        m_treeFiles->expand(parent->index());
        m_treeFiles->scrollTo(child->index());
        m_treeFiles->selectionModel()->select(child->index(), QItemSelectionModel::ClearAndSelect);

        m_currentChild = child;

        setMarker(m_currentChild, CurrentMarker, true);
        updateParentItemMarkers(parent);
    }
}

// private slot
void FilesWidget::updateParentIndexMarkers(const QModelIndex &index)
{
    Q_ASSERT(index.isValid());

    updateParentItemMarkers(m_model.itemFromIndex(index));
}

// private slot
void FilesWidget::updateParentItemMarkers(QStandardItem *parent)
{
    Q_ASSERT(parent != NULL);
    Q_ASSERT(parent->parent() == NULL);

    if (m_treeFiles->isExpanded(parent->index())) {
        // Check if this is the parent of the hidden current child
        bool hasHiddenCurrentChild = false;

        if (m_currentChild != NULL && m_currentChild->parent() == parent) {
            hasHiddenCurrentChild = m_treeFiles->isRowHidden(m_currentChild->row(), parent->index());
        }

        setMarker(parent, CurrentMarker, hasHiddenCurrentChild);

        // Check if this is a parent of hidden modified children
        int childRowCount = parent->rowCount();
        bool hasHiddenModifiedChild = false;

        for (int childRow = 0; childRow < childRowCount; ++childRow) {
            QStandardItem *child = parent->child(childRow);
            Document *document = child->data(DocumentRole).value<Document *>();

            if (m_treeFiles->isRowHidden(childRow, parent->index()) && document != NULL && document->isModified()) {
                hasHiddenModifiedChild = true;
                break;
            }
        }

        setMarker(parent, ModifiedMarker, hasHiddenModifiedChild);
        setMarker(parent, ClosedMarker, false);
    } else {
        // Check if this is the parent of the current child
        setMarker(parent, CurrentMarker, m_currentChild != NULL && m_currentChild->parent() == parent);

        // Check if this is a parent of modified children or has only closed children
        int childRowCount = parent->rowCount();
        bool hasModifiedChild = false;
        bool hasClosedChild = false;

        for (int childRow = 0; childRow < childRowCount; ++childRow) {
            QStandardItem *child = parent->child(childRow);
            Document *document = child->data(DocumentRole).value<Document *>();

            if (document != NULL && document->isModified()) {
                hasModifiedChild = true;
            }

            if (document == NULL) {
                hasClosedChild = true;
            }

            if (hasModifiedChild && hasClosedChild) {
                break;
            }
        }

        setMarker(parent, ModifiedMarker, hasModifiedChild);
        setMarker(parent, ClosedMarker, hasClosedChild);
    }
}

// private slot
void FilesWidget::updateLocationOfSender()
{
    Document *document = qobject_cast<Document *>(sender());

    Q_ASSERT(document != NULL);

    QStandardItem *child = m_openChildren.value(document, NULL);

    Q_ASSERT(child != NULL);

    const Location &location = document->location();

    Q_ASSERT(!location.isEmpty());

    QStandardItem *parent = child->parent();

    if (parent->data(DirectoryPathRole).value<QString>() != location.directoryPath()) {
        takeChildFromParent(child);

        parent = findOrCreateParent(location);

        parent->appendRow(child);

        updateParentItemMarkers(child->parent());
    }

    const QString &fileName = location.fileName();

    child->setText(fileName);
    child->setToolTip(location.path());
    child->setData(fileName, FileNameRole);
    child->setData(fileName.toLower(), LowerCaseNameRole);

    parent->sortChildren(0);

    m_treeFiles->expand(parent->index());
    m_treeFiles->scrollTo(child->index());
    m_treeFiles->selectionModel()->select(child->index(), QItemSelectionModel::ClearAndSelect);

    applyFilter();
}

// private slot
void FilesWidget::updateModifiedMarkerOfSender()
{
    updateModifiedMarker(qobject_cast<Document *>(sender()));
}

// private
void FilesWidget::updateModifiedMarker(Document *document)
{
    Q_ASSERT(document != NULL);

    QStandardItem *child = m_openChildren.value(document, NULL);

    Q_ASSERT(child != NULL);

    setMarker(child, ModifiedMarker, document->isModified());
    updateParentItemMarkers(child->parent());
}

// private
void FilesWidget::setMarker(QStandardItem *item, Marker marker, bool enable) const
{
    Q_ASSERT(item != NULL);

    Markers markers = item->data(MarkersRole).value<Markers>();

    if (markers.testFlag(marker) == enable) {
        return;
    }

    markers.setFlag(marker, enable);

    item->setData(QVariant::fromValue(markers), MarkersRole);

    if (marker == CurrentMarker) {
        Utils::setFontUnderline(item, enable);
    } else if (marker == ModifiedMarker) {
        QString text;

        if (item->parent() == NULL) {
            text = item->data(DirectoryPathRole).value<QString>();
        } else {
            text = item->data(FileNameRole).value<QString>();
        }

        if (enable) {
            item->setText(text + "*");
            item->setForeground(Qt::red);
        } else {
            item->setText(text);

            if (markers.testFlag(ClosedMarker)) {
                item->setForeground(Qt::darkGray);
            } else {
                item->setForeground(palette().color(QPalette::Text));
            }
        }
    } else if (marker == ClosedMarker) {
        if (!markers.testFlag(ModifiedMarker)) {
            if (enable) {
                item->setForeground(Qt::darkGray);
            } else {
                item->setForeground(palette().color(QPalette::Text));
            }
        }
    }
}

// private
void FilesWidget::applyFilter()
{
    QStandardItem *root = m_model.invisibleRootItem();
    int parentRowCount = root->rowCount();

    for (int parentRow = 0; parentRow < parentRowCount; ++parentRow) {
        QStandardItem *parent = root->child(parentRow);
        int childRowCount = parent->rowCount();
        bool hideParent = true;

        for (int childRow = 0; childRow < childRowCount; ++childRow) {
            QStandardItem *child = parent->child(childRow);
            bool hideChild = !filterAcceptsChild(child->index());

            if (!hideChild) {
                hideParent = false;
            }

            m_treeFiles->setRowHidden(childRow, parent->index(), hideChild);
        }

        updateParentItemMarkers(parent);

        m_treeFiles->setRowHidden(parentRow, root->index(), hideParent);
    }
}

// private
bool FilesWidget::filterAcceptsChild(const QModelIndex &index) const
{
    Q_ASSERT(index.isValid());

    if (!m_showModifiedFilesOnly && !m_filterEnabled) {
        return true;
    }

    Document *document = index.data(DocumentRole).value<Document *>();

    if (m_showModifiedFilesOnly && (document == NULL || !document->isModified())) {
        return false;
    }

    if (m_filterEnabled && m_filterRegularExpression.isValid() && !m_filterRegularExpression.pattern().isEmpty()) {
        const QString &fileName = index.data(FileNameRole).value<QString>();

        return m_filterRegularExpression.match(fileName).hasMatch();
    }

    return true;
}

// private
QStandardItem *FilesWidget::findOrCreateParent(const Location &location)
{
    const QString &directoryPath = location.directoryPath("unnamed");
    const QModelIndexList &parents = m_model.match(m_model.index(0, 0, QModelIndex()), DirectoryPathRole,
                                                   directoryPath, 1, Qt::MatchExactly);
    QStandardItem *parent;

    if (parents.isEmpty()) {
        parent = new QStandardItem(QIcon(":/icons/16x16/folder.png"), directoryPath);

        parent->setToolTip(directoryPath);
        parent->setData(directoryPath, DirectoryPathRole);
        parent->setData(location.isEmpty() ? "" : directoryPath.toLower(), LowerCaseNameRole);
        parent->setData(QVariant::fromValue(Markers()), MarkersRole);

        m_model.appendRow(parent);
        m_model.sort(0);
    } else {
        parent = m_model.itemFromIndex(parents.first());
    }

    return parent;
}

// private
void FilesWidget::takeChildFromParent(QStandardItem *child)
{
    Q_ASSERT(child != NULL);

    QStandardItem *parent = child->parent();

    Q_ASSERT(parent != NULL);

    // Save currently selected item, because the QStandardItem::removeRow call seems to affect the selection even if
    // the currently selected item is not the one being removed
    const QItemSelection &selection = m_treeFiles->selectionModel()->selection();
    QStandardItem *selectedItem = NULL;

    if (!selection.isEmpty()) {
        selectedItem = m_model.itemFromIndex(selection.first().indexes().first());

        Q_ASSERT(selectedItem != NULL);

        if (selectedItem == child) {
            selectedItem = NULL;
        }
    }

    parent->takeRow(child->row());

    if (parent->rowCount() > 0) {
        updateParentItemMarkers(parent);
    } else {
        if (selectedItem == parent) {
            selectedItem = NULL;
        }

        m_model.removeRow(parent->row());
    }

    if (selectedItem != NULL) {
        m_treeFiles->selectionModel()->select(selectedItem->index(), QItemSelectionModel::ClearAndSelect);
    }
}
