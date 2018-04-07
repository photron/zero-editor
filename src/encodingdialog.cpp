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

#include "encodingdialog.h"
#include "ui_encodingdialog.h"

#include "textcodec.h"

#include <QDebug>

EncodingDialog::EncodingDialog(TextCodec *codec, QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::EncodingDialog),
    m_codec(codec)
{
    m_ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QListWidgetItem *current = NULL;

    foreach (qint64 number, TextCodec::knownNumbers()) {
        TextCodec *other = TextCodec::fromNumber(number);
        QListWidgetItem *item = new QListWidgetItem(other->name());

        if (current == NULL && codec != NULL && codec->number() == number) {
            current = item;

            QFont font(item->font());

            font.setUnderline(true);

            item->setFont(font);
        }

        item->setData(Qt::UserRole, qVariantFromValue(number));

        m_ui->listCodecs->addItem(item);

        foreach (const QString &alias, other->aliases()) {
            QListWidgetItem *item = new QListWidgetItem(alias + " \u2192 " + other->name());

            item->setData(Qt::UserRole, qVariantFromValue(number));

            m_ui->listCodecs->addItem(item);
        }
    }

    m_ui->listCodecs->sortItems();

    QListWidgetItem *item = new QListWidgetItem("Binary");

    if (current == NULL && codec == NULL) {
        current = item;

        QFont font(item->font());

        font.setUnderline(true);

        item->setFont(font);
    }

    m_ui->listCodecs->insertItem(0, item);

    if (current != NULL) {
        m_ui->listCodecs->scrollToItem(current);
        current->setSelected(true);
    }

    m_ui->listCodecs->setFocus();

    connect(m_ui->listCodecs, &QListWidget::currentItemChanged, this, &EncodingDialog::setCodec);
    connect(m_ui->listCodecs, &QListWidget::itemActivated, this, &EncodingDialog::setCodecAndAccept);
    connect(m_ui->buttonSelect, &QPushButton::clicked, this, &EncodingDialog::accept);
    connect(m_ui->buttonCancel, &QPushButton::clicked, this, &EncodingDialog::reject);
}

EncodingDialog::~EncodingDialog()
{
    delete m_ui;
}

// private slot
void EncodingDialog::setCodec(QListWidgetItem *item)
{
    bool ok;
    qint64 number = item->data(Qt::UserRole).toLongLong(&ok);

    if (!ok) {
        m_codec = NULL;
    } else {
        m_codec = TextCodec::fromNumber(number);
    }
}

// private slot
void EncodingDialog::setCodecAndAccept(QListWidgetItem *item)
{
    setCodec(item);
    accept();
}
