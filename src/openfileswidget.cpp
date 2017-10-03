//
// Zero Editor
// Copyright (C) 2015-2017 Matthias Bolte <matthias.bolte@googlemail.com>
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

#include "openfileswidget.h"
#include "ui_openfileswidget.h"

#include "documentmanager.h"

OpenFilesWidget::OpenFilesWidget(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::OpenFilesWidget)
{
    m_ui->setupUi(this);

    m_ui->editFilter->setVisible(m_ui->checkFilter->isChecked());

    connect(m_ui->radioModified, &QRadioButton::toggled, m_ui->widgetFiles, &FilesWidget::showModifiedDocumentsOnly);
    connect(m_ui->checkFilter, &QCheckBox::toggled, m_ui->widgetFiles, &FilesWidget::setFilterEnabled);
    connect(m_ui->checkFilter, &QCheckBox::toggled, this, &OpenFilesWidget::setFilterVisible);
    connect(m_ui->editFilter, &QLineEdit::textChanged, m_ui->widgetFiles, &FilesWidget::setFilterPattern);
    connect(m_ui->widgetFiles, &FilesWidget::filterValidityChanged, this, &OpenFilesWidget::markFilterAsValid);
    connect(DocumentManager::instance(), &DocumentManager::modificationCountChanged, this, &OpenFilesWidget::updateModifiedText);
}

OpenFilesWidget::~OpenFilesWidget()
{
    delete m_ui;
}

void OpenFilesWidget::installLineEditEventFilter(QObject *filter)
{
    m_ui->editFilter->installEventFilter(filter);
}

// private slot
void OpenFilesWidget::setFilterVisible(bool visible)
{
    m_ui->editFilter->setVisible(visible);

    if (visible) {
        m_ui->editFilter->setFocus();
    }
}

// private slot
void OpenFilesWidget::markFilterAsValid(bool mark)
{
    m_ui->editFilter->setStyleSheet(mark ? "" : "QLineEdit { color: red }");
}

// private slot
void OpenFilesWidget::updateModifiedText(int modificationCount)
{
    m_ui->radioModified->setText(modificationCount > 0 ? QString("Modified (%1)").arg(modificationCount) : "Modified");
}
