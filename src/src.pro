TEMPLATE = lib
QT -= gui
QT *= network
TARGET = qfiber
VERSION = 1.0.0

include(../conf.pri)

CONFIG += create_prl staticlib

include(src.pri)
