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
               src/encodingdialog.cpp \
               src/eventfilter.cpp \
               src/filedialog.cpp \
               src/fileswidget.cpp \
               src/findandreplacewidget.cpp \
               src/findinfileswidget.cpp \
               src/gitdiffwidget.cpp \
               src/lexer.cpp \
               src/location.cpp \
               src/main.cpp \
               src/mainwindow.cpp \
               src/monospacefontmetrics.cpp \
               src/openfileswidget.cpp \
               src/recentfileswidget.cpp \
               src/settings.cpp \
               src/style.cpp \
               src/syntaxhighlighter.cpp \
               src/textcodec.cpp \
               src/texteditor.cpp \
               src/texteditorwidget.cpp \
               src/textdocument.cpp \
               src/unsaveddiffwidget.cpp \
               src/utils.cpp
HEADERS     += src/binaryeditor.h \
               src/binaryeditorwidget.h \
               src/binarydocument.h \
               src/bookmarkswidget.h \
               src/document.h \
               src/documentmanager.h \
               src/editor.h \
               src/editorcolors.h \
               src/encodingdialog.h \
               src/eventfilter.h \
               src/filedialog.h \
               src/fileswidget.h \
               src/findandreplacewidget.h \
               src/findinfileswidget.h \
               src/gitdiffwidget.h \
               src/lexer.h \
               src/location.h \
               src/mainwindow.h \
               src/monospacefontmetrics.h \
               src/openfileswidget.h \
               src/recentfileswidget.h \
               src/settings.h \
               src/style.h \
               src/syntaxhighlighter.h \
               src/textcodec.h \
               src/texteditor.h \
               src/texteditorwidget.h \
               src/textdocument.h \
               src/unsaveddiffwidget.h \
               src/utils.h
FORMS       += src/bookmarkswidget.ui \
               src/encodingdialog.ui \
               src/filedialog.ui \
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
