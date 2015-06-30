TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    main.cpp

QMAKE_CXXFLAGS += -O3 -std=c++14 -g

HEADERS += \
    flags.hpp \
    bytes.hpp \
    constants.hpp \
    objects.hpp

OTHER_FILES += \
    resources/Example.java

