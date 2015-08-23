
#include "openfileswidget.h"
#include "ui_openfileswidget.h"

OpenFilesWidget::OpenFilesWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OpenFilesWidget)
{
    ui->setupUi(this);

    connect(ui->checkShowFilter, QCheckBox::toggled, ui->editFilter, QLineEdit::setVisible);

    ui->editFilter->setVisible(ui->checkShowFilter->isChecked());
}

OpenFilesWidget::~OpenFilesWidget()
{
    delete ui;
}

void OpenFilesWidget::installLineEditEventFilter(QObject *filter)
{
    ui->editFilter->installEventFilter(filter);
}
