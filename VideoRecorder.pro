#-------------------------------------------------
#
# Project created by QtCreator 2015-04-01T17:15:51
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VideoRecorder
TEMPLATE = app

SOURCES += src/main.cpp\
        src/mainwindow.cpp \
    src/video/savevideofile.cpp \
    src/video/screenrecorder.cpp \
    src/widget/selectrect.cpp \
    src/widget/pushpoint.cpp \
    src/video/getvideothread.cpp

HEADERS  += src/mainwindow.h \
    src/video/savevideofile.h \
    src/video/screenrecorder.h \
    src/widget/selectrect.h \
    src/widget/pushpoint.h \
    src/video/getvideothread.h

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
