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
#include <QRegularExpression>
#include <QStandardItemModel>
#include <QWidget>

class Document;
class StandardItem;

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
    void setCurrentChild(Document *document);
    void updateCurrentParent(const QModelIndex &index);
    void setModificationMarkerOfSender(bool enable);
    void showModifiedDocumentsOnly(bool enable);
    void setFilterEnabled(bool enable);
    void setFilterPattern(const QString &pattern);

private:
    enum {
        DocumentPointerRole = Qt::UserRole,
        FileNameRole
    };

    void setModificationMarker(Document *document, bool enable);
    void applyFilter();
    bool filterAcceptsChild(const QModelIndex &index) const;

    Ui::OpenDocumentsWidget *m_ui;

    QStandardItemModel m_model;
    QHash<Document *, StandardItem *> m_children; // values owned by QStandardItemModel
    StandardItem *m_lastCurrentChild;

    bool m_showModifiedDocumentsOnly;

    bool m_filterEnabled;
    QRegularExpression m_filterRegularExpression;
};

#endif // OPENDOCUMENTSWIDGET_H
