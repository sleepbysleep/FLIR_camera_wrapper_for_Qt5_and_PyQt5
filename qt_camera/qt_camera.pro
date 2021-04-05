QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 console

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    flir_camera.cpp

HEADERS += \
    mainwindow.h \
    flir_camera.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

LIBS += -L/usr/lib/ -L/opt/spinnaker/lib -lSpinnaker -lSpinnaker_C
INCLUDEPATH += /opt/spinnaker/include

CONFIG += link_pkgconfig
#PKGCONFIG += libusb-1.0
PKGCONFIG += opencv4

#LIBS += -lboost_system

#QMAKE_CFLAGS_RELEASE += -fopenmp
#QMAKE_CXXFLAGS_RELEASE += -fopenmp