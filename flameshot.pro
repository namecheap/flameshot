#-------------------------------------------------
#
# Project created by Dharkael 2017-04-21T00:42:49
#
#-------------------------------------------------


QT       += core gui
QT       += x11extras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG    += c++11
CONFIG    += link_pkgconfig
PKGCONFIG += x11

TARGET = flameshot
TEMPLATE = app

TRANSLATIONS = translation/Internationalization_es.ts

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
include(src/singleapplication/singleapplication.pri)
include(src/Qt-Color-Widgets//color_widgets.pri)

DEFINES += QAPPLICATION_CLASS=QApplication

SOURCES += src/main.cpp\
    src/nativeeventfilter.cpp \
    src/controller.cpp \
    src/capture/button.cpp \
    src/capture/buttonhandler.cpp \
    src/infowindow.cpp \
    src/config/configwindow.cpp \
    src/capture/screenshot.cpp \
    src/capture/capturewidget.cpp \
    src/capture/capturemodification.cpp \
    src/capture/colorpicker.cpp \
    src/config/buttonlistview.cpp \
    src/config/uicoloreditor.cpp

HEADERS  += \
    src/nativeeventfilter.h \
    src/controller.h \
    src/capture/button.h \
    src/capture/buttonhandler.h \
    src/infowindow.h \
    src/config/configwindow.h \
    src/capture/screenshot.h \
    src/capture/capturewidget.h \
    src/capture/capturemodification.h \
    src/capture/colorpicker.h \
    src/config/buttonlistview.h \
    src/config/uicoloreditor.h

RESOURCES += \
    graphics.qrc

unix: {
    qmfile.path  = /usr/share/flameshot/translations
    qmfile.files = translation/Internationalization_es.qm
    INSTALLS += qmfile
}
