#-------------------------------------------------
#
# Project created by QtCreator 2018-10-14T22:16:23
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gui
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += ../core/common \
               ../core/gas_parameters \
               ../../include
win32:INCLUDEPATH += ../../include/target_sys/win
linux:INCLUDEPATH += ../../include/target_sys/_nix

build_pass:CONFIG(debug) {
  win32:LIBS += ../../libs/win \
                ../../libs/win/debug
  linux: LIBS += -L$$PWD/../../libs/_nix/debug/ -llmodels \
                 -L$$PWD/../../libs/_nix/ -lpugixml
         PRE_TARGETDEPS += $$PWD/../../libs/_nix/debug/liblmodels.a \
                           $$PWD/../../libs/_nix/pugixml.a

}
linux: LIBS += -L$$PWD/../../libs/_nix/debug/ -llmodels \
               -L$$PWD/../../libs/_nix/ -lpugixml
linux: PRE_TARGETDEPS += $$PWD/../../libs/_nix/debug/liblmodels.a \
                         $$PWD/../../libs/_nix/libpugixml.a
#linux:LIBS += ./pugixml.a  \
#              ./lmodels.a

SOURCES += main.cpp\
        mainwindow.cpp \
    program_data.cpp \
    gui_elements.cpp

HEADERS  += mainwindow.h \
    program_data.h \
    models_errors.h \
    gui_elements.h

FORMS    += mainwindow.ui

# unix:!macx: LIBS += -L$$PWD/../../libs/_nix/ -lpugixml

INCLUDEPATH += $$PWD/../core/common
DEPENDPATH += $$PWD/../core/common

# unix:!macx: PRE_TARGETDEPS += $$PWD/../../libs/_nix/debug/liblmodels.a
