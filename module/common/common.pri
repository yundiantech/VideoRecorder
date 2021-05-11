INCLUDEPATH += $$PWD/src
INCLUDEPATH += $$PWD/src/Audio
INCLUDEPATH += $$PWD/src/Video

SOURCES += \
        $$PWD/src/Audio/Mix/PcmMix.cpp \
        $$PWD/src/MoudleConfig.cpp \
        $$PWD/src/Audio/AudioFrame/AACFrame.cpp \
        $$PWD/src/Audio/AudioFrame/PCMFrame.cpp \
        $$PWD/src/Mutex/Cond.cpp \
        $$PWD/src/Mutex/Mutex.cpp \
        $$PWD/src/NALU/nalu.cpp \
        $$PWD/src/Video/VideoFrame/VideoRawFrame.cpp \
        $$PWD/src/Video/VideoFrame/VideoEncodedFrame.cpp \
        $$PWD/src/LogWriter/LogWriter.cpp

HEADERS += \
        $$PWD/src/Audio/Mix/PcmMix.h \
        $$PWD/src/MoudleConfig.h \
        $$PWD/src/Audio/AudioFrame/AACFrame.h \
        $$PWD/src/Audio/AudioFrame/PCMFrame.h \
        $$PWD/src/Mutex/Cond.h \
        $$PWD/src/Mutex/Mutex.h \
        $$PWD/src/NALU/h264.h \
        $$PWD/src/NALU/h265.h \
        $$PWD/src/NALU/nalu.h \
        $$PWD/src/Video/VideoFrame/VideoRawFrame.h \
        $$PWD/src/Video/VideoFrame/VideoEncodedFrame.h \
        $$PWD/src/LogWriter/LogWriter.h

#### lib ### Begin
#    include($$PWD/../lib/lib.pri)
#### lib ### End
