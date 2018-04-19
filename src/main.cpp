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

#include "documentmanager.h"
#include "editorcolors.h"
#include "eventfilter.h"
#include "mainwindow.h"
#include "monospacefontmetrics.h"
#include "settings.h"
#include "style.h"
#include "textcodec.h"

#include <QApplication>
#include <QDebug>
#include <QFontDatabase>

int main(int argc, char **argv)
{
    QApplication application(argc, argv);

    application.setWindowIcon(QIcon(":/icons/zero-editor.ico"));

    if (QFontDatabase::addApplicationFont(":/fonts/DejaVuSansMono.ttf") < 0) {
        qDebug() << "main: Loading DejaVuSansMono.ttf failed";
    }

    Settings::initialize();
    TextCodec::initialize();
    MonospaceFontMetrics::initialize();
    EditorColors::initialize();
    QApplication::setStyle(new Style(QApplication::style()));
    QIcon::setThemeName("zero-editor");

    EventFilter eventFilter;
    DocumentManager documentManager;
    MainWindow mainWindow;

    mainWindow.show();

    return application.exec();
}
