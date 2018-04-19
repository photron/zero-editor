//
// Zero Editor
// Copyright (C) 2018 Matthias Bolte <matthias.bolte@googlemail.com>
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

#include "filedialog.h"
#include "ui_filedialog.h"

#include "eventfilter.h"
#include "settings.h"
#include "utils.h"

#include <QDebug>
#include <QFileIconProvider>
#include <QFileSystemModel>
#include <QMenu>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QStorageInfo>
#include <QToolButton>

FileDialog::~FileDialog()
{
    delete m_ui;
    delete m_fileIconProvider;
}

// static
QList<Location> FileDialog::getOpenLocations(QWidget *parent, const Location &suggestion)
{
    FileDialog dialog(parent, "Open Files", "Open", QTreeView::ExtendedSelection, false, suggestion);

    if (dialog.exec() != QDialog::Accepted) {
        return QList<Location>();
    }

    return dialog.m_pickedLocations;
}

// static
Location FileDialog::getSaveLocation(QWidget *parent, const Location &suggestion)
{
    FileDialog dialog(parent, "Save File", "Save", QTreeView::SingleSelection, true, suggestion);

    if (dialog.exec() != QDialog::Accepted) {
        return Location();
    }

    return dialog.m_pickedLocations.first();
}

// private slot
void FileDialog::updateNavigationFromButton()
{
    QToolButton *button = qobject_cast<QToolButton *>(sender());

    Q_ASSERT(button != NULL);

    const Location &location = button->property("_zero_location").toString();

    // FIXME: The navigation is not updated when the model updates itself in the background if directories are renamed
    //        or (re)moved. Therefore, the location mapped to this navigation button might not exist anymore.
    if (!location.exists()) {
        reportError(QString("\"%1\" does not exist anymore.").arg(location.path()));

        return;
    }

    setLocation(location);

    Utils::setFontUnderline(m_buttonActiveNavigation, false);
    Utils::setFontUnderline(button, true);

    m_buttonActiveNavigation = button;

    m_ui->treeFiles->selectionModel()->clear();
    m_ui->editLocation->setText("");
}

// private slot
void FileDialog::updateNavigationFromBookmark(const QModelIndex &index)
{
    Q_ASSERT(index.isValid());

    QStandardItem *item = m_bookmarksModel->itemFromIndex(index);

    Q_ASSERT(item != NULL);

    if (!item->isEnabled()) {
        return;
    }

    const Location &location = item->data(BookmarkLocationRole).toString();

    if (!location.exists()) {
        reportError(QString("\"%1\" does not exist anymore.").arg(location.path()));

        return;
    }

    setNavigation(setLocation(location));

    m_ui->treeFiles->selectionModel()->clear();
    m_ui->editLocation->setText("");
}

// private slot
void FileDialog::updateLocationEdit()
{
    const QItemSelection &selection = m_ui->treeFiles->selectionModel()->selection();
    const QModelIndexList &indexes = Utils::convertItemSelectionToIndexList(selection, 0);

    if (indexes.size() == 1) {
        m_ui->editLocation->setText(m_filesModel->fileName(indexes.first()));
    } else {
        m_ui->editLocation->setText("");
    }
}

// private slot
void FileDialog::updateCreateDirectoryButton()
{
    m_ui->buttonCreateDirectory->setEnabled(!m_ui->editLocation->text().isEmpty());
}

// private slot
void FileDialog::showBookmarkMenu(const QPoint &position)
{
    const QModelIndex &index = m_ui->treeBookmarks->indexAt(position);

    if (!index.isValid()) {
        return;
    }

    QStandardItem *item = m_bookmarksModel->itemFromIndex(index);

    Q_ASSERT(item != NULL);

    bool editable = item->data(BookmarkEditableRole).toBool();

    QMenu menu;
    QAction *moveUp = menu.addAction(QIcon(":/icons/16x16/up.png"), "Move Up");
    QAction *moveDown = menu.addAction(QIcon(":/icons/16x16/down.png"), "Move Down");

    menu.addSeparator();

    QAction *rename = menu.addAction(QIcon(":/icons/16x16/rename.png"), "Rename");

    menu.addSeparator();

    QAction *remove = menu.addAction(QIcon(":/icons/16x16/remove.png"), "Remove");

    moveUp->setEnabled(index.row() > 0);
    moveDown->setEnabled(index.row() < m_bookmarksModel->rowCount() - 1);
    rename->setEnabled(editable);
    remove->setEnabled(editable);

    QAction *selected = menu.exec(m_ui->treeBookmarks->viewport()->mapToGlobal(position));

    if (selected == moveUp) {
        int row = item->row();

        Q_ASSERT(row > 0);

        m_bookmarksModel->takeRow(row);
        m_bookmarksModel->insertRow(row - 1, item);

        m_ui->treeBookmarks->selectionModel()->select(m_bookmarksModel->index(row - 1, 0),
                                                      QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    } else if (selected == moveDown) {
        int row = item->row();

        Q_ASSERT(row < m_bookmarksModel->rowCount() - 1);

        m_bookmarksModel->takeRow(row);
        m_bookmarksModel->insertRow(row + 1, item);

        m_ui->treeBookmarks->selectionModel()->select(m_bookmarksModel->index(row + 1, 0),
                                                      QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    } else if (selected == rename) {
        Q_ASSERT(editable);

        // FIXME: Use QItemEditorFactory to create an QLineEdit editor with the EventFilter installed
        m_ui->treeBookmarks->edit(index);
    } else if (selected == remove) {
        Q_ASSERT(editable);

        m_bookmarksModel->removeRow(item->row());
    }
}

// private slot
void FileDialog::showFileMenu(const QPoint &position)
{
    const QModelIndex &index = m_ui->treeFiles->indexAt(position);

    if (!index.isValid()) {
        return;
    }

    QMenu menu;
    QAction *addBookmark_ = menu.addAction(QIcon(":/icons/16x16/add.png"), "Add Bookmark");

    addBookmark_->setEnabled(m_filesModel->isDir(index));

    if (menu.exec(m_ui->treeFiles->viewport()->mapToGlobal(position)) != addBookmark_) {
        return;
    }

    const Location &location = m_filesModel->filePath(index) + "/"; // Append slash to indicate directory

    addBookmark(location.directoryName(), location, true, true);
}

// private slot
void FileDialog::createDirectory()
{
    Q_ASSERT(m_saveFileMode);

    const QString &text = m_ui->editLocation->text();

    Q_ASSERT(!text.isEmpty());

    // Leading/Trailing whitespace is a problem in directory names because at least the Windows Explorer doesn't
    // handle it well and produces strange errors if you try to rename or delete such a directory.
    QStringList parts;

    foreach (const QString &part, text.split(QRegularExpression("[/\\\\]"))) {
        parts << part.trimmed();
    }

    if (parts.join("/") != QString(text).replace("\\", "/")) {
        reportError(QString("Cannot create directory \"%1\" with leading/trailing whitespace.").arg(text));

        return;
    }

    // Create directory
    const QString &path = QDir::isAbsolutePath(text) ? text : rootDirectoryPath() + text;

    // FIXME: Check that path prefix does not already exist as file

    if (!QDir().mkpath(path)) {
        reportError(QString("Could not create directory \"%1\": Unknown error.").arg(text));

        return;
    }

    setNavigation(setLocation(path + "/")); // Append slash to indicate directory
}

// private slot
void FileDialog::pick()
{
    const QString &text = m_ui->editLocation->text();

    if (!text.isEmpty()) {
        const Location &location = QDir::isAbsolutePath(text) ? text : rootDirectoryPath() + text;
        const QModelIndex &directoryIndex = m_filesModel->index(location.directoryPath());

        // If the directory part of the picked location doesn't exist then neither can a file be opened from it nor can
        // a file be saved to it.
        if (!directoryIndex.isValid()) {
            reportError(QString("\"%1\" does not exist.").arg(location.directoryPath()));

            return;
        }

        // The directory part of the picked location might exist, but as a file
        if (!m_filesModel->isDir(directoryIndex)) {
            reportError(QString("\"%1\" does exist, but is not a directory.").arg(location.directoryPath()));

            return;
        }

        const QModelIndex &index = m_filesModel->index(location.path());

        if (!index.isValid()) {
            if (m_saveFileMode && location.isFile()) {
                m_pickedLocations << location;

                accept();
            } else {
                reportError(QString("\"%1\" does not exist.").arg(location.path()));
            }

            return;
        }

        if (m_filesModel->isDir(index)) {
            const QString &directoryPath = m_filesModel->filePath(index) + "/"; // Append slash to indicate directory

            setNavigation(setLocation(directoryPath));
        } else {
            const Location &location = m_filesModel->filePath(index);

            if (m_saveFileMode && !confirmOverwrite(location)) {
                return;
            }

            m_pickedLocations << location;

            accept();
        }
    } else {
        const QItemSelection &selection = m_ui->treeFiles->selectionModel()->selection();
        QList<Location> locations;

        foreach (const QModelIndex &index, Utils::convertItemSelectionToIndexList(selection, 0)) {
            if (m_filesModel->isDir(index)) {
                reportError("Cannot pick directory along other items.");

                return;
            }

            locations << m_filesModel->filePath(index);
        }

        if (!locations.isEmpty()) {
            if (m_saveFileMode && !confirmOverwrite(locations.first())) {
                return;
            }

            m_pickedLocations = locations;

            accept();
        }
    }
}

// private slot
void FileDialog::saveBookmarks()
{
    QSettings *settings = Settings::settings();
    int index = 0;

    settings->beginWriteArray("FileDialog/Bookmarks");

    for (int row = 0; row < m_bookmarksModel->rowCount(); ++row) {
        QStandardItem *item = m_bookmarksModel->item(row);

        if (!item->data(BookmarkEditableRole).toBool()) {
            continue;
        }

        const QString &name = item->text();
        const QString &location = item->data(BookmarkLocationRole).toString();

        settings->setArrayIndex(index);
        settings->setValue("Name", name);
        settings->setValue("Location", location);

        ++index;
    }

    settings->endArray();
}

// private
FileDialog::FileDialog(QWidget *parent, const QString &title, const QString &action,
                       QTreeView::SelectionMode selectionMode, bool saveFileMode, const Location &suggestion) :
    QDialog(parent),
    m_ui(new Ui::FileDialog),
    m_fileIconProvider(new QFileIconProvider()),
    m_bookmarksModel(new QStandardItemModel(this)),
    m_filesModel(new QFileSystemModel(this)),
    m_buttonActiveNavigation(NULL),
    m_saveFileMode(saveFileMode)
{
    m_ui->setupUi(this);

    setWindowTitle(title);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    m_ui->splitter->setSizes(QList<int>() << 100 << 300);

    // FIXME: Empty icon trick for alignment stolen from QFileSystemModel::headerData()
    QImage image(16, 1, QImage::Format_Mono);

    image.fill(0);
    image.setAlphaChannel(image.createAlphaMask());

    // FIXME: Text in the header has a 1px vertical offset (at least on Windows 10) compared to the header of the files
    //        QTreeView. There is no obvious reason for this.
    m_bookmarksModel->setHorizontalHeaderItem(0, new QStandardItem(QPixmap::fromImage(image), "Name"));

#ifdef Q_OS_WIN
    foreach (const QStorageInfo &info, QStorageInfo::mountedVolumes()) {
        QString name = info.name();
        const Location &location = info.rootPath() + "/"; // Append slash to indicate directory

        if (name.isEmpty()) {
            name = location.directoryName();
        } else {
            name += QString(" (%1)").arg(location.directoryName());
        }

        addBookmark(name, location, info.isValid() && info.isReady(), false);
    }
#endif

    QSettings *settings = Settings::settings();
    int size = settings->beginReadArray("FileDialog/Bookmarks");

    for (int i = 0; i < size; ++i) {
        settings->setArrayIndex(i);

        const QString &name = settings->value("Name", "<Invalid>").toString();
        const QString &location = settings->value("Location").toString();

        addBookmark(name, location, true, true);
    }

    settings->endArray();

    m_ui->treeBookmarks->setModel(m_bookmarksModel);

    m_ui->treeFiles->setModel(m_filesModel);
    m_ui->treeFiles->setSelectionMode(selectionMode);
    m_ui->treeFiles->setColumnHidden(2, true); // Hide type column
    m_ui->treeFiles->setColumnWidth(0, 400);
    m_ui->treeFiles->setFocus();

    m_ui->buttonCreateDirectory->setVisible(m_saveFileMode);

    m_ui->buttonPick->setText(action);

    m_ui->editLocation->installEventFilter(EventFilter::instance());

    connect(m_ui->treeBookmarks, &QTreeView::activated, this, &FileDialog::updateNavigationFromBookmark);
    connect(m_ui->treeBookmarks, &QTreeView::customContextMenuRequested, this, &FileDialog::showBookmarkMenu);
    connect(m_ui->treeFiles->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FileDialog::updateLocationEdit);
    connect(m_ui->treeFiles, &QTreeView::activated, this, &FileDialog::pick);
    connect(m_ui->treeFiles, &QTreeView::customContextMenuRequested, this, &FileDialog::showFileMenu);
    connect(m_ui->editLocation, &QLineEdit::textChanged, this, &FileDialog::updateCreateDirectoryButton);
    connect(m_ui->buttonCreateDirectory, &QPushButton::clicked, this, &FileDialog::createDirectory);
    connect(m_ui->buttonPick, &QPushButton::clicked, this, &FileDialog::pick);
    connect(m_ui->buttonCancel, &QPushButton::clicked, this, &FileDialog::reject);
    connect(this, &QDialog::finished, this, &FileDialog::saveBookmarks);

    if (suggestion.exists()) {
        setNavigation(setLocation(suggestion));
    } else {
        setNavigation(setLocation(Location::home()));
    }

    updateCreateDirectoryButton();
}

// private
QString FileDialog::rootDirectoryPath() const
{
    const QModelIndex &index = m_ui->treeFiles->rootIndex();

    Q_ASSERT(index.isValid());

    const QString &directoryPath = m_filesModel->filePath(index) + "/"; // Append slash to indicate directory

    Q_ASSERT(Location(directoryPath).exists());

    return directoryPath;
}

// private
Location FileDialog::setLocation(const Location &location)
{
    const QModelIndex &root = m_filesModel->setRootPath(location.directoryPath());

    m_ui->treeFiles->setRootIndex(root);
    m_ui->treeFiles->selectionModel()->clear();
    m_ui->editLocation->setText("");

    if (location.isFile()) {
        const QModelIndex &index = m_filesModel->index(location.path());

        if (index.isValid()) {
            m_ui->treeFiles->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
    }

    return m_filesModel->filePath(root) + "/"; // Append slash to indicate directory
}

// private
void FileDialog::setNavigation(const Location &location)
{
    // Check if location is already part of the navigation
    for (int i = 0; i < m_ui->layoutNavigation->count(); ++i) {
        QLayoutItem *item = m_ui->layoutNavigation->itemAt(i);
        QToolButton *button = qobject_cast<QToolButton *>(item->widget());

        if (button == NULL) {
            continue;
        }

        if (location == button->property("_zero_location").toString()) {
            button->click();

            return;
        }
    }

    // If not found recreate navigation
    while (m_ui->layoutNavigation->count() > 0) {
        QLayoutItem *item = m_ui->layoutNavigation->takeAt(0);

        delete item->widget();
        delete item;
    }

    QString directoryPath;
    QToolButton *button = NULL;

    foreach (const QString &part, location.directoryPath().split(QDir::separator(), QString::SkipEmptyParts)) {
        directoryPath += part + QDir::separator();

        button = new QToolButton(this);
        button->setText(part + QDir::separator());
        button->setProperty("_zero_location", QVariant::fromValue(directoryPath));

        connect(button, &QToolButton::clicked, this, &FileDialog::updateNavigationFromButton);

        m_ui->layoutNavigation->addWidget(button);
    }

    m_ui->layoutNavigation->addStretch();

    if (button != NULL) {
        Utils::setFontUnderline(button, true);

        m_buttonActiveNavigation = button;
    }
}

// private
bool FileDialog::confirmOverwrite(const Location &location)
{
    Q_ASSERT(m_saveFileMode);

    const QString &message = QString("Overwrite already existing file \"%1\"?").arg(location.path());

    return QMessageBox::question(this, "Overwrite File", message, "Overwrite", "Cancel") == 0;
}

// private
void FileDialog::reportError(const QString &message)
{
    QMessageBox::critical(this, windowTitle() + " Error", message);
}

// private
void FileDialog::addBookmark(const QString &name, const Location &location, bool enable, bool editable)
{
    QStandardItem *item = new QStandardItem(m_fileIconProvider->icon(QFileInfo(location.path())), name);

    item->setData(location.path(), BookmarkLocationRole);
    item->setData(QVariant::fromValue(editable), BookmarkEditableRole);
    item->setEnabled(enable);
    item->setToolTip(location.path());

    m_bookmarksModel->appendRow(item);
}
