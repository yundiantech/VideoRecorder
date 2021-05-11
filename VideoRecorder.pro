#-------------------------------------------------
#
# Project created by QtCreator 2020-02-06T20:41:27
#
#-------------------------------------------------

QT       += core gui network websockets multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app

TARGET = VideoRecorder

UI_DIR  = obj/Gui
MOC_DIR = obj/Moc
OBJECTS_DIR = obj/Obj

#将输出文件直接放到源码目录下的bin目录下，将dll都放在了此目录中，用以解决运行后找不到dll的问
#DESTDIR=$$PWD/bin/
win32{
    contains(QT_ARCH, i386) {
        message("32-bit")
        DESTDIR = $${PWD}/bin/win32
    } else {
        message("64-bit")
        DESTDIR = $${PWD}/bin/win64
    }
}

### lib ### Begin
    include($$PWD/module/lib/lib.pri)
### lib ### End

### common ### Begin
    include($$PWD/module/common/common.pri)
### common ### End

#包含可拖动窗体的代码
include(module/DragAbleWidget/DragAbleWidget.pri)

#包含 录屏模块
include(module/ScreenRecorder/ScreenRecorder.pri)

SOURCES +=  \
    src/AppConfig.cpp \
    src/CaptureTask/CapturePictureWidget.cpp \
    src/CaptureTask/CaptureTaskManager.cpp \
    src/CaptureTask/CaptureWindowWidget.cpp \
    src/CaptureTask/SelectAreaWidget/SelectAreaWidget.cpp \
    src/CaptureTask/SelectAreaWidget/ShowAreaWdiget.cpp \
    src/CaptureTask/SelectProgram/SelectRunningProgramDialog.cpp \
    src/CaptureTask/SelectProgram/ShowProgramPictureWidget.cpp \
    src/MainWindow.cpp \
    src/Widget/CustomWidget/MyCustomerWidget.cpp \
    src/Widget/CustomWidget/flowlayout.cpp \
    src/main.cpp\
    src/Base64/Base64.cpp \
    src/DeviceTest/AudioInfo.cpp \
    src/DeviceTest/DeviceSettingDialog.cpp \
    src/Widget/mymessagebox_withTitle.cpp \
    src/Base/FunctionTransfer.cpp \
    src/Widget/ShowRedRectWidget.cpp \
    src/Camera/ShowCameraWidget.cpp \
    src/Widget/Video/ShowVideoWidget.cpp

HEADERS  += \
    src/AppConfig.h \
    src/Base64/Base64.h \
    src/CaptureTask/CapturePictureWidget.h \
    src/CaptureTask/CaptureTaskManager.h \
    src/CaptureTask/CaptureWindowWidget.h \
    src/CaptureTask/SelectAreaWidget/SelectAreaWidget.h \
    src/CaptureTask/SelectAreaWidget/ShowAreaWdiget.h \
    src/CaptureTask/SelectProgram/SelectRunningProgramDialog.h \
    src/CaptureTask/SelectProgram/ShowProgramPictureWidget.h \
    src/DeviceTest/AudioInfo.h \
    src/DeviceTest/DeviceSettingDialog.h \
    src/MainWindow.h \
    src/Widget/CustomWidget/MyCustomerWidget.h \
    src/Widget/CustomWidget/flowlayout.h \
    src/Widget/mymessagebox_withTitle.h \
    src/Base/FunctionTransfer.h \
    src/Widget/ShowRedRectWidget.h \
    src/Camera/ShowCameraWidget.h \
    src/Widget/Video/ShowVideoWidget.h

INCLUDEPATH += $$PWD/src \
               $$PWD/src/widget/common

FORMS    += \
    src/CaptureTask/CapturePictureWidget.ui \
    src/CaptureTask/CaptureTaskManager.ui \
    src/CaptureTask/CaptureWindowWidget.ui \
    src/CaptureTask/SelectAreaWidget/SelectAreaWidget.ui \
    src/CaptureTask/SelectAreaWidget/ShowAreaWdiget.ui \
    src/CaptureTask/SelectProgram/SelectRunningProgramDialog.ui \
    src/CaptureTask/SelectProgram/ShowProgramPictureWidget.ui \
    src/MainWindow.ui \
    src/DeviceTest/DeviceSettingDialog.ui \
    src/Widget/CustomWidget/MyCustomerWidget.ui \
    src/Widget/mymessagebox_withTitle.ui \
    src/Widget/ShowRedRectWidget.ui \
    src/Camera/ShowCameraWidget.ui \
    src/widget/video/ShowVideoWidget.ui

RESOURCES += \
    res/resources.qrc

win32:RC_FILE=$$PWD/res/main.rc
macx:ICON = $$PWD/res/logo.icns
