QT += core widgets serialport

TARGET = camm-grinder
TEMPLATE = app

CONFIG += c++17

SOURCES += \
    src/main.cpp \
    src/MainWindow.cpp \
    src/SerialManager.cpp \
    src/CommandFactory.cpp \
    src/DeviceControlWidget.cpp \
    src/ManualControlWidget.cpp \
    src/SettingsManager.cpp

HEADERS += \
    src/MainWindow.h \
    src/SerialManager.h \
    src/CommandFactory.h \
    src/DeviceControlWidget.h \
    src/ManualControlWidget.h \
    src/SettingsManager.h

RESOURCES += \
    resources/resources.qrc

debug {
    CONFIG += debug
    DEFINES += DEBUG
}

release {
    CONFIG += release
    DEFINES += QT_NO_DEBUG_OUTPUT
    QMAKE_CXXFLAGS_RELEASE += -O2
}

win32: {
    QMAKE_LFLAGS += /MANIFESTUAC:\"level=\'requireAdministrator\' uiAccess=\'false\'\"
}

VERSION = 1.0.0
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
