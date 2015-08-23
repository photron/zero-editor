
#include "mainwindow.h"
#include "style.h"

#include <QApplication>

int main(int argc, char **argv)
{
    QApplication application(argc, argv);

    QApplication::setStyle(new Style(QApplication::style()));
    QIcon::setThemeName("zero-editor");

    MainWindow mainWindow;

    mainWindow.show();

    return application.exec();
}
