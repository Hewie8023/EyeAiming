#-------------------------------------------------
#
# Project created by QtCreator 2018-11-25T21:15:54
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = EyeAiming
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    main.cpp \
    svd.cpp \
    CCircle_detector.cpp \
    PupilTracker.cpp \
    mainwindow.cpp \
    cdocalibration.cpp \
    showcal.cpp

HEADERS += \
    svd.h \
    CCircle_detector.h \
    PupilTracker.h \
    mainwindow.h \
    cdocalibration.h \
    showcal.h \
    util.h

FORMS += \
    mainwindow.ui \
    showcal.ui \
    showcal.ui


INCLUDEPATH += D:\\opencv\\build\\include
INCLUDEPATH += D:\\opencv\\build\\include\\opencv
INCLUDEPATH += D:\\opencv\\build\\include\\opencv2


CONFIG(release,debug|release)
{
    LIBS += D:\\opencv\\build\\x64\\vc14\\lib\\opencv_*.lib
}

INCLUDEPATH += D:\\tbb\\tbb2018_20180822oss\\include

CONFIG(release,debug|release)
{
    LIBS += D:\\tbb\\tbb2018_20180822oss\\lib\\intel64\\vc14\\tbb*.lib
}
