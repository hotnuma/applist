TEMPLATE = app
TARGET = applist
CONFIG = c99 link_pkgconfig
DEFINES =
INCLUDEPATH =
PKGCONFIG =

PKGCONFIG += gtk+-3.0
PKGCONFIG += tinyc
PKGCONFIG += tinyui

HEADERS = \
    appwindow.h \
    config.h \
    preferences.h

SOURCES = \
    0Temp.c \
    appwindow.c \
    main.c \
    preferences.c

DISTFILES = \
    data/applist.desktop \
    install.sh \
    License.txt \
    meson.build \
    Readme.md \


