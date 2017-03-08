!contains( included_modules, $$PWD ) {
    included_modules += $$PWD
    QT += core gui widgets

    !include($$PWD/PluginInterface/UrgBenriPluginInterface.pri) {
            error("Unable to include Viewer Plugin Interface.")
    }

    !include($$PWD/QUrgLib/QUrgLib.pri) {
            error("Unable to include QUrg Library.")
    }

    DEPENDPATH += $$PWD
    INCLUDEPATH += $$PWD

    SOURCES += \
        $$PWD/LogRecorderHelperPlugin.cpp \
        $$PWD/diskUtils.cpp

    HEADERS  += \
        $$PWD/LogRecorderHelperPlugin.h \
        $$PWD/diskUtils.h

    FORMS += \
        $$PWD/LogRecorderHelperPlugin.ui

    RESOURCES += \
        $$PWD/LogRecorderHelperPlugin.qrc

    TRANSLATIONS = $$PWD/i18n/plugin_fr.ts \
            $$PWD/i18n/plugin_en.ts \
            $$PWD/i18n/plugin_ja.ts
}
