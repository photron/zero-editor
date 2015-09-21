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

#include "opendocumentswidget.h"
#include "ui_opendocumentswidget.h"

#include "document.h"
#include "documentmanager.h"
#include "editor.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>

OpenDocumentsWidget::OpenDocumentsWidget(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::OpenDocumentsWidget),
    m_currentChild(NULL),
    m_showModifiedDocumentsOnly(false),
    m_filterEnabled(false)
{
    m_ui->setupUi(this);

    // FIXME: enable edit triggers to allow renaming files on disk from the open documents tree?

    m_model.setSortRole(LowerCaseNameRole);

    m_ui->editFilter->setVisible(m_ui->checkFilter->isChecked());
    m_ui->treeDocuments->setModel(&m_model);

    connect(m_ui->radioModified, &QRadioButton::toggled, this, &OpenDocumentsWidget::showModifiedDocumentsOnly);
    connect(m_ui->checkFilter, &QCheckBox::toggled, this, &OpenDocumentsWidget::setFilterEnabled);
    connect(m_ui->editFilter, &QLineEdit::textChanged, this, &OpenDocumentsWidget::setFilterPattern);
    connect(m_ui->treeDocuments, &QTreeView::activated, this, &OpenDocumentsWidget::setCurrentDocument);
    connect(m_ui->treeDocuments, &QTreeView::expanded, this, &OpenDocumentsWidget::updateParentMarkers);
    connect(m_ui->treeDocuments, &QTreeView::collapsed, this, &OpenDocumentsWidget::updateParentMarkers);

    connect(DocumentManager::instance(), &DocumentManager::opened, this, &OpenDocumentsWidget::addDocument);
    connect(DocumentManager::instance(), &DocumentManager::aboutToBeClosed, this, &OpenDocumentsWidget::removeDocument);
    connect(DocumentManager::instance(), &DocumentManager::currentChanged, this, &OpenDocumentsWidget::setCurrentChild);
    connect(DocumentManager::instance(), &DocumentManager::modificationCountChanged, this, &OpenDocumentsWidget::updateModifiedButton);
}

OpenDocumentsWidget::~OpenDocumentsWidget()
{
    delete m_ui;
}

void OpenDocumentsWidget::installLineEditEventFilter(QObject *filter)
{
    m_ui->editFilter->installEventFilter(filter);
}

// private slot
void OpenDocumentsWidget::addDocument(Document *document)
{
    Q_ASSERT(document != NULL);

    QString absoluteFilePath;
    QString absolutePath;
    QString fileName;

    if (document->filePath().isEmpty()) {
        absoluteFilePath = "unnamed";
        absolutePath = "unnamed";
        fileName = "unnamed";
    } else {
        QFileInfo fileInfo(document->filePath());

        absoluteFilePath = QDir::toNativeSeparators(fileInfo.absoluteFilePath());
        absolutePath = QDir::toNativeSeparators(fileInfo.absolutePath());
        fileName = fileInfo.fileName();
    }

    // Find or create parent item
    QModelIndexList parents = m_model.match(m_model.index(0, 0, QModelIndex()), AbsolutePathRole,
                                            absolutePath, 1, Qt::MatchExactly);
    QStandardItem *parent;

    if (parents.isEmpty()) {
        parent = new QStandardItem(QIcon(":/icons/16x16/folder.png"), absolutePath);

        parent->setToolTip(absolutePath);
        parent->setData(absolutePath, AbsolutePathRole);
        parent->setData(document->filePath().isEmpty() ? "" : absolutePath.toLower(), LowerCaseNameRole);

        m_model.appendRow(parent);
        m_model.sort(0);
    } else {
        parent = m_model.itemFromIndex(parents.first());
    }

    // Create child item
    QStandardItem *child = new QStandardItem(QIcon(":/icons/16x16/file.png"), fileName);

    child->setToolTip(absoluteFilePath);
    child->setData(qVariantFromValue((void *)document), DocumentPointerRole);
    child->setData(fileName, FileNameRole);
    child->setData(document->filePath().isEmpty() ? "" : fileName.toLower(), LowerCaseNameRole);

    m_children.insert(document, child);

    parent->appendRow(child);
    parent->sortChildren(0);

    connect(document, &Document::modificationChanged, this, &OpenDocumentsWidget::updateModificationMarkerOfSender);

    // Apply current filter
    if (!filterAcceptsChild(child->index())) {
        m_ui->treeDocuments->setRowHidden(child->row(), parent->index(), true);

        if (parent->rowCount() == 1) {
            m_ui->treeDocuments->setRowHidden(parent->row(), m_model.invisibleRootItem()->index(), true);
        }
    }

    // Show modification marker, if necessary
    updateModificationMarker(document);
}

// private slot
void OpenDocumentsWidget::removeDocument(Document *document)
{
    Q_ASSERT(document != NULL);

    disconnect(document, &Document::modificationChanged, this, &OpenDocumentsWidget::updateModificationMarkerOfSender);

    QStandardItem *child = m_children.value(document, NULL);

    Q_ASSERT(child != NULL);
    Q_ASSERT(child != m_currentChild);

    QStandardItem *parent = child->parent();

    parent->removeRow(child->row());

    if (parent->rowCount() > 0) {
        updateParentMarkers(parent->index());
    } else {
        m_model.removeRow(parent->row());
    }

    m_children.remove(document);

    if (m_currentChild != NULL) {
        m_ui->treeDocuments->selectionModel()->select(m_currentChild->index(), QItemSelectionModel::ClearAndSelect);
    }
}

// private slot
void OpenDocumentsWidget::setCurrentDocument(const QModelIndex &index)
{
    Q_ASSERT(index.isValid());

    Document *document = static_cast<Document *>(index.data(DocumentPointerRole).value<void *>());

    if (document != NULL) {
        DocumentManager::setCurrent(document);
    }
}

// private slot
void OpenDocumentsWidget::setCurrentChild(Document *document)
{
    // Set current child and parent back to normal
    if (m_currentChild != NULL) {
        markItemAsCurrent(m_currentChild->parent(), false);
        markItemAsCurrent(m_currentChild, false);

        m_currentChild = NULL;
    }

    // Mark new current child as current
    if (document != NULL) {
        QStandardItem *child = m_children.value(document, NULL);

        Q_ASSERT(child != NULL);

        markItemAsCurrent(child, true);

        m_ui->treeDocuments->expand(child->parent()->index());
        m_ui->treeDocuments->scrollTo(child->index());
        m_ui->treeDocuments->selectionModel()->select(child->index(), QItemSelectionModel::ClearAndSelect);

        m_currentChild = child;
    }
}

// private slot
void OpenDocumentsWidget::updateModifiedButton(int modificationCount)
{
    m_ui->radioModified->setText(modificationCount ? QString("Modified (%1)").arg(modificationCount) : "Modified");
}

// private slot
void OpenDocumentsWidget::updateParentMarkers(const QModelIndex &index)
{
    Q_ASSERT(index.isValid());

    QStandardItem *parent = m_model.itemFromIndex(index);

    if (parent->parent() != NULL) {
        return; // Not a top-level item
    }

    if (m_ui->treeDocuments->isExpanded(index)) {
        markItemAsCurrent(parent, false);
        markItemAsModified(parent, false);
    } else {
        // Check if this is the parent of the current child
        markItemAsCurrent(parent, m_currentChild != NULL && m_currentChild->parent() == parent);

        // Check if this is a parent of modified children
        int childRowCount = parent->rowCount();
        bool hasModifiedChild = false;

        for (int childRow = 0; childRow < childRowCount; ++childRow) {
            QStandardItem *child = parent->child(childRow);
            Document *document = static_cast<Document *>(child->data(DocumentPointerRole).value<void *>());

            if (document->isModified()) {
                hasModifiedChild = true;
                break;
            }
        }

        markItemAsModified(parent, hasModifiedChild);
    }
}

// private slot
void OpenDocumentsWidget::updateModificationMarkerOfSender()
{
    updateModificationMarker(qobject_cast<Document *>(sender()));
}

// private slot
void OpenDocumentsWidget::showModifiedDocumentsOnly(bool enable)
{
    if (m_showModifiedDocumentsOnly != enable) {
        m_showModifiedDocumentsOnly = enable;

        applyFilter();
    }
}

// private slot
void OpenDocumentsWidget::setFilterEnabled(bool enable)
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
void OpenDocumentsWidget::setFilterPattern(const QString &pattern)
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
void OpenDocumentsWidget::updateModificationMarker(Document *document)
{
    Q_ASSERT(document != NULL);

    QStandardItem *child = m_children.value(document, NULL);

    Q_ASSERT(child != NULL);

    markItemAsModified(child, document->isModified());

    QStandardItem *parent = child->parent();

    if (!m_ui->treeDocuments->isExpanded(parent->index())) {
        if (document->isModified()) {
            markItemAsModified(parent, true);
        } else {
            int childRowCount = parent->rowCount();

            for (int childRow = 0; childRow < childRowCount; ++childRow) {
                QStandardItem *otherChild = parent->child(childRow);
                Document *otherDocument = static_cast<Document *>(otherChild->data(DocumentPointerRole).value<void *>());

                if (otherDocument->isModified()) {
                    return;
                }
            }

            markItemAsModified(parent, false);
        }
    }
}

// private
void OpenDocumentsWidget::markItemAsCurrent(QStandardItem *item, bool mark) const
{
    QFont font(item->font());

    font.setUnderline(mark);

    item->setFont(font);
}

// private
void OpenDocumentsWidget::markItemAsModified(QStandardItem *item, bool mark) const
{
    QString text;

    if (item->parent() == NULL) {
        text = item->data(AbsolutePathRole).value<QString>();
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
void OpenDocumentsWidget::applyFilter()
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

            if (child == m_currentChild && m_ui->treeDocuments->isExpanded(parent->index())) {
                markItemAsCurrent(parent, hideChild);
            }

            m_ui->treeDocuments->setRowHidden(childRow, parent->index(), hideChild);
        }

        m_ui->treeDocuments->setRowHidden(parentRow, root->index(), hideParent);
    }
}

// private
bool OpenDocumentsWidget::filterAcceptsChild(const QModelIndex &index) const
{
    Q_ASSERT(index.isValid());

    if (!m_showModifiedDocumentsOnly && !m_filterEnabled) {
        return true;
    }

    Document *document = static_cast<Document *>(index.data(DocumentPointerRole).value<void *>());

    Q_ASSERT(document != NULL);

    if (m_showModifiedDocumentsOnly && !document->isModified()) {
        return false;
    }

    if (m_filterEnabled && m_filterRegularExpression.isValid() && !m_filterRegularExpression.pattern().isEmpty()) {
        QString fileName(index.data(FileNameRole).value<QString>());

        return m_filterRegularExpression.match(fileName).hasMatch();
    }

    return true;
}
