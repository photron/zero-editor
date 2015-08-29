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
