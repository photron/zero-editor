TEMPLATE     = app
TARGET       = zero-editor
QT          += core gui widgets
SOURCES     += src/findandreplacewidget.cpp \
               src/findinfileswidget.cpp \
               src/main.cpp \
               src/mainwindow.cpp \
               src/openfileswidget.cpp \
               src/recentfileswidget.cpp \
               src/style.cpp \
               src/texteditorwidget.cpp
HEADERS     += src/findandreplacewidget.h \
               src/findinfileswidget.h \
               src/mainwindow.h \
               src/openfileswidget.h \
               src/recentfileswidget.h \
               src/style.h \
               src/texteditorwidget.h
FORMS       += src/findandreplacewidget.ui \
               src/findinfileswidget.ui \
               src/mainwindow.ui \
               src/openfileswidget.ui \
               src/recentfileswidget.ui
RESOURCES   += fonts/fonts.qrc \
               icons/icons.qrc
RC_ICONS     = icons/zero-editor.ico
