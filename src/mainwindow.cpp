//
// Zero Editor
// Copyright (C) 2015-2018 Matthias Bolte <matthias.bolte@googlemail.com>
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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "document.h"
#include "documentmanager.h"
#include "editor.h"
#include "eventfilter.h"
#include "textdocument.h"

#include <QContextMenuEvent>
#include <QDebug>
#include <QLineEdit>
#include <QMessageBox>
#include <QProcess>
#include <QToolButton>
#include <QWidgetAction>

MainWindow *MainWindow::s_instance = NULL;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_lastCurrentDocument(NULL)
{
    s_instance = this;

    m_ui->setupUi(this);

    // File menu
    m_ui->actionSave->setEnabled(false);
    m_ui->actionSave_Tool->setEnabled(m_ui->actionSave->isEnabled());

    m_ui->actionSaveAs->setEnabled(false);

    m_ui->actionSaveAll->setEnabled(false);
    m_ui->actionSaveAll_Tool->setEnabled(m_ui->actionSaveAll->isEnabled());

    m_ui->actionRevert->setEnabled(false);
    m_ui->actionRevert_Tool->setEnabled(m_ui->actionRevert->isEnabled());

    m_ui->actionClose->setEnabled(false);
    m_ui->actionClose_Tool->setEnabled(m_ui->actionClose->isEnabled());

    connect(m_ui->actionNew, &QAction::triggered, this, &MainWindow::newDocument);
    connect(m_ui->actionNew_Tool, &QAction::triggered, this, &MainWindow::newDocument);
    connect(m_ui->actionOpen, &QAction::triggered, this, &MainWindow::openDocuments);
    connect(m_ui->actionOpen_Tool, &QAction::triggered, this, &MainWindow::openDocuments);
    connect(m_ui->actionSave, &QAction::triggered, this, &MainWindow::saveDocument);
    connect(m_ui->actionSave_Tool, &QAction::triggered, this, &MainWindow::saveDocument);
    connect(m_ui->actionSaveAs, &QAction::triggered, this, &MainWindow::saveDocumentAs);
    connect(m_ui->actionSaveAll, &QAction::triggered, this, &MainWindow::saveAllDocuments);
    connect(m_ui->actionSaveAll_Tool, &QAction::triggered, this, &MainWindow::saveAllDocuments);
    connect(m_ui->actionClose, &QAction::triggered, this, &MainWindow::closeDocument);
    connect(m_ui->actionClose_Tool, &QAction::triggered, this, &MainWindow::closeDocument);
    connect(m_ui->actionExit, &QAction::triggered, this, &QMainWindow::close);

    // Edit menu
    m_ui->actionUndo->setEnabled(false);
    m_ui->actionRedo->setEnabled(false);
    m_ui->actionCut->setEnabled(false);
    m_ui->actionCopy->setEnabled(false);
    m_ui->actionPaste->setEnabled(false);
    m_ui->actionDelete->setEnabled(false);
    m_ui->actionSelectAll->setEnabled(false);

    m_ui->actionToggleCase->setEnabled(false);
    m_ui->actionToggleCase_Tool->setEnabled(m_ui->actionToggleCase->isEnabled());

    // FIXME: don't do all this forwading here, but directly connect the signals
    connect(m_ui->actionUndo, &QAction::triggered, this, &MainWindow::undo);
    connect(m_ui->actionRedo, &QAction::triggered, this, &MainWindow::redo);
    connect(m_ui->actionCut, &QAction::triggered, this, &MainWindow::cut);
    connect(m_ui->actionCopy, &QAction::triggered, this, &MainWindow::copy);
    connect(m_ui->actionPaste, &QAction::triggered, this, &MainWindow::paste);
    connect(m_ui->actionDelete, &QAction::triggered, this, &MainWindow::delete_);
    connect(m_ui->actionSelectAll, &QAction::triggered, this, &MainWindow::selectAll);
    connect(m_ui->actionToggleCase, &QAction::triggered, this, &MainWindow::toggleCase);
    connect(m_ui->actionToggleCase_Tool, &QAction::triggered, this, &MainWindow::toggleCase);

    // Search menu
    connect(m_ui->actionFindAndReplace, &QAction::triggered, this, &MainWindow::showFindAndReplaceWidget);
    connect(m_ui->actionFindInFiles, &QAction::triggered, this, &MainWindow::showFindInFilesWidget);

    // Options menu
    m_ui->actionEncoding->setEnabled(false);
    m_ui->actionWordWrapping->setEnabled(false);
    m_ui->actionWordWrapping->setChecked(false);

    connect(m_ui->actionEncoding, &QAction::triggered, this, &MainWindow::showEncodingDialog);
    connect(m_ui->actionWordWrapping, &QAction::triggered, this, &MainWindow::setWordWrapping);

    // Tools menu
    connect(m_ui->actionTerminal, &QAction::triggered, this, &MainWindow::openTerminal);
    connect(m_ui->actionTerminal_Tool, &QAction::triggered, this, &MainWindow::openTerminal);
    connect(m_ui->actionUnsavedDiff, &QAction::triggered, this, &MainWindow::showUnsavedDiffWidget);
    connect(m_ui->actionUnsavedDiff_Tool, &QAction::triggered, this, &MainWindow::showUnsavedDiffWidget);
    connect(m_ui->actionGitDiff, &QAction::triggered, this, &MainWindow::showGitDiffWidget);
    connect(m_ui->actionGitDiff_Tool, &QAction::triggered, this, &MainWindow::showGitDiffWidget);

    connect(m_ui->widgetFindAndReplace, &FindAndReplaceWidget::hideClicked, m_ui->widgetStackedHelpers, &QStackedWidget::hide);
    connect(m_ui->widgetFindInFiles, &FindInFilesWidget::hideClicked, m_ui->widgetStackedHelpers, &QStackedWidget::hide);
    connect(m_ui->widgetUnsavedDiff, &UnsavedDiffWidget::hideClicked, m_ui->widgetStackedHelpers, &QStackedWidget::hide);
    connect(m_ui->widgetGitDiff, &GitDiffWidget::hideClicked, m_ui->widgetStackedHelpers, &QStackedWidget::hide);

    connect(DocumentManager::instance(), &DocumentManager::opened, this, &MainWindow::addEditor);
    connect(DocumentManager::instance(), &DocumentManager::aboutToBeClosed, this, &MainWindow::removeEditor);
    connect(DocumentManager::instance(), &DocumentManager::currentChanged, this, &MainWindow::setCurrentDocument);
    connect(DocumentManager::instance(), &DocumentManager::modificationCountChanged, this, &MainWindow::updateSaveAllAction);

    m_ui->widgetBookmarks->hide();
    m_ui->widgetStackedHelpers->hide();

    // Setup toolbar tooltips
    foreach (QAction *toolBarAction, m_ui->toolBar->actions()) {
        QString name = toolBarAction->objectName();

        if (name.endsWith("_Tool")) {
            name.chop(5);
        }

        QAction *menuAction = findChild<QAction *>(name, Qt::FindDirectChildrenOnly);

        Q_ASSERT(menuAction != NULL);

        toolBarAction->setToolTip(formatToolBarActionToolTip(menuAction));
    }

    // Setup toolbar go-to-line edit
    QLineEdit *editGoToLine = new QLineEdit;

    editGoToLine->installEventFilter(EventFilter::instance());
    editGoToLine->setClearButtonEnabled(true);
    editGoToLine->setPlaceholderText("Line");

    QIntValidator *validatorGoToLine = new QIntValidator(editGoToLine);

    validatorGoToLine->setBottom(0);

    editGoToLine->setValidator(validatorGoToLine);

    QWidget *widgetGoToLine = new QWidget;
    QHBoxLayout *layoutGoToLine = new QHBoxLayout(widgetGoToLine);

    layoutGoToLine->setMargin(0);
    layoutGoToLine->addSpacing(5);
    layoutGoToLine->addWidget(editGoToLine);
    layoutGoToLine->addSpacing(5);

    widgetGoToLine->setFixedWidth(90);

    QWidgetAction *actionGoToLine = new QWidgetAction(this);

    actionGoToLine->setDefaultWidget(widgetGoToLine);

    m_ui->toolBar->insertAction(m_ui->actionGoToLine_Tool, actionGoToLine);

    // Setup toolbar find-quick edit
    QLineEdit *editFindQuick = new QLineEdit;

    editFindQuick->installEventFilter(EventFilter::instance());
    editFindQuick->setClearButtonEnabled(true);
    editFindQuick->setPlaceholderText("Find");

    QWidget *widgetFindQuick = new QWidget;
    QHBoxLayout *layoutFindQuick = new QHBoxLayout(widgetFindQuick);

    layoutFindQuick->setMargin(0);
    layoutFindQuick->addSpacing(5);
    layoutFindQuick->addWidget(editFindQuick);
    layoutFindQuick->addSpacing(5);

    widgetFindQuick->setFixedWidth(300);

    QWidgetAction *actionFindQuick = new QWidgetAction(this);

    actionFindQuick->setDefaultWidget(widgetFindQuick);

    m_ui->toolBar->insertAction(m_ui->actionFindQuick_Tool, actionFindQuick);
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

// private slot
void MainWindow::newDocument()
{
    DocumentManager::create();
}

// private slot
void MainWindow::openDocuments()
{
    DocumentManager::showOpenDialog();
}

// private slot
void MainWindow::saveDocument()
{
    DocumentManager::save(DocumentManager::current());
}

// private slot
void MainWindow::saveDocumentAs()
{
    DocumentManager::showSaveAsDialog(DocumentManager::current());
}

// private slot
void MainWindow::saveAllDocuments()
{
    DocumentManager::saveAll();
}

// private slot
void MainWindow::closeDocument()
{
    DocumentManager::close(DocumentManager::current());
}

// private slot
void MainWindow::undo()
{
    Editor *editor = DocumentManager::editor(DocumentManager::current());

    Q_ASSERT(editor != NULL);

    editor->undo();
}

// private slot
void MainWindow::redo()
{
    Editor *editor = DocumentManager::editor(DocumentManager::current());

    Q_ASSERT(editor != NULL);

    editor->redo();
}

// private slot
void MainWindow::cut()
{
    Editor *editor = DocumentManager::editor(DocumentManager::current());

    Q_ASSERT(editor != NULL);

    editor->cut();
}

// private slot
void MainWindow::copy()
{
    Editor *editor = DocumentManager::editor(DocumentManager::current());

    Q_ASSERT(editor != NULL);

    editor->copy();
}

// private slot
void MainWindow::paste()
{
    Editor *editor = DocumentManager::editor(DocumentManager::current());

    Q_ASSERT(editor != NULL);

    editor->paste();
}

// private slot
void MainWindow::delete_()
{
    Editor *editor = DocumentManager::editor(DocumentManager::current());

    Q_ASSERT(editor != NULL);

    editor->delete_();
}

// private slot
void MainWindow::selectAll()
{
    Editor *editor = DocumentManager::editor(DocumentManager::current());

    Q_ASSERT(editor != NULL);

    editor->selectAll();
}

// private slot
void MainWindow::toggleCase()
{
    Editor *editor = DocumentManager::editor(DocumentManager::current());

    Q_ASSERT(editor != NULL);

    editor->toggleCase();
}

// private slot
void MainWindow::showFindAndReplaceWidget()
{
    m_ui->widgetStackedHelpers->setCurrentIndex(m_ui->widgetStackedHelpers->indexOf(m_ui->widgetFindAndReplace));
    m_ui->widgetStackedHelpers->show();
    m_ui->widgetFindAndReplace->prepareForShow();
}

// private slot
void MainWindow::showFindInFilesWidget()
{
    m_ui->widgetStackedHelpers->setCurrentIndex(m_ui->widgetStackedHelpers->indexOf(m_ui->widgetFindInFiles));
    m_ui->widgetStackedHelpers->show();
    m_ui->widgetFindInFiles->prepareForShow();
}

// private slot
void MainWindow::showEncodingDialog()
{
    DocumentManager::showEncodingDialog(DocumentManager::current());
}

// private slot
void MainWindow::setWordWrapping(bool enable)
{
    Editor *editor = DocumentManager::editor(DocumentManager::current());

    Q_ASSERT(editor != NULL);

    editor->setWordWrapping(enable);
}

// private slot
void MainWindow::openTerminal()
{
    QString program = QString::fromLocal8Bit(qgetenv("COMSPEC")); // FIXME: this is Windows specific
    QStringList arguments;
    QString workingDirectory;

    QProcess::startDetached(program, arguments, workingDirectory);
}

// private slot
void MainWindow::showUnsavedDiffWidget()
{
    m_ui->widgetStackedHelpers->setCurrentIndex(m_ui->widgetStackedHelpers->indexOf(m_ui->widgetUnsavedDiff));
    m_ui->widgetStackedHelpers->show();
}

// private slot
void MainWindow::showGitDiffWidget()
{
    m_ui->widgetStackedHelpers->setCurrentIndex(m_ui->widgetStackedHelpers->indexOf(m_ui->widgetGitDiff));
    m_ui->widgetStackedHelpers->show();
}

// private slot
void MainWindow::addEditor(Document *document)
{
    Editor *editor = DocumentManager::editor(document);

    Q_ASSERT(editor != NULL);

    m_ui->widgetStackedEditors->addWidget(editor->widget());
}

// private slot
void MainWindow::removeEditor(Document *document)
{
    Editor *editor = DocumentManager::editor(document);

    Q_ASSERT(editor != NULL);

    m_ui->widgetStackedEditors->removeWidget(editor->widget());
}

// private slot
void MainWindow::setCurrentDocument(Document *document)
{
    if (m_lastCurrentDocument != NULL) {
        Editor *editor = DocumentManager::editor(m_lastCurrentDocument);

        Q_ASSERT(editor != NULL);

        disconnect(m_lastCurrentDocument, &Document::locationChanged, this, &MainWindow::updateWindowTitle);
        disconnect(m_lastCurrentDocument, &Document::locationChanged, this, &MainWindow::updateFileMenuText);
        disconnect(m_lastCurrentDocument, &Document::modificationChanged, m_ui->actionSave, &QAction::setEnabled);
        disconnect(m_lastCurrentDocument, &Document::modificationChanged, m_ui->actionSave_Tool, &QAction::setEnabled);
        disconnect(m_lastCurrentDocument, &Document::modificationChanged, m_ui->actionRevert, &QAction::setEnabled);
        disconnect(m_lastCurrentDocument, &Document::modificationChanged, m_ui->actionRevert_Tool, &QAction::setEnabled);
        disconnect(editor, &Editor::actionAvailabilityChanged, this, &MainWindow::updateEditMenuAction);

        m_lastCurrentDocument = NULL;
    }

    if (document == NULL) {
        setWindowTitle("Zero Editor");

        // File menu
        m_ui->actionSave->setEnabled(false);
        m_ui->actionSave->setText("Save");
        m_ui->actionSave_Tool->setEnabled(m_ui->actionSave->isEnabled());
        m_ui->actionSave_Tool->setToolTip(formatToolBarActionToolTip(m_ui->actionSave));

        m_ui->actionSaveAs->setEnabled(false);
        m_ui->actionSaveAs->setText("Save As...");

        m_ui->actionRevert->setEnabled(false);
        m_ui->actionRevert->setText("Revert");
        m_ui->actionRevert_Tool->setEnabled(m_ui->actionRevert->isEnabled());
        m_ui->actionRevert_Tool->setToolTip(formatToolBarActionToolTip(m_ui->actionRevert));

        m_ui->actionClose->setEnabled(false);
        m_ui->actionClose->setText("Close");
        m_ui->actionClose_Tool->setEnabled(m_ui->actionClose->isEnabled());
        m_ui->actionClose_Tool->setToolTip(formatToolBarActionToolTip(m_ui->actionClose));

        // Edit menu
        m_ui->actionUndo->setEnabled(false);
        m_ui->actionRedo->setEnabled(false);
        m_ui->actionCut->setEnabled(false);
        m_ui->actionCopy->setEnabled(false);
        m_ui->actionPaste->setEnabled(false);
        m_ui->actionDelete->setEnabled(false);
        m_ui->actionSelectAll->setEnabled(false);

        m_ui->actionToggleCase->setEnabled(false);
        m_ui->actionToggleCase_Tool->setEnabled(m_ui->actionToggleCase->isEnabled());

        // Options menu
        m_ui->actionEncoding->setEnabled(false);
        m_ui->actionWordWrapping->setEnabled(false);
        m_ui->actionWordWrapping->setChecked(false);
    } else {
        const Location &location = document->location();

        updateWindowTitle(location);

        Editor *editor = DocumentManager::editor(document);

        Q_ASSERT(editor != NULL);

        m_ui->widgetStackedEditors->setCurrentWidget(editor->widget());

        // File menu
        updateFileMenuText(location);

        m_ui->actionSave->setEnabled(document->isModified());
        m_ui->actionSave_Tool->setEnabled(m_ui->actionSave->isEnabled());

        m_ui->actionSaveAs->setEnabled(true);

        m_ui->actionRevert->setEnabled(document->isModified());
        m_ui->actionRevert_Tool->setEnabled(m_ui->actionRevert->isEnabled());

        m_ui->actionClose->setEnabled(true);
        m_ui->actionClose_Tool->setEnabled(m_ui->actionClose->isEnabled());

        // Edit menu
        m_ui->actionUndo->setEnabled(editor->isActionAvailable(Editor::Undo));
        m_ui->actionRedo->setEnabled(editor->isActionAvailable(Editor::Redo));
        m_ui->actionCut->setEnabled(editor->isActionAvailable(Editor::Cut));
        m_ui->actionCopy->setEnabled(editor->isActionAvailable(Editor::Copy));
        m_ui->actionPaste->setEnabled(editor->isActionAvailable(Editor::Paste));
        m_ui->actionDelete->setEnabled(editor->isActionAvailable(Editor::Delete));
        m_ui->actionSelectAll->setEnabled(editor->isActionAvailable(Editor::SelectAll));

        m_ui->actionToggleCase->setEnabled(editor->isActionAvailable(Editor::ToggleCase));
        m_ui->actionToggleCase_Tool->setEnabled(editor->isActionAvailable(Editor::ToggleCase));

        // Options menu
        m_ui->actionEncoding->setEnabled(true);
        m_ui->actionWordWrapping->setEnabled(editor->hasFeature(Editor::WordWrapping));
        m_ui->actionWordWrapping->setChecked(editor->isWordWrapping());

        connect(document, &Document::locationChanged, this, &MainWindow::updateWindowTitle);
        connect(document, &Document::locationChanged, this, &MainWindow::updateFileMenuText);
        connect(document, &Document::modificationChanged, m_ui->actionSave, &QAction::setEnabled);
        connect(document, &Document::modificationChanged, m_ui->actionSave_Tool, &QAction::setEnabled);
        connect(document, &Document::modificationChanged, m_ui->actionRevert, &QAction::setEnabled);
        connect(document, &Document::modificationChanged, m_ui->actionRevert_Tool, &QAction::setEnabled);
        connect(editor, &Editor::actionAvailabilityChanged, this, &MainWindow::updateEditMenuAction);

        m_lastCurrentDocument = document;
    }
}

// private slot
void MainWindow::updateWindowTitle(const Location &location)
{
    const QString &fileName = location.fileName("unnamed");
    QString title = fileName + " - ";

    if (!location.isEmpty()) {
        title += location.directoryPath() + " - ";
    }

    setWindowTitle(title + "Zero Editor");
}

// private slot
void MainWindow::updateFileMenuText(const Location &location)
{
    const QString &fileName = location.fileName("unnamed");

    m_ui->actionSave->setText(QString("Save \"%1\"").arg(fileName));
    m_ui->actionSave_Tool->setToolTip(formatToolBarActionToolTip(m_ui->actionSave));

    m_ui->actionSaveAs->setText(QString("Save \"%1\" As...").arg(fileName));

    m_ui->actionRevert->setText(QString("Revert \"%1\"").arg(fileName));
    m_ui->actionRevert_Tool->setToolTip(formatToolBarActionToolTip(m_ui->actionRevert));

    m_ui->actionClose->setText(QString("Close \"%1\"").arg(fileName));
    m_ui->actionClose_Tool->setToolTip(formatToolBarActionToolTip(m_ui->actionClose));
}

// private slot
void MainWindow::updateSaveAllAction(int modificationCount)
{
    m_ui->actionSaveAll->setEnabled(modificationCount > 0);
    m_ui->actionSaveAll->setText(modificationCount > 0 ? QString("Save All (%1)").arg(modificationCount) : "Save All");
    m_ui->actionSaveAll_Tool->setEnabled(m_ui->actionSaveAll->isEnabled());
    m_ui->actionSaveAll_Tool->setToolTip(formatToolBarActionToolTip(m_ui->actionSaveAll));
}

// private slot
void MainWindow::updateEditMenuAction(Editor::Action action, bool available)
{
    switch (action) {
    case Editor::Undo:
        m_ui->actionUndo->setEnabled(available);
        break;

    case Editor::Redo:
        m_ui->actionRedo->setEnabled(available);
        break;

    case Editor::Cut:
        m_ui->actionCut->setEnabled(available);
        break;

    case Editor::Copy:
        m_ui->actionCopy->setEnabled(available);
        break;

    case Editor::Paste:
        m_ui->actionPaste->setEnabled(available);
        break;

    case Editor::Delete:
        m_ui->actionDelete->setEnabled(available);
        break;

    case Editor::SelectAll:
        m_ui->actionSelectAll->setEnabled(available);
        break;

    case Editor::ToggleCase:
        m_ui->actionToggleCase->setEnabled(available);
        m_ui->actionToggleCase_Tool->setEnabled(m_ui->actionToggleCase->isEnabled());
        break;
    }
}

// private static
QString MainWindow::formatToolBarActionToolTip(QAction *menuAction)
{
    const QKeySequence &shortcut = menuAction->shortcut();
    QString name = menuAction->text();

    if (name.endsWith("...")) {
        name.chop(3);
    }

    if (shortcut.isEmpty()) {
        return name;
    } else {
        return name + "<br><small><b>" + shortcut.toString(QKeySequence::NativeText) + "</b></small>";
    }
}
