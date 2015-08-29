/*
 * Zero Editor
 * Copyright (C) 2015 Matthias Bolte <matthias.bolte@googlemail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
