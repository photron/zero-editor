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

#ifndef ENCODINGDIALOG_H
#define ENCODINGDIALOG_H

#include <QDialog>

class QListWidgetItem;

namespace Ui {
class EncodingDialog;
}

class EncodingDialog : public QDialog
{
    Q_OBJECT

public:
    EncodingDialog(QTextCodec *codec, bool byteOrderMark, QWidget *parent = NULL);
    ~EncodingDialog();

    QTextCodec *codec() const { return m_codec; }
    bool byteOrderMark() const { return m_byteOrderMark; }

private slots:
    void setCodec(QListWidgetItem *item);
    void setCodecAndAccept(QListWidgetItem *item);
    void setByteOrderMark(bool byteOrderMark);

private:
    Ui::EncodingDialog *m_ui;
    QTextCodec *m_codec;
    bool m_byteOrderMark;
};

#endif // ENCODINGDIALOG_H
