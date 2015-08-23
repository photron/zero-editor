
#include "mainwindow.h"
#include "style.h"

#include <QApplication>
#include <QFontDatabase>

int main(int argc, char **argv)
{
    QApplication application(argc, argv);

    QFontDatabase::addApplicationFont(":/fonts/DejaVuSansMono.ttf");
    QApplication::setStyle(new Style(QApplication::style()));
    QIcon::setThemeName("zero-editor");

    MainWindow mainWindow;

    mainWindow.show();

    return application.exec();
}
