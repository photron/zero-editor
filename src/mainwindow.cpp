
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QContextMenuEvent>
#include <QLineEdit>
#include <QProcess>
#include <QToolButton>
#include <QWidgetAction>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionExit, QAction::triggered, this, close);
    connect(ui->actionFindAndReplace, QAction::triggered, this, showFindAndReplaceWidget);
    connect(ui->actionFindInFiles, QAction::triggered, this, showFindInFilesWidget);
    connect(ui->actionTerminal, QAction::triggered, this, openTerminal);
    connect(ui->actionTerminal_Tool, QAction::triggered, this, openTerminal);
    connect(ui->widgetFindAndReplace, FindAndReplaceWidget::hideClicked, ui->widgetStackedSearches, QStackedWidget::hide);
    connect(ui->widgetFindInFiles, FindInFilesWidget::hideClicked, ui->widgetStackedSearches, QStackedWidget::hide);

    ui->widgetOpenFiles->installLineEditEventFilter(this);
    ui->widgetFindAndReplace->installLineEditEventFilter(this);
    ui->widgetFindInFiles->installLineEditEventFilter(this);

    ui->widgetStackedSearches->hide();

    ui->actionSave->setText("Save \"brickd.c\"");
    ui->actionSave_Tool->setToolTip("Save \"brickd.c\"");
    ui->actionSaveAs->setText("Save \"brickd.c\" As...");
    ui->actionSaveAll->setText("Save All (2 Files)");
    ui->actionSaveAll_Tool->setText("Save All (2 Files)");
    ui->actionRevert->setText("Revert \"brickd.c\"");
    ui->actionRevert_Tool->setToolTip("Revert \"brickd.c\"");
    ui->actionClose->setText("Close \"brickd.c\"");
    ui->actionClose_Tool->setToolTip("Close \"brickd.c\"");

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

    ui->toolBar->insertAction(ui->actionGoToLine_Tool, actionGoToLine);

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

    ui->toolBar->insertAction(ui->actionFindQuick_Tool, actionFindQuick);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// protected
bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    QLineEdit *edit = qobject_cast<QLineEdit *>(object);

    if (edit != NULL) {
        if (event->type() == QEvent::ContextMenu) {
            QContextMenuEvent *contextMenuEvent = static_cast<QContextMenuEvent *>(event);
            QMenu *menu = edit->createStandardContextMenu();

            if (menu != NULL) {
                foreach (QAction *action, menu->actions()) {
                    if (action->text() == "Select All") {
                        action->setIcon(QIcon(":/icons/16x16/select-all.png"));

                        break;
                    }
                }

                menu->setAttribute(Qt::WA_DeleteOnClose);
                menu->popup(contextMenuEvent->globalPos());
            }

            return true;
        } else {
            return false;
        }
    } else {
        return QMainWindow::eventFilter(object, event);
    }
}

// private slot
void MainWindow::showFindAndReplaceWidget()
{
    ui->widgetStackedSearches->setCurrentIndex(ui->widgetStackedSearches->indexOf(ui->widgetFindAndReplace));
    ui->widgetStackedSearches->show();
    ui->widgetFindAndReplace->prepareForShow();
}

// private slot
void MainWindow::showFindInFilesWidget()
{
    ui->widgetStackedSearches->setCurrentIndex(ui->widgetStackedSearches->indexOf(ui->widgetFindInFiles));
    ui->widgetStackedSearches->show();
    ui->widgetFindInFiles->prepareForShow();
}

// private slot
void MainWindow::openTerminal()
{
    QString program = QString::fromLocal8Bit(qgetenv("COMSPEC")); // FIXME: this is Windows specific
    QStringList arguments;
    QString workingDirectory;

    QProcess::startDetached(program, arguments, workingDirectory);
}
