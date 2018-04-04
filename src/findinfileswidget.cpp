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

#include "findinfileswidget.h"
#include "ui_findinfileswidget.h"

#include <QLineEdit>

FindInFilesWidget::FindInFilesWidget(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::FindInFilesWidget)
{
    m_ui->setupUi(this);

    connect(m_ui->buttonHide, &QToolButton::clicked, this, &FindInFilesWidget::hideClicked);

    m_ui->comboFind->lineEdit()->setClearButtonEnabled(true);
    m_ui->comboFind->lineEdit()->setPlaceholderText("Find");

    m_ui->comboDirectory->lineEdit()->setClearButtonEnabled(true);
    m_ui->comboDirectory->lineEdit()->setPlaceholderText("Directory");

    m_ui->comboIncludeFilter->lineEdit()->setClearButtonEnabled(true);
    m_ui->comboIncludeFilter->lineEdit()->setPlaceholderText("Include Filter");

    m_ui->comboExcludeFilter->lineEdit()->setClearButtonEnabled(true);
    m_ui->comboExcludeFilter->lineEdit()->setPlaceholderText("Exclude Filter");
}

FindInFilesWidget::~FindInFilesWidget()
{
    delete m_ui;
}

void FindInFilesWidget::installLineEditEventFilter(QObject *filter)
{
    m_ui->comboFind->lineEdit()->installEventFilter(filter);
    m_ui->comboDirectory->lineEdit()->installEventFilter(filter);
    m_ui->comboIncludeFilter->lineEdit()->installEventFilter(filter);
    m_ui->comboExcludeFilter->lineEdit()->installEventFilter(filter);
}

void FindInFilesWidget::prepareForShow()
{
    m_ui->comboFind->setFocus();
    m_ui->comboFind->lineEdit()->selectAll();
}
