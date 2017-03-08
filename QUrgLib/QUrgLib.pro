QT       += core testlib
QT       -= gui

TARGET = QUrgLibTest
#CONFIG   += console
#CONFIG   -= app_bundle

TEMPLATE = app

!include(QUrgLib.pri) {
        error("Unable to include QUrgLib Library.")
}

SOURCES += \
#    test/main.cpp \
    test/TestUrgDevice.cpp \
    test/CustomConnection.cpp

HEADERS += \
    test/TestUrgDevice.h \
    test/CustomConnection.h

RESOURCES += \
    QUrgLib.qrc
