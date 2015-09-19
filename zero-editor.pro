TEMPLATE     = app
TARGET       = zero-editor
QT          += core gui widgets
SOURCES     += src/binaryeditor.cpp \
               src/binaryeditorwidget.cpp \
               src/binarydocument.cpp \
               src/bookmarkswidget.cpp \
               src/document.cpp \
               src/documentmanager.cpp \
               src/editor.cpp \
               src/editorcolors.cpp \
               src/findandreplacewidget.cpp \
               src/findinfileswidget.cpp \
               src/gitdiffwidget.cpp \
               src/main.cpp \
               src/mainwindow.cpp \
               src/monospacefontmetrics.cpp \
               src/opendocumentswidget.cpp \
               src/recentfileswidget.cpp \
               src/standarditem.cpp \
               src/style.cpp \
               src/texteditor.cpp \
               src/texteditorwidget.cpp \
               src/textdocument.cpp \
               src/unsaveddiffwidget.cpp
HEADERS     += src/binaryeditor.h \
               src/binaryeditorwidget.h \
               src/binarydocument.h \
               src/bookmarkswidget.h \
               src/document.h \
               src/documentmanager.h \
               src/editor.h \
               src/editorcolors.h \
               src/findandreplacewidget.h \
               src/findinfileswidget.h \
               src/gitdiffwidget.h \
               src/mainwindow.h \
               src/monospacefontmetrics.h \
               src/opendocumentswidget.h \
               src/recentfileswidget.h \
               src/standarditem.h \
               src/style.h \
               src/texteditor.h \
               src/texteditorwidget.h \
               src/textdocument.h \
               src/unsaveddiffwidget.h
FORMS       += src/bookmarkswidget.ui \
               src/findandreplacewidget.ui \
               src/findinfileswidget.ui \
               src/gitdiffwidget.ui \
               src/mainwindow.ui \
               src/opendocumentswidget.ui \
               src/recentfileswidget.ui \
               src/unsaveddiffwidget.ui
RESOURCES   += fonts/fonts.qrc \
               icons/icons.qrc
RC_ICONS     = icons/zero-editor.ico
