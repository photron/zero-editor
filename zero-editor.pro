TEMPLATE     = app
TARGET       = zero-editor
QT          += core gui widgets
SOURCES     += src/binaryeditorwidget.cpp \
               src/bookmarkswidget.cpp \
               src/editorcolors.cpp \
               src/findandreplacewidget.cpp \
               src/findinfileswidget.cpp \
               src/gitdiffwidget.cpp \
               src/main.cpp \
               src/mainwindow.cpp \
               src/monospacefontmetrics.cpp \
               src/openfileswidget.cpp \
               src/recentfileswidget.cpp \
               src/style.cpp \
               src/texteditorwidget.cpp \
               src/unsaveddiffwidget.cpp
HEADERS     += src/binaryeditorwidget.h \
               src/bookmarkswidget.h \
               src/editorcolors.h \
               src/findandreplacewidget.h \
               src/findinfileswidget.h \
               src/gitdiffwidget.h \
               src/mainwindow.h \
               src/monospacefontmetrics.h \
               src/openfileswidget.h \
               src/recentfileswidget.h \
               src/style.h \
               src/texteditorwidget.h \
               src/unsaveddiffwidget.h
FORMS       += src/bookmarkswidget.ui \
               src/findandreplacewidget.ui \
               src/findinfileswidget.ui \
               src/gitdiffwidget.ui \
               src/mainwindow.ui \
               src/openfileswidget.ui \
               src/recentfileswidget.ui \
               src/unsaveddiffwidget.ui
RESOURCES   += fonts/fonts.qrc \
               icons/icons.qrc
RC_ICONS     = icons/zero-editor.ico
