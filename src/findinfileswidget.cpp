
#include "findinfileswidget.h"
#include "ui_findinfileswidget.h"

#include <QLineEdit>

FindInFilesWidget::FindInFilesWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FindInFilesWidget)
{
    ui->setupUi(this);

    connect(ui->buttonHide, &QToolButton::clicked, this, &FindInFilesWidget::hideClicked);

    ui->comboFind->lineEdit()->setClearButtonEnabled(true);
    ui->comboFind->lineEdit()->setPlaceholderText("Find");

    ui->comboDirectory->lineEdit()->setClearButtonEnabled(true);
    ui->comboDirectory->lineEdit()->setPlaceholderText("Directory");

    ui->comboFilter->lineEdit()->setClearButtonEnabled(true);
    ui->comboFilter->lineEdit()->setPlaceholderText("Filter");
}

FindInFilesWidget::~FindInFilesWidget()
{
    delete ui;
}

void FindInFilesWidget::installLineEditEventFilter(QObject *filter)
{
    ui->comboFind->lineEdit()->installEventFilter(filter);
    ui->comboDirectory->lineEdit()->installEventFilter(filter);
    ui->comboFilter->lineEdit()->installEventFilter(filter);
}

void FindInFilesWidget::prepareForShow()
{
    ui->comboFind->setFocus();
    ui->comboFind->lineEdit()->selectAll();
}
