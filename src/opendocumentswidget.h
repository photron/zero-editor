//
// Zero Editor
// Copyright (C) 2015 Matthias Bolte <matthias.bolte@googlemail.com>
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

#ifndef OPENDOCUMENTSWIDGET_H
#define OPENDOCUMENTSWIDGET_H

#include <QHash>
#include <QRegExp>
#include <QStandardItemModel>
#include <QWidget>

class Document;

namespace Ui {
class OpenDocumentsWidget;
}

class OpenDocumentsWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(OpenDocumentsWidget)

public:
    explicit OpenDocumentsWidget(QWidget *parent = NULL);
    ~OpenDocumentsWidget();

    void installLineEditEventFilter(QObject *filter);

private slots:
    void addDocument(Document *document);
    void setCurrentDocument(const QModelIndex &index);
    void setCurrentItem(Document *document);
    void updateItemModification(bool modified);
    void showModifiedDocumentsOnly(bool enable);
    void setFilterPattern(const QString &pattern);
    void setFilterEnabled(bool enable);

private:
    enum {
        DocumentPointerRole = Qt::UserRole,
        FileNameRole
    };

    void applyFilter();
    bool filterAcceptsChild(const QModelIndex &index) const;

    Ui::OpenDocumentsWidget *m_ui;
    QStandardItemModel m_model;
    QHash<Document *, QStandardItem *> m_items; // values owned by QStandardItemModel
    QStandardItem *m_lastCurrentItem;
    bool m_showModifiedDocumentsOnly;
    QRegExp m_filterRegExp;
    bool m_filterEnabled;
};

#endif // OPENDOCUMENTSWIDGET_H
