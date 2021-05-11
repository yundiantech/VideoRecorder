INCLUDEPATH += $$PWD

####  Windows ### - Begin
win32{

    #DEFINES += __STDC_LIMIT_MACROS
#    DEFINES += NO_CRYPTO

    QMAKE_LFLAGS_DEBUG      = /DEBUG /NODEFAULTLIB:libc.lib /NODEFAULTLIB:libcmt.lib
    QMAKE_LFLAGS_RELEASE    = /RELEASE /NODEFAULTLIB:libc.lib /NODEFAULTLIB:libcmt.lib

    contains(QT_ARCH, i386) {
        message("32-bit")

        INCLUDEPATH += $$PWD/win32/ffmpeg/include   \
                       $$PWD/win32/SDL2/include \
                       $$PWD/win32/openssl/include  \
                       $$PWD/win32/zlib/include \
                       $$PWD/win32/libyuv/include

#        LIBS += $$PWD/win32/openssl/lib/libeay32.lib
#        LIBS += $$PWD/win32/openssl/lib/ssleay32.lib
#        LIBS += $$PWD/win32/zlib/lib/zlibstat.lib

#        LIBS += $$PWD/win32/ffmpeg/lib/avcodec.lib \
#                $$PWD/win32/ffmpeg/lib/avdevice.lib \
#                $$PWD/win32/ffmpeg/lib/avfilter.lib \
#                $$PWD/win32/ffmpeg/lib/avformat.lib \
#                $$PWD/win32/ffmpeg/lib/avutil.lib \
#                $$PWD/win32/ffmpeg/lib/postproc.lib \
#                $$PWD/win32/ffmpeg/lib/swresample.lib \
#                $$PWD/win32/ffmpeg/lib/swscale.lib

        LIBS += -L$$PWD/win32/ffmpeg/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale
        LIBS += -L$$PWD/win32/SDL2/lib -lSDL2
        LIBS += -L$$PWD/win32/libyuv/lib -lyuv

    } else {
        message("64-bit")
        INCLUDEPATH += $$PWD/lib/win64/ffmpeg/include \
                       $$PWD/lib/win64/SDL2/include \
                       $$PWD/src

        LIBS += -L$$PWD/lib/win64/ffmpeg/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale
        LIBS += -L$$PWD/lib/win64/SDL2/lib -lSDL2
    }

#    LIBS += WS2_32.lib AdvAPI32.lib winmm.lib User32.lib GDI32.lib
    LIBS += -lWS2_32 -lUser32 -lGDI32 -lAdvAPI32 -lwinmm -lStrmiids -loleaut32

}
####  Windows  ### - End

### librtmp ### Begin
    include($$PWD/common/librtmp/librtmp.pri)
### librtmp ### End

### RtAudio ### Begin
    include($$PWD/common/RtAudio/RtAudio.pri)
### RtAudio ### End
