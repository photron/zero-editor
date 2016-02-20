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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "editor.h"

#include <QMainWindow>

class Document;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_DISABLE_COPY(MainWindow)

public:
    explicit MainWindow(QWidget *parent = NULL);
    ~MainWindow();

    static MainWindow *instance() { return s_instance; }

protected:
    bool eventFilter(QObject *object, QEvent *event);

private slots:
    void newDocument();
    void openFile();
    void closeDocument();

    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void delete_();
    void selectAll();
    void toggleCase();

    void showFindAndReplaceWidget();
    void showFindInFilesWidget();

    void showEncodingDialog();
    void setWordWrapping(bool enable);

    void openTerminal();
    void showUnsavedDiffWidget();
    void showGitDiffWidget();

    void addEditor(Document *document);
    void removeEditor(Document *document);
    void setCurrentDocument(Document *document);

    void updateSaveAllAction(int modificationCount);
    void updateEditMenuAction(Editor::Action action, bool available);

private:
    static MainWindow *s_instance;

    Ui::MainWindow *m_ui;
    Document *m_lastCurrentDocument; // owned by DocumentManager
};

#endif // MAINWINDOW_H
