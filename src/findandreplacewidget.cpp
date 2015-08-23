
#include "findandreplacewidget.h"
#include "ui_findandreplacewidget.h"

#include <QLineEdit>

FindAndReplaceWidget::FindAndReplaceWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FindAndReplaceWidget)
{
    ui->setupUi(this);

    connect(ui->buttonHide, &QToolButton::clicked, this, &FindAndReplaceWidget::hideClicked);

    ui->comboFind->lineEdit()->setClearButtonEnabled(true);
    ui->comboFind->lineEdit()->setPlaceholderText("Find");

    ui->comboReplace->lineEdit()->setClearButtonEnabled(true);
    ui->comboReplace->lineEdit()->setPlaceholderText("Replace");
}

FindAndReplaceWidget::~FindAndReplaceWidget()
{
    delete ui;
}

void FindAndReplaceWidget::installLineEditEventFilter(QObject *filter)
{
    ui->comboFind->lineEdit()->installEventFilter(filter);
    ui->comboReplace->lineEdit()->installEventFilter(filter);
}

void FindAndReplaceWidget::prepareForShow()
{
    ui->comboFind->setFocus();
    ui->comboFind->lineEdit()->selectAll();
}
