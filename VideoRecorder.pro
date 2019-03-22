#-------------------------------------------------
#
# Project created by QtCreator 2016-09-01T16:10:47
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VideoRecorder
TEMPLATE = app


SOURCES += src/main.cpp \
    src/mainwindow.cpp \
    src/audio/AACEncoder.cpp \
    src/audio/getaudiothread.cpp

HEADERS  += \
    src/mainwindow.h \
    src/audio/AACEncoder.h \
    src/audio/getaudiothread.h

FORMS    += \
    src/mainwindow.ui

INCLUDEPATH += $$PWD/ffmpeg/include \
               $$PWD/src

LIBS += $$PWD/ffmpeg/lib/avcodec.lib \
        $$PWD/ffmpeg/lib/avdevice.lib \
        $$PWD/ffmpeg/lib/avfilter.lib \
        $$PWD/ffmpeg/lib/avformat.lib \
        $$PWD/ffmpeg/lib/avutil.lib \
        $$PWD/ffmpeg/lib/postproc.lib \
        $$PWD/ffmpeg/lib/swresample.lib \
        $$PWD/ffmpeg/lib/swscale.lib
