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

#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include "location.h"

#include <QDialog>
#include <QTreeView>

class QFileIconProvider;
class QFileSystemModel;
class QStandardItemModel;
class QToolButton;

namespace Ui {
class FileDialog;
}

class FileDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(FileDialog)

public:
    ~FileDialog();

    static QList<Location> getOpenLocations(QWidget *parent, const Location &location);
    static Location getSaveLocation(QWidget *parent, const Location &location);

private slots:
    void updateNavigationFromButton();
    void updateNavigationFromBookmark(const QModelIndex &index);
    void updateLocationEdit();
    void updateCreateDirectoryButton();
    void showBookmarkMenu(const QPoint &position);
    void showFileMenu(const QPoint &position);
    void createDirectory();
    void pick();
    void saveBookmarks();

private:
    enum {
        BookmarkLocationRole = Qt::UserRole,
        BookmarkEditableRole
    };

    FileDialog(QWidget *parent, const QString &title, const QString &action, QTreeView::SelectionMode selectionMode,
               bool saveFileMode, const Location &suggestion);

    QString rootDirectoryPath() const;
    Location setLocation(const Location &location);
    void setNavigation(const Location &location);
    bool confirmOverwrite(const Location &location);
    void reportError(const QString &message);
    void addBookmark(const QString &name, const Location &location, bool enable, bool editable);

    Ui::FileDialog *m_ui;

    QFileIconProvider *m_fileIconProvider;
    QStandardItemModel *m_bookmarksModel;
    QFileSystemModel *m_filesModel;
    QToolButton *m_buttonActiveNavigation;
    bool m_saveFileMode;
    QList<Location> m_pickedLocations;
};

#endif // FILEDIALOG_H
