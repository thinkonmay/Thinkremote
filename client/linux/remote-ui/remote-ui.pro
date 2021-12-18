QT       += core gui multimedia multimediawidgets widgets network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
QT_CONFIG -= no-pkg-config
CONFIG += C:/gstreamer/1.0/msvc_x86_64/bin link_pkgconfig
PKGCONFIG += gobject-2.0 glib-2.0 gio-2.0 gstreamer-sdp-1.0 gstreamer-1.0 json-glib-1.0 libsoup-2.4 gstreamer-webrtc-1.0 libsoup-2.4 gstreamer-webrtc-1.0 gstreamer-controller-1.0 gstreamer-video-1.0



win32:LIBS += -LD:\Thinkmay-client\bin -lremote-app -lshared-items

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
#win32:LIBS += -L"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.19041.0\um\x86" -lUser32

SOURCES += \
    main.cpp \
    remoteui.cpp

HEADERS += \
    GEventLogger.h \
    remoteui.h


INCLUDEPATH += \
    ../remote-app/include \
    ../shared-items/include



FORMS += \
    remoteui.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
