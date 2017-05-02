QT += core
QT -= gui
QT += serialport
QT += network

CONFIG += c++11

TARGET = powermeter
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp
