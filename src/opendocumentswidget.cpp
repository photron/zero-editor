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
    m_lastCurrentItem(NULL),
    m_showModifiedDocumentsOnly(false),
    m_filterRegExp("", Qt::CaseSensitive, QRegExp::WildcardUnix),
    m_filterEnabled(false)
{
    m_ui->setupUi(this);

    // FIXME: enable edit triggers to allow renaming files on disk from the open documents tree?

    m_ui->editFilter->setVisible(m_ui->checkFilter->isChecked());
    m_ui->treeDocuments->setModel(&m_model);

    connect(m_ui->radioModified, &QRadioButton::toggled, this, &OpenDocumentsWidget::showModifiedDocumentsOnly);
    connect(m_ui->checkFilter, &QCheckBox::toggled, this, &OpenDocumentsWidget::setFilterEnabled);
    connect(m_ui->checkFilter, &QCheckBox::toggled, m_ui->editFilter, &QLineEdit::setVisible);
    connect(m_ui->editFilter, &QLineEdit::textChanged, this, &OpenDocumentsWidget::setFilterPattern);
    connect(m_ui->treeDocuments, &QTreeView::activated, this, &OpenDocumentsWidget::setCurrentDocument);
    connect(DocumentManager::instance(), &DocumentManager::documentOpened, this, &OpenDocumentsWidget::addDocument);
    connect(DocumentManager::instance(), &DocumentManager::currentDocumentChanged, this, &OpenDocumentsWidget::setCurrentItem);
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

    QFileInfo fileInfo(document->filePath());
    QString absolutePath(QDir::toNativeSeparators(fileInfo.absolutePath()));
    QString fileName(fileInfo.fileName());
    QList<QStandardItem *> parents(m_model.findItems(absolutePath));
    QStandardItem *parent;

    if (parents.isEmpty()) {
        parent = new QStandardItem(absolutePath);

        parent->setIcon(QIcon(":/icons/16x16/folder.png"));
        parent->setToolTip(absolutePath);

        m_model.appendRow(parent);
        m_model.sort(0);
    } else {
        parent = parents.first();
    }

    QStandardItem *child = new QStandardItem(fileName);

    child->setIcon(QIcon(":/icons/16x16/file.png"));
    child->setToolTip(QDir::toNativeSeparators(fileInfo.absoluteFilePath()));
    child->setData(qVariantFromValue((void *)document), DocumentPointerRole);
    child->setData(fileName, FileNameRole);

    m_items.insert(document, child);

    parent->appendRow(child);
    parent->sortChildren(0);

    connect(document, &Document::modificationChanged, this, &OpenDocumentsWidget::updateItemModification);
}

// private slot
void OpenDocumentsWidget::setCurrentDocument(const QModelIndex &index)
{
    Q_ASSERT(index.isValid());

    Document *document = static_cast<Document *>(index.data(DocumentPointerRole).value<void *>());

    if (document != NULL) {
        DocumentManager::setCurrentDocument(document);
    }
}

// private slot
void OpenDocumentsWidget::setCurrentItem(Document *document)
{
    Q_ASSERT(document != NULL);

    if (m_lastCurrentItem != NULL) {
        QFont font(m_lastCurrentItem->font());

        font.setBold(false);

        m_lastCurrentItem->setFont(font);
    }

    QStandardItem *item = m_items.value(document, NULL);

    if (item != NULL) {
        QFont font(item->font());

        font.setBold(true);

        item->setFont(font);

        m_ui->treeDocuments->expand(item->index());
        m_ui->treeDocuments->scrollTo(item->index());
    }

    m_lastCurrentItem = item;
}

// private slot
void OpenDocumentsWidget::updateItemModification(bool modified)
{
    Document *document = qobject_cast<Document *>(sender());

    Q_ASSERT(document != NULL);

    QStandardItem *item = m_items.value(document, NULL);

    if (item != NULL) {
        QString fileName = item->data(FileNameRole).value<QString>();

        item->setText(fileName + (modified ? "*" : ""));
        item->setForeground(modified ? Qt::red : palette().color(QPalette::Text));
    }
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

        applyFilter();
    }
}

// private slot
void OpenDocumentsWidget::setFilterPattern(const QString &pattern)
{
    if (m_filterRegExp.pattern() != pattern) {
        bool wasValid = m_filterRegExp.isValid();

        m_filterRegExp = QRegExp(pattern, Qt::CaseSensitive, QRegExp::WildcardUnix);

        if (m_filterRegExp.isValid() != wasValid) {
            m_ui->editFilter->setStyleSheet(m_filterRegExp.isValid() ? "" : "QLineEdit { color: red }");
        }

        if (m_filterEnabled) {
            applyFilter();
        }
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

            m_ui->treeDocuments->setRowHidden(childRow, parent->index(), hideChild);
        }

        m_ui->treeDocuments->setRowHidden(parentRow, root->index(), hideParent);
    }
}

// private
bool OpenDocumentsWidget::filterAcceptsChild(const QModelIndex &index) const
{
    if (!m_showModifiedDocumentsOnly && !m_filterEnabled) {
        return true;
    }

    Document *document = static_cast<Document *>(index.data(DocumentPointerRole).value<void *>());

    Q_ASSERT(document != NULL);

    if (m_showModifiedDocumentsOnly && !document->isModified()) {
        return false;
    }

    if (m_filterEnabled && m_filterRegExp.isValid() && !m_filterRegExp.isEmpty()) {
        QString fileName(index.data(FileNameRole).value<QString>());

        if (!m_filterRegExp.exactMatch(fileName)) {
            return false;
        }
    }

    return true;
}
