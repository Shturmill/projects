QT += core gui widgets
TARGET = myproject
SOURCES += main.cpp lab2.cpp
HEADERS += lab2.h
QMAKE_CXXFLAGS += -fopenmp
LIBS += -fopenmp