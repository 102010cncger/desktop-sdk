QT       -= core
QT       -= gui

TARGET   = HtmlFileInternal
CONFIG   += app_bundle
TEMPLATE = app

DEFINES += ASC_HIDE_WINDOW

CORE_ROOT_DIR = $$PWD/../../../core
PWD_ROOT_DIR = $$PWD
include($$CORE_ROOT_DIR/Common/base.pri)
include($$CORE_ROOT_DIR/Common/3dParty/icu/icu.pri)

win32 {
    include(./Internal_windows.pri)
}
linux-g++ | linux-g++-64 | linux-g++-32 {
    include(./Internal_linux.pri)

    #CONFIG += build_for_centos6

    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN\'"
    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/converter\'"
    QMAKE_LFLAGS += -static-libstdc++ -static-libgcc

    build_for_centos6 {
        QMAKE_LFLAGS += -Wl,--dynamic-linker=./ld-linux-x86-64.so.2
        DESTDIR = $$DESTDIR/CentOS6
    }
}
