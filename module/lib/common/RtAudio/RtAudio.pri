
INCLUDEPATH += $$PWD/src

INCLUDEPATH += $$PWD/include

win32{

HEADERS += \
    $$PWD/include/asio.h \
    $$PWD/include/asiodrivers.h \
    $$PWD/include/asiodrvr.h \
    $$PWD/include/asiolist.h \
    $$PWD/include/asiosys.h \
    $$PWD/include/dsound.h \
    $$PWD/include/functiondiscoverykeys_devpkey.h \
    $$PWD/include/ginclude.h \
    $$PWD/include/iasiodrv.h \
    $$PWD/include/iasiothiscallresolver.h \
    $$PWD/include/soundcard.h \
    $$PWD/src/RtAudio.h \
    $$PWD/src/rtaudio_c.h

SOURCES += \
    $$PWD/include/asio.cpp \
    $$PWD/include/asiodrivers.cpp \
    $$PWD/include/asiolist.cpp \
    $$PWD/include/iasiothiscallresolver.cpp \
    $$PWD/src/RtAudio.cpp \
    $$PWD/src/rtaudio_c.cpp

    LIBS += -lAdvapi32 -luser32 -lole32 -ldsound
}

unix{

HEADERS += \
    $$PWD/include/asio.h \
    $$PWD/include/asiodrivers.h \
    $$PWD/include/asiodrvr.h \
    $$PWD/include/asiolist.h \
    $$PWD/include/asiosys.h \
    $$PWD/include/dsound.h \
    $$PWD/include/functiondiscoverykeys_devpkey.h \
    $$PWD/include/ginclude.h \
    $$PWD/include/soundcard.h \
    $$PWD/src/RtAudio.h \
    $$PWD/src/rtaudio_c.h

SOURCES += \
    $$PWD/src/RtAudio.cpp \
    $$PWD/src/rtaudio_c.cpp

}

DISTFILES += \
    $$PWD/src/rtaudio.pc.in
