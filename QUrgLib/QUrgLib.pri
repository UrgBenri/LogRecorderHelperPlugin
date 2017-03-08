!contains( included_modules, $$PWD ) {
    included_modules += $$PWD
QT       += core

DEPENDPATH += "$$PWD"/src
INCLUDEPATH += "$$PWD"/src

win32:LIBS += -lwsock32 -lsetupapi -ladvapi32

HEADERS += \
    $$PWD/src/UrgUsbCom.h \
    $$PWD/src/UrgLogHandler.h \
    $$PWD/src/UrgDevice.h \
    $$PWD/src/ScipHandler.h \
    $$PWD/src/IsUsbCom.h \
    $$PWD/src/isUsingComDriver.h \
    $$PWD/src/RangeCaptureMode.h \
    $$PWD/src/RangeSensorParameter.h \
    $$PWD/src/RangeSensorInternalInformation.h \
    $$PWD/src/RangeSensorInformation.h \
    $$PWD/src/delay.h \
    $$PWD/src/RangeSensor.h \
    $$PWD/src/Connection.h \
    $$PWD/src/SerialDevice.h \
    $$PWD/src/CaptureSettings.h \
    $$PWD/src/ticks.h \
    $$PWD/src/Thread.h \
    $$PWD/src/log_printf.h \
    $$PWD/src/MathUtils.h \
    $$PWD/src/ConnectionUtils.h \
    $$PWD/src/Position.h \
    $$PWD/src/Angle.h \
    $$PWD/src/RingBuffer.h \
    $$PWD/src/TcpDevice.h \
    $$PWD/src/FindComPorts.h \
    $$PWD/src/BasicExcel.hpp

SOURCES += \
    $$PWD/src/UrgUsbCom.cpp \
    $$PWD/src/UrgLogHandler.cpp \
    $$PWD/src/UrgDevice.cpp \
    $$PWD/src/ScipHandler.cpp \
    $$PWD/src/isUsingComDriver.cpp \
    $$PWD/src/SerialDevice_win.cpp \
    $$PWD/src/SerialDevice_lin.cpp \
    $$PWD/src/SerialDevice.cpp \
    $$PWD/src/log_printf.cpp \
    $$PWD/src/MathUtils.cpp \
    $$PWD/src/ConnectionUtils.cpp \
    $$PWD/src/Angle.cpp \
    $$PWD/src/ticks.cpp \
    $$PWD/src/Thread.cpp \
    $$PWD/src/delay.cpp \
    $$PWD/src/TcpDevice.cpp \
    $$PWD/src/FindComPorts.cpp \
    $$PWD/src/BasicExcel.cpp \
    $$PWD/src/Connection.cpp
}

HEADERS += \
    $$PWD/src/Converter.h

SOURCES += \
    $$PWD/src/Converter.cpp
