#-------------------------------------------------
#
# Project created by QtCreator 2024-02-22T11:18:43
#
#-------------------------------------------------

QT       += core gui sql

QMAKE_CXXFLAGS += -fpermissive -Wunused-parameter -Wunused-but-set-variable -Wunused-variable -Wreturn-type

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FILE-MANAGER
TEMPLATE = app

win32:RC_FILE = file.rc

SOURCES += main.cpp\
        MainWindow.cpp \
    Pane.cpp \
    PreferencesDialog.cpp \
    Properties.cpp

HEADERS  += MainWindow.h \
    Pane.h \
    PreferencesDialog.h \
    Properties.h

RESOURCES += \
    Resources.qrc











