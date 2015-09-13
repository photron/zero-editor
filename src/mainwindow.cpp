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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "document.h"
#include "editor.h"

#include <QContextMenuEvent>
#include <QDebug>
#include <QFileDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QProcess>
#include <QToolButton>
#include <QWidgetAction>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_documentManager(new DocumentManager(this))
{
    m_ui->setupUi(this);

    connect(m_ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(m_ui->actionOpen_Tool, &QAction::triggered, this, &MainWindow::openFile);
    connect(m_ui->actionExit, &QAction::triggered, this, &QMainWindow::close);
    connect(m_ui->actionToggleCase, &QAction::triggered, this, &MainWindow::toggleCase);
    connect(m_ui->actionToggleCase_Tool, &QAction::triggered, this, &MainWindow::toggleCase);
    connect(m_ui->actionFindAndReplace, &QAction::triggered, this, &MainWindow::showFindAndReplaceWidget);
    connect(m_ui->actionFindInFiles, &QAction::triggered, this, &MainWindow::showFindInFilesWidget);
    connect(m_ui->actionWordWrapping, &QAction::triggered, this, &MainWindow::setWordWrapping);
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

    connect(DocumentManager::instance(), &DocumentManager::documentOpened, this, &MainWindow::addDocument);
    connect(DocumentManager::instance(), &DocumentManager::currentDocumentChanged, this, &MainWindow::setCurrentDocument);

    m_ui->widgetOpenDocuments->installLineEditEventFilter(this);
    m_ui->widgetFindAndReplace->installLineEditEventFilter(this);
    m_ui->widgetFindInFiles->installLineEditEventFilter(this);

    m_ui->widgetBookmarks->hide();
    m_ui->widgetStackedHelpers->hide();

    m_ui->actionSave->setText("Save \"brickd.c\"");
    m_ui->actionSave_Tool->setToolTip("Save \"brickd.c\"");
    m_ui->actionSaveAs->setText("Save \"brickd.c\" As...");
    m_ui->actionSaveAll->setText("Save All (2 Files)");
    m_ui->actionSaveAll_Tool->setText("Save All (2 Files)");
    m_ui->actionRevert->setText("Revert \"brickd.c\"");
    m_ui->actionRevert_Tool->setToolTip("Revert \"brickd.c\"");
    m_ui->actionClose->setText("Close \"brickd.c\"");
    m_ui->actionClose_Tool->setToolTip("Close \"brickd.c\"");

    // Setup toolbar go-to-line edit
    QLineEdit *editGoToLine = new QLineEdit();

    editGoToLine->installEventFilter(this);
    editGoToLine->setClearButtonEnabled(true);
    editGoToLine->setPlaceholderText("Line");

    QIntValidator *validatorGoToLine = new QIntValidator(editGoToLine);

    validatorGoToLine->setBottom(0);

    editGoToLine->setValidator(validatorGoToLine);

    QWidget *widgetGoToLine = new QWidget();
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
    QLineEdit *editFindQuick = new QLineEdit();

    editFindQuick->installEventFilter(this);
    editFindQuick->setClearButtonEnabled(true);
    editFindQuick->setPlaceholderText("Find");

    QWidget *widgetFindQuick = new QWidget();
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
    delete m_documentManager;
    delete m_ui;
}

// protected
bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::ContextMenu) {
        QLineEdit *edit = qobject_cast<QLineEdit *>(object);

        if (edit != NULL) {
            QContextMenuEvent *contextMenuEvent = static_cast<QContextMenuEvent *>(event);
            QMenu *menu = edit->createStandardContextMenu();
            QList<QAction *> actions = menu->actions();

            for (int i = actions.length() - 1; i >= 0; --i) {
                QAction *action = actions.at(i);

                if (action->text() == "Select All") {
                    action->setIcon(QIcon(":/icons/16x16/select-all.png"));

                    break;
                }
            }

            menu->setAttribute(Qt::WA_DeleteOnClose);
            menu->popup(contextMenuEvent->globalPos());

            return true;
        }
    }

    return QMainWindow::eventFilter(object, event);
}

// private slot
void MainWindow::openFile()
{
    QStringList filePaths = QFileDialog::getOpenFileNames(this, "Open File");
    QString error;

    foreach (const QString &filePath, filePaths) {
        if (!DocumentManager::open(filePath, &error)) {
            if (error.isEmpty()) {
                error = QString("Could not open '%1': Unknown error").arg(QDir::toNativeSeparators(filePath));
            }

            QMessageBox::critical(this, "File Open Error", error);
        }
    }
}

// private slot
void MainWindow::toggleCase()
{
    Document *document = DocumentManager::currentDocument();

    if (document == NULL) {
        return;
    }

    Editor *editor = DocumentManager::editorForDocument(document);

    Q_ASSERT(editor);

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
void MainWindow::setWordWrapping(bool enable)
{
    Document *document = DocumentManager::currentDocument();

    if (document == NULL) {
        return;
    }

    Editor *editor = DocumentManager::editorForDocument(document);

    Q_ASSERT(editor);

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
void MainWindow::addDocument(Document *document)
{
    Q_ASSERT(document);

    Editor *editor = DocumentManager::editorForDocument(document);

    Q_ASSERT(editor);

    m_ui->widgetStackedEditors->addWidget(editor->widget());
}

// private slot
void MainWindow::setCurrentDocument(Document *document)
{
    Q_ASSERT(document);

    QFileInfo fileInfo(document->filePath());
    QString absolutePath(QDir::toNativeSeparators(fileInfo.absolutePath()));
    QString fileName(fileInfo.fileName());

    setWindowTitle(fileName + " - " + absolutePath + " - Zero Editor");

    Editor *editor = DocumentManager::editorForDocument(document);

    Q_ASSERT(editor);

    m_ui->widgetStackedEditors->setCurrentWidget(editor->widget());

    m_ui->actionToggleCase->setEnabled(editor->hasFeature(Editor::ToggleCase));
    m_ui->actionToggleCase_Tool->setEnabled(editor->hasFeature(Editor::ToggleCase));

    m_ui->actionWordWrapping->setEnabled(editor->hasFeature(Editor::WordWrapping));
    m_ui->actionWordWrapping->setChecked(editor->isWordWrapping());
}
