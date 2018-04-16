//
// Zero Editor
// Copyright (C) 2018 Matthias Bolte <matthias.bolte@googlemail.com>
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

#include "eventfilter.h"

#include <QContextMenuEvent>
#include <QLineEdit>
#include <QMenu>

EventFilter *EventFilter::s_instance = NULL;

EventFilter::EventFilter(QObject *parent) :
    QObject(parent)
{
    s_instance = this;
}

EventFilter::~EventFilter()
{
    s_instance = NULL;
}

// protected
bool EventFilter::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::ContextMenu) {
        QLineEdit *edit = qobject_cast<QLineEdit *>(object);

        if (edit != NULL) {
            QContextMenuEvent *contextMenuEvent = static_cast<QContextMenuEvent *>(event);
            QMenu *menu = edit->createStandardContextMenu();
            QList<QAction *> actions = menu->actions();

            for (int i = actions.length() - 1; i >= 0; --i) {
                QAction *action = actions.at(i);
                const QString &text = action->text().split('\t').first();

                // Qt only sets shortcuts for the QLineEdit context menu QActions if there isn't another active
                // shortcut with the same key sequence. But because as such shortcuts still work if the QLineEdit has
                // focus there is no reason to not have them set for the context menu QActions.
                if (text == "&Undo") {
                    action->setShortcut(QKeySequence::Undo);
                } else if (text == "&Redo") {
                    action->setShortcut(QKeySequence::Redo);
                } else if (text == "Cu&t") {
                    action->setShortcut(QKeySequence::Cut);
                } else if (text == "&Copy") {
                    action->setShortcut(QKeySequence::Copy);
                } else if (text == "&Paste") {
                    action->setShortcut(QKeySequence::Paste);
                } else if (text == "Select All") {
                    action->setShortcut(QKeySequence::SelectAll);

                    // Qt doesn't add an icon for the select all action. So it's set here.
                    action->setIcon(QIcon(":/icons/16x16/select-all.png"));
                }
            }

            menu->setAttribute(Qt::WA_DeleteOnClose);
            menu->popup(contextMenuEvent->globalPos());

            return true;
        }
    }

    return QObject::eventFilter(object, event);
}
