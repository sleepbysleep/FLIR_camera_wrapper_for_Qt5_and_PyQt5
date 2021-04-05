# FLIR_camera_wrapper_for_Qt5_and_PyQt5
Qt Camera Class with generating signal of image reception for Spinnaker FLIR camera

Qt Camera Class with generating signal of image reception for Spinnaker FLIR camera. Which is possible by connection Qt Signal object to image event handler of Spinnaker library. It makes speed up frame per second, and reduce down effort of polling routine to receive and provide image through Spinnaker object.
It supports both of Qt and PyQt5, which are named as qt_camera, and pyqt_camera, respectively.

# Method
There are three kinds of ways for triggering the camera hardware. The default is No triggering that no need to provide any trigger, and can access streaming images at any time. The second is the Software Trigger, in which an image is available after 'sendSwTrigger()' was excecuted. The last is the Hardware Trigger. The camera provides an image via Qt Signal, only if electrical PWM is connected to specific input ports of external Hirose Connector.


# How to use
```
qcamera.initSystem()
cam = QCamera()
cam.sendImage.connect(onReceiveImage) cam.enableAcquisition()
...
cam.disableAcquisition() del cam
...
@QtCore.pyqtSlot(np.ndarray, int, str)
def onReceiveImage(image, image_count, camera_serial):
    #doing something related to image processing
    pass
```

# Spinnaker SDK 2.3.0.77
Changes
- Event -> ImageEventHandler
- RegisterEvent() -> RegisterEventHandler()
- UnregisterEvent() -> UnregisterEventHandler()


# Dependencies

$ sudo apt install libavcodec58 libavformat58 \

libswscale5 libswresample3 libavutil56 libusb-1.0-0 \
libpcre2-16-0 libdouble-conversion3 libxcb-xinput0 \
libxcb-xinerama0


# Installation

$ sudo sh install_spinnaker.sh

CAUTION ! 
To avoid conflict with installed qtcreator, unistall spinnake-qt and spinnaker-qt-dev

$ sudo dpkg -r spinnaker-qt
$ sudo dpkg -r spinnaker-qt-dev

# Additional Project Setting for Qtcreator
In qtcreator -> projects -> build -> Build Environment/details,
Set FLIR_GENTL64_CTI to /opt/spinnaker/lib/flir-gentl/FLIR_GenTL.cti

# Screenshots
For Qt,

![alt Qt Camera](https://github.com/sleepbysleep/FLIR_camera_wrapper_for_Qt5_and_PyQt5/blob/main/Screenshot_2021-04-05_13-59-19.png?raw=true)

For PyQt5,

![alt PyQt Camera](https://github.com/sleepbysleep/FLIR_camera_wrapper_for_Qt5_and_PyQt5/blob/main/Screenshot_2021-04-05_14-00-11.png?raw=true)
