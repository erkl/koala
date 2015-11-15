TEMPLATE = app
TARGET = koala

CONFIG += console
CONFIG -= app_bundle
QT += network webkitwidgets

RESOURCES += ../qrc/koala.qrc

HEADERS += ../src/cookies.h \
           ../src/network.h \
           ../src/sandbox.h \
           ../src/stdio.h \
           ../src/util.h

SOURCES += ../src/cookies.cxx \
           ../src/main.cxx \
           ../src/network.cxx \
           ../src/sandbox.cxx \
           ../src/stdio.cxx \
           ../src/util.cxx
