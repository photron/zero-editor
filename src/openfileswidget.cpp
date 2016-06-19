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

#include "openfileswidget.h"
#include "ui_openfileswidget.h"

#include "document.h"
#include "documentmanager.h"
#include "editor.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>

OpenFilesWidget::OpenFilesWidget(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::OpenFilesWidget),
    m_currentChild(NULL),
    m_showModifiedFilesOnly(false),
    m_filterEnabled(false)
{
    m_ui->setupUi(this);

    // FIXME: enable edit triggers to allow renaming files on disk from the open documents tree?

    m_model.setSortRole(LowerCaseNameRole);

    m_ui->editFilter->setVisible(m_ui->checkFilter->isChecked());
    m_ui->treeFiles->setModel(&m_model);

    connect(m_ui->radioModified, &QRadioButton::toggled, this, &OpenFilesWidget::showModifiedDocumentsOnly);
    connect(m_ui->checkFilter, &QCheckBox::toggled, this, &OpenFilesWidget::setFilterEnabled);
    connect(m_ui->editFilter, &QLineEdit::textChanged, this, &OpenFilesWidget::setFilterPattern);
    connect(m_ui->treeFiles, &QTreeView::activated, this, &OpenFilesWidget::setCurrentDocument);
    connect(m_ui->treeFiles, &QTreeView::expanded, this, &OpenFilesWidget::updateParentIndexMarkers);
    connect(m_ui->treeFiles, &QTreeView::collapsed, this, &OpenFilesWidget::updateParentIndexMarkers);

    connect(DocumentManager::instance(), &DocumentManager::opened, this, &OpenFilesWidget::addDocument);
    connect(DocumentManager::instance(), &DocumentManager::aboutToBeClosed, this, &OpenFilesWidget::removeDocument);
    connect(DocumentManager::instance(), &DocumentManager::currentChanged, this, &OpenFilesWidget::setCurrentChild);
    connect(DocumentManager::instance(), &DocumentManager::modificationCountChanged, this, &OpenFilesWidget::updateModifiedButton);
}

OpenFilesWidget::~OpenFilesWidget()
{
    delete m_ui;
}

void OpenFilesWidget::installLineEditEventFilter(QObject *filter)
{
    m_ui->editFilter->installEventFilter(filter);
}

// private slot
void OpenFilesWidget::addDocument(Document *document)
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

    connect(document, &Document::locationChanged, this, &OpenFilesWidget::updateLocationOfSender);
    connect(document, &Document::modificationChanged, this, &OpenFilesWidget::updateModificationMarkerOfSender);

    // Apply current filter
    if (!filterAcceptsChild(child->index())) {
        m_ui->treeFiles->setRowHidden(child->row(), parent->index(), true);

        if (parent->rowCount() == 1) {
            m_ui->treeFiles->setRowHidden(parent->row(), m_model.invisibleRootItem()->index(), true);
        }
    }

    // Show modification marker, if necessary
    updateModificationMarker(document);
    updateParentItemMarkers(parent);
}

// private slot
void OpenFilesWidget::removeDocument(Document *document)
{
    Q_ASSERT(document != NULL);

    disconnect(document, &Document::modificationChanged, this, &OpenFilesWidget::updateModificationMarkerOfSender);
    disconnect(document, &Document::locationChanged, this, &OpenFilesWidget::updateLocationOfSender);

    QStandardItem *child = m_children.value(document, NULL);

    Q_ASSERT(child != NULL);
    Q_ASSERT(child != m_currentChild);

    QStandardItem *parent = child->parent();

    Q_ASSERT(parent != NULL);

    parent->removeRow(child->row());

    if (parent->rowCount() > 0) {
        updateParentItemMarkers(parent);
    } else {
        m_model.removeRow(parent->row());
    }

    m_children.remove(document);

    if (m_currentChild != NULL) {
        m_ui->treeFiles->selectionModel()->select(m_currentChild->index(), QItemSelectionModel::ClearAndSelect);
    }
}

// private slot
void OpenFilesWidget::setCurrentDocument(const QModelIndex &index)
{
    Q_ASSERT(index.isValid());

    Document *document = static_cast<Document *>(index.data(DocumentPointerRole).value<void *>());

    if (document != NULL) {
        DocumentManager::setCurrent(document);
    }
}

// private slot
void OpenFilesWidget::setCurrentChild(Document *document)
{
    // Set current child and parent back to normal
    if (m_currentChild != NULL) {
        markItemAsCurrent(m_currentChild, false);

        QStandardItem *parent = m_currentChild->parent();

        m_currentChild = NULL;

        updateParentItemMarkers(parent);
    }

    // Mark new current child as current
    if (document != NULL) {
        QStandardItem *child = m_children.value(document, NULL);

        Q_ASSERT(child != NULL);

        QStandardItem *parent = child->parent();

        Q_ASSERT(parent != NULL);

        m_ui->treeFiles->expand(parent->index());
        m_ui->treeFiles->scrollTo(child->index());
        m_ui->treeFiles->selectionModel()->select(child->index(), QItemSelectionModel::ClearAndSelect);

        m_currentChild = child;

        markItemAsCurrent(m_currentChild, true);
        updateParentItemMarkers(parent);
    }
}

// private slot
void OpenFilesWidget::updateModifiedButton(int modificationCount)
{
    m_ui->radioModified->setText(modificationCount ? QString("Modified (%1)").arg(modificationCount) : "Modified");
}

// private slot
void OpenFilesWidget::updateParentIndexMarkers(const QModelIndex &index)
{
    Q_ASSERT(index.isValid());

    updateParentItemMarkers(m_model.itemFromIndex(index));
}

// private slot
void OpenFilesWidget::updateParentItemMarkers(QStandardItem *item)
{
    Q_ASSERT(item != NULL);
    Q_ASSERT(item->parent() == NULL);

    if (m_ui->treeFiles->isExpanded(item->index())) {
        // Check if this is the parent of the hidden current child
        bool hasHiddenCurrentChild = false;

        if (m_currentChild != NULL && m_currentChild->parent() == item) {
            hasHiddenCurrentChild = m_ui->treeFiles->isRowHidden(m_currentChild->row(), item->index());
        }

        markItemAsCurrent(item, hasHiddenCurrentChild);

        // Check if this is a parent of hidden modified children
        int childRowCount = item->rowCount();
        bool hasHiddenModifiedChild = false;

        for (int childRow = 0; childRow < childRowCount; ++childRow) {
            QStandardItem *child = item->child(childRow);
            Document *document = static_cast<Document *>(child->data(DocumentPointerRole).value<void *>());

            Q_ASSERT(document != NULL);

            if (m_ui->treeFiles->isRowHidden(childRow, item->index()) && document->isModified()) {
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
void OpenFilesWidget::updateLocationOfSender()
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
void OpenFilesWidget::updateModificationMarkerOfSender()
{
    updateModificationMarker(qobject_cast<Document *>(sender()));
}

// private slot
void OpenFilesWidget::showModifiedDocumentsOnly(bool enable)
{
    if (m_showModifiedFilesOnly != enable) {
        m_showModifiedFilesOnly = enable;

        applyFilter();
    }
}

// private slot
void OpenFilesWidget::setFilterEnabled(bool enable)
{
    if (m_filterEnabled != enable) {
        m_filterEnabled = enable;

        m_ui->editFilter->setVisible(m_filterEnabled);

        if (m_filterEnabled) {
            m_ui->editFilter->setFocus();
        }

        applyFilter();
    }
}

// private slot
void OpenFilesWidget::setFilterPattern(const QString &pattern)
{
    if (m_filterRegularExpression.pattern() != pattern) {
        bool wasValid = m_filterRegularExpression.isValid();

        m_filterRegularExpression = QRegularExpression(pattern);

        if (m_filterRegularExpression.isValid() != wasValid) {
            m_ui->editFilter->setStyleSheet(m_filterRegularExpression.isValid() ? "" : "QLineEdit { color: red }");
        }

        applyFilter();
    }
}

// private
void OpenFilesWidget::updateModificationMarker(Document *document)
{
    Q_ASSERT(document != NULL);

    QStandardItem *item = m_children.value(document, NULL);

    Q_ASSERT(item != NULL);

    markItemAsModified(item, document->isModified());
    updateParentItemMarkers(item->parent());
}

// private
void OpenFilesWidget::markItemAsCurrent(QStandardItem *item, bool mark) const
{
    QFont font(item->font());

    font.setUnderline(mark);

    item->setFont(font);
}

// private
void OpenFilesWidget::markItemAsModified(QStandardItem *item, bool mark) const
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
void OpenFilesWidget::applyFilter()
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

            m_ui->treeFiles->setRowHidden(childRow, parent->index(), hideChild);
        }

        updateParentItemMarkers(parent);

        m_ui->treeFiles->setRowHidden(parentRow, root->index(), hideParent);
    }
}

// private
bool OpenFilesWidget::filterAcceptsChild(const QModelIndex &index) const
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
