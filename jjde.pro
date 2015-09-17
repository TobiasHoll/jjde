TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    main.cpp

QMAKE_CXXFLAGS += -std=c++14 -g

HEADERS += \
    flags.hpp \
    bytes.hpp \
    constants.hpp \
    objects.hpp \
    types.hpp \
    class.hpp \
    disassembler.hpp \
    instructions.hpp \
    annotater.hpp

OTHER_FILES += \
    resources/Example.java \
    resources/Types.java \
    resources/Structures.java


