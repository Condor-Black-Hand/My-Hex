QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

DESTDIR = $$PWD/bin
include($$PWD/src/src.pri)
include($$PWD/lib/lib.pri)

RESOURCES += \
    res/res.qrc
