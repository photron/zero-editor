//
// Zero Editor
// Copyright (C) 2015-2016 Matthias Bolte <matthias.bolte@googlemail.com>
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

#include <QDebug>
#include <QTextCodec>

EncodingDialog::EncodingDialog(QTextCodec *codec, bool byteOrderMark, QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::EncodingDialog)
{
    m_ui->setupUi(this);

    QListWidgetItem *current = NULL;

    foreach (int mib, QTextCodec::availableMibs()) {
        QTextCodec *other = QTextCodec::codecForMib(mib);
        QString name = ((QByteArrayList() << other->name()) + other->aliases()).join(" | ");
        QListWidgetItem *item = new QListWidgetItem(name);

        if (current == NULL && mib == codec->mibEnum()) {
            current = item;

            QFont font(item->font());

            font.setUnderline(true);

            item->setFont(font);
        }

        item->setData(Qt::UserRole, qVariantFromValue(mib));

        m_ui->listCodecs->addItem(item);
    }

    m_ui->listCodecs->sortItems();
    m_ui->listCodecs->insertItem(0, new QListWidgetItem("None"));

    if (current != NULL) {
        m_ui->listCodecs->scrollToItem(current);
        current->setSelected(true);
    }

    m_ui->checkUnicodeBOM->setChecked(byteOrderMark);

    connect(m_ui->listCodecs, &QListWidget::currentItemChanged, this, EncodingDialog::setCodec);
    connect(m_ui->listCodecs, &QListWidget::itemActivated, this, EncodingDialog::setCodecAndAccept);
    connect(m_ui->checkUnicodeBOM, &QCheckBox::toggled, this, EncodingDialog::setByteOrderMark);
    connect(m_ui->buttonSelect, &QPushButton::clicked, this, EncodingDialog::accept);
    connect(m_ui->buttonCancel, &QPushButton::clicked, this, EncodingDialog::reject);
}

EncodingDialog::~EncodingDialog()
{
    delete m_ui;
}

// private slot
void EncodingDialog::setCodec(QListWidgetItem *item)
{
    bool ok;
    int mib = item->data(Qt::UserRole).toInt(&ok);

    if (!ok) {
        m_codec = NULL;
    } else {
        QTextCodec *codec = QTextCodec::codecForMib(mib);

        if (codec != NULL) {
            m_codec = codec;
        }
    }
}

// private slot
void EncodingDialog::setCodecAndAccept(QListWidgetItem *item)
{
    setCodec(item);
    accept();
}

// private slot
void EncodingDialog::setByteOrderMark(bool byteOrderMark)
{
    m_byteOrderMark = byteOrderMark;
}
