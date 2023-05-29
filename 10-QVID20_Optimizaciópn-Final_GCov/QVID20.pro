QT += widgets network sql

SOURCES += \
    adminbd.cpp \
    main.cpp \
    netcsv.cpp

HEADERS += \
    adminbd.h \
    netcsv.h

CONFIG += debug

QMAKE_CXXFLAGS_DEBUG *= -ftest-coverage -fprofile-arcs
QMAKE_LFLAGS_DEBUG *= -ftest-coverage -fprofile-arcs
