INCLUDEPATH += $$PWD/src
INCLUDEPATH += $$PWD/src/Media/Audio

SOURCES += \
        $$PWD/src/EventHandle/VideoRecorderEventHandle.cpp \
        $$PWD/src/Media/Image/ImageReader.cpp \
        $$PWD/src/Media/Video/CaptureWindowThread.cpp \
        $$PWD/src/Media/Audio/AudioEncoder.cpp \
        $$PWD/src/Media/Audio/GetAudioThread.cpp \
        $$PWD/src/Media/MediaManager.cpp \
        $$PWD/src/Media/Video/GetVideoThread.cpp \
        $$PWD/src/Media/Video/VideoEncoder.cpp \
        $$PWD/src/Media/Video/VideoFileWriter.cpp \
        $$PWD/src/Media/Image/yuv420p.cpp

HEADERS += \
        $$PWD/src/EventHandle/VideoRecorderEventHandle.h \
        $$PWD/src/Media/Image/ImageReader.h \
        $$PWD/src/Media/Video/CaptureWindowThread.h \
        $$PWD/src/Media/MediaManager.h \
        $$PWD/src/Media/Video/VideoFileInfoTypes.h \
        $$PWD/src/Media/Video/VideoFileWriter.h \
        $$PWD/src/Media/Image/yuv420p.h \
        $$PWD/src/Media/Audio/AudioEncoder.h \
        $$PWD/src/Media/Audio/GetAudioThread.h \
        $$PWD/src/Media/Video/GetVideoThread.h \
        $$PWD/src/Media/Video/VideoEncoder.h

#### lib ### Begin
#    include($$PWD/../lib/lib.pri)
#### lib ### End
