TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.c \
    linklist.c \
    linkque.c \
    mailbox.c \
    drive_application.c \
    data_processing.c \
    framebuff.c \
    utf.c

HEADERS += \
    linklist.h \
    linkque.h \
    mailbox.h \
    drive_application.h \
    data_processing.h \
    framebuff.h \
    utf.h

LIBS += -lpthread
