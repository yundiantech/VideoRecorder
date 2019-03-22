#-------------------------------------------------
#
# Project created by QtCreator 2015-01-06T22:20:29
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VideoRecorder
TEMPLATE = app


SOURCES += src/main.cpp\
        src/mainwindow.cpp \
    src/savevideofile.cpp \
    src/screenrecorder.cpp

HEADERS  += src/mainwindow.h \
    src/savevideofile.h \
    src/screenrecorder.h

FORMS    += src/mainwindow.ui

INCLUDEPATH += $$PWD/lib/ffmpeg/include \
               $$PWD/lib/SDL2/include \
               $$PWD/src

LIBS += $$PWD/lib/ffmpeg/lib/avcodec.lib \
        $$PWD/lib/ffmpeg/lib/avdevice.lib \
        $$PWD/lib/ffmpeg/lib/avfilter.lib \
        $$PWD/lib/ffmpeg/lib/avformat.lib \
        $$PWD/lib/ffmpeg/lib/avutil.lib \
        $$PWD/lib/ffmpeg/lib/postproc.lib \
        $$PWD/lib/ffmpeg/lib/swresample.lib \
        $$PWD/lib/ffmpeg/lib/swscale.lib \
        $$PWD/lib/SDL2/lib/x86/SDL2.lib

LIBS += -lwinmm
LIBS += -lws2_32
