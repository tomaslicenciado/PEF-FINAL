QT += widgets network sql

SOURCES += \
    adminbd.cpp \
    main.cpp \
    netcsv.cpp

HEADERS += \
    adminbd.h \
    netcsv.h

LIBS += -lgcov

CONFIG += debug

QMAKE_CXXFLAGS_DEBUG += -coverage -ftest-coverage -fprofile-arcs
QMAKE_LFLAGS_DEBUG += -coverage -ftest-coverage -fprofile-arcs
