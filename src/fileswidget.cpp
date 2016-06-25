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

#include "fileswidget.h"

#include "document.h"
#include "documentmanager.h"

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
    const QString &filePath = location.filePath("unnamed");
    const QString &directoryPath = location.directoryPath("unnamed");
    const QString &fileName = location.fileName("unnamed");

    // Find or create parent item
    const QModelIndexList &parents = m_model.match(m_model.index(0, 0, QModelIndex()), DirectoryPathRole,
                                                   directoryPath, 1, Qt::MatchExactly);
    QStandardItem *parent;

    if (parents.isEmpty()) {
        parent = new QStandardItem(QIcon(":/icons/16x16/folder.png"), directoryPath);

        parent->setToolTip(directoryPath);
        parent->setData(directoryPath, DirectoryPathRole);
        parent->setData(location.isEmpty() ? "" : directoryPath.toLower(), LowerCaseNameRole);

        m_model.appendRow(parent);
        m_model.sort(0);
    } else {
        parent = m_model.itemFromIndex(parents.first());
    }

    // Create child item
    QStandardItem *child = new QStandardItem(QIcon(":/icons/16x16/file.png"), fileName);

    child->setToolTip(filePath);
    child->setData(qVariantFromValue((void *)document), DocumentPointerRole);
    child->setData(fileName, FileNameRole);
    child->setData(location.isEmpty() ? "" : fileName.toLower(), LowerCaseNameRole);

    m_children.insert(document, child);

    parent->appendRow(child);
    parent->sortChildren(0);

    connect(document, &Document::locationChanged, this, &FilesWidget::updateLocationOfSender);
    connect(document, &Document::modificationChanged, this, &FilesWidget::updateModificationMarkerOfSender);

    // Apply current filter
    if (!filterAcceptsChild(child->index())) {
        m_treeFiles->setRowHidden(child->row(), parent->index(), true);

        if (parent->rowCount() == 1) {
            m_treeFiles->setRowHidden(parent->row(), m_model.invisibleRootItem()->index(), true);
        }
    }

    // Show modification marker, if necessary
    updateModificationMarker(document);
    updateParentItemMarkers(parent);
}

// private slot
void FilesWidget::removeDocument(Document *document)
{
    Q_ASSERT(document != NULL);
    Q_ASSERT(document != DocumentManager::current());

    disconnect(document, &Document::modificationChanged, this, &FilesWidget::updateModificationMarkerOfSender);
    disconnect(document, &Document::locationChanged, this, &FilesWidget::updateLocationOfSender);

    QStandardItem *child = m_children.value(document, NULL);

    Q_ASSERT(child != NULL);
    Q_ASSERT(child != m_currentChild);

    QStandardItem *parent = child->parent();

    Q_ASSERT(parent != NULL);

    // Save currently selected item, because the QStandardItem::removeRow call seems to affect the selection even
    // if the DocumentManager takes care of not closing the current document. This problem only occurs if the
    // current document is selected before its closing is triggered. If the current document is not before its
    // closing is triggered then the selection behaves as expected and is not affected by the removeRow call.
    const QItemSelection &selection = m_treeFiles->selectionModel()->selection();
    QStandardItem *selectedItem = NULL;

    if (!selection.isEmpty()) {
        selectedItem = m_model.itemFromIndex(selection.first().indexes().first());

        Q_ASSERT(selectedItem != NULL);
        Q_ASSERT(selectedItem != child);
    }

    parent->removeRow(child->row());

    if (parent->rowCount() > 0) {
        updateParentItemMarkers(parent);
    } else {
        if (selectedItem == parent) {
            selectedItem = NULL;
        }

        m_model.removeRow(parent->row());
    }

    m_children.remove(document);

    if (selectedItem != NULL) {
        m_treeFiles->selectionModel()->select(selectedItem->index(), QItemSelectionModel::ClearAndSelect);
    }
}

// private slot
void FilesWidget::setCurrentDocument(const QModelIndex &index)
{
    Q_ASSERT(index.isValid());

    if (!index.parent().isValid()) {
        return; // ignore activation of parent item
    }

    Document *document = static_cast<Document *>(index.data(DocumentPointerRole).value<void *>());

    Q_ASSERT(document != NULL);

    DocumentManager::setCurrent(document);
}

// private slot
void FilesWidget::setCurrentChild(Document *document)
{
    // Set current child and parent back to normal
    if (m_currentChild != NULL) {
        markItemAsCurrent(m_currentChild, false);

        QStandardItem *parent = m_currentChild->parent();

        m_treeFiles->scrollTo(parent->index());
        m_treeFiles->selectionModel()->select(parent->index(), QItemSelectionModel::ClearAndSelect);

        m_currentChild = NULL;

        updateParentItemMarkers(parent);
    }

    // Mark new current child as current
    if (document != NULL) {
        QStandardItem *child = m_children.value(document, NULL);

        Q_ASSERT(child != NULL);

        QStandardItem *parent = child->parent();

        Q_ASSERT(parent != NULL);

        m_treeFiles->expand(parent->index());
        m_treeFiles->scrollTo(child->index());
        m_treeFiles->selectionModel()->select(child->index(), QItemSelectionModel::ClearAndSelect);

        m_currentChild = child;

        markItemAsCurrent(m_currentChild, true);
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
void FilesWidget::updateParentItemMarkers(QStandardItem *item)
{
    Q_ASSERT(item != NULL);
    Q_ASSERT(item->parent() == NULL);

    if (m_treeFiles->isExpanded(item->index())) {
        // Check if this is the parent of the hidden current child
        bool hasHiddenCurrentChild = false;

        if (m_currentChild != NULL && m_currentChild->parent() == item) {
            hasHiddenCurrentChild = m_treeFiles->isRowHidden(m_currentChild->row(), item->index());
        }

        markItemAsCurrent(item, hasHiddenCurrentChild);

        // Check if this is a parent of hidden modified children
        int childRowCount = item->rowCount();
        bool hasHiddenModifiedChild = false;

        for (int childRow = 0; childRow < childRowCount; ++childRow) {
            QStandardItem *child = item->child(childRow);
            Document *document = static_cast<Document *>(child->data(DocumentPointerRole).value<void *>());

            Q_ASSERT(document != NULL);

            if (m_treeFiles->isRowHidden(childRow, item->index()) && document->isModified()) {
                hasHiddenModifiedChild = true;
                break;
            }
        }

        markItemAsModified(item, hasHiddenModifiedChild);
    } else {
        // Check if this is the parent of the current child
        markItemAsCurrent(item, m_currentChild != NULL && m_currentChild->parent() == item);

        // Check if this is a parent of modified children
        int childRowCount = item->rowCount();
        bool hasModifiedChild = false;

        for (int childRow = 0; childRow < childRowCount; ++childRow) {
            QStandardItem *child = item->child(childRow);
            Document *document = static_cast<Document *>(child->data(DocumentPointerRole).value<void *>());

            Q_ASSERT(document != NULL);

            if (document->isModified()) {
                hasModifiedChild = true;
                break;
            }
        }

        markItemAsModified(item, hasModifiedChild);
    }
}

// private slot
void FilesWidget::updateLocationOfSender()
{
    Document *document = qobject_cast<Document *>(sender());

    Q_ASSERT(document != NULL);

    if (document == DocumentManager::current()) {
        DocumentManager::setCurrent(NULL);
    }

    removeDocument(document);
    addDocument(document);

    if (DocumentManager::current() == NULL) {
        DocumentManager::setCurrent(document);
    }
}

// private slot
void FilesWidget::updateModificationMarkerOfSender()
{
    updateModificationMarker(qobject_cast<Document *>(sender()));
}

// private
void FilesWidget::updateModificationMarker(Document *document)
{
    Q_ASSERT(document != NULL);

    QStandardItem *item = m_children.value(document, NULL);

    Q_ASSERT(item != NULL);

    markItemAsModified(item, document->isModified());
    updateParentItemMarkers(item->parent());
}

// private
void FilesWidget::markItemAsCurrent(QStandardItem *item, bool mark) const
{
    QFont font(item->font());

    font.setUnderline(mark);

    item->setFont(font);
}

// private
void FilesWidget::markItemAsModified(QStandardItem *item, bool mark) const
{
    Q_ASSERT(item != NULL);

    QString text;

    if (item->parent() == NULL) {
        text = item->data(DirectoryPathRole).value<QString>();
    } else {
        text = item->data(FileNameRole).value<QString>();
    }

    if (mark) {
        item->setText(text + "*");
        item->setForeground(Qt::red);
    } else {
        item->setText(text);
        item->setForeground(palette().color(QPalette::Text));
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

    Document *document = static_cast<Document *>(index.data(DocumentPointerRole).value<void *>());

    Q_ASSERT(document != NULL);

    if (m_showModifiedFilesOnly && !document->isModified()) {
        return false;
    }

    if (m_filterEnabled && m_filterRegularExpression.isValid() && !m_filterRegularExpression.pattern().isEmpty()) {
        const QString &fileName = index.data(FileNameRole).value<QString>();

        return m_filterRegularExpression.match(fileName).hasMatch();
    }

    return true;
}
