import sys
import enum
import time
import math
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from PyQt5 import uic
import numpy as np
import cv2
import PySpin
import flir_camera as FLIRVision
from numpy2qimage import *
from utils import dumpException

class MainWindow(QMainWindow):

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        uic.loadUi('mainwindow.ui', self)
        self.imageCanvas.setStyleSheet('QLabel { background-color : black; color : blue; }')
        self.statusBar().showMessage("Hello world.", 3000);
        self.elapsedTime = QElapsedTimer()
        self.fpsCount = 0
        self.fpsSum = 0.0

        try:
            FLIRVision.initSystem()
            self.visionCamera = FLIRVision.Camera()
            self.setWindowTitle('PyQt Camera - ' + self.visionCamera.getDeviceName())

            # self.visionCamera.setTrigger(TriggerType.SOFTWARE)
            self.visionCamera.imageArrived.connect(self.onReceiveImage)

            minValue = math.floor(self.visionCamera.getMinExposure())
            #maxValue = math.ceil(self.visionCamera.getMaxExposure())
            maxValue = 100000

            value = round(self.visionCamera.getExposure())
            self.exposureSlider.setRange(minValue, maxValue)
            self.exposureSlider.setValue(value)
            self.exposureLabel.setText(str(value))
            self.exposureSlider.valueChanged.connect(self.onChangedExposure)
    
            minValue = math.floor(self.visionCamera.getMinGain())
            maxValue = math.ceil(self.visionCamera.getMaxGain())
            value = round(self.visionCamera.getGain())
            self.gainSlider.setRange(minValue, maxValue)
            self.gainSlider.setValue(value)
            self.gainLabel.setText(str(value))
            self.gainSlider.valueChanged.connect(self.onChangedGain)

            self.visionCamera.enableAcquisition()
            self.elapsedTime.start()
        except Exception as e:
            dumpException(e)

    def closeEvent(self, event):
        try:
            self.visionCamera.disableAcquisition()
            del self.visionCamera
            FLIRVision.deinitSystem()
            #def __del__(self):
        except Exception as e:
            dumpException(e)

        super().closeEvent(event)

    @pyqtSlot(np.ndarray, int, str)
    def onReceiveImage(self, image, count, serial):
        #print(serial, count)
        qimage = QImage(image.data, image.shape[1], image.shape[0], QImage.Format_RGB888)
        # qimage = QImage(image.data, image.cols, image.rows, image.step, QImage.Format_RGB888) #.rgbSwapped();
        self.imageCanvas.setPixmap(
            QPixmap.fromImage(
                qimage.scaled(
                    self.imageCanvas.size(),
                    Qt.KeepAspectRatio,
                    Qt.SmoothTransformation
                )
            )
        )

        elapsed = self.elapsedTime.elapsed()
        self.fpsSum += 1000.0/(elapsed+1.0)
        self.fpsCount += 1
        if self.fpsCount >= 100:
            self.statusBar().showMessage(f'{self.fpsSum/self.fpsCount:3.0f} [fps]')
            self.fpsSum = 0.0
            self.fpsCount = 0
        self.elapsedTime.restart()

    def onChangedExposure(self, value):
        self.exposureSlider.setValue(value)
        self.exposureLabel.setText(f'{value:08d}')

        try:
            self.visionCamera.setExposure(value)
        except Exception as e:
            dumpException(e)

    def onChangedGain(self, value):
        self.gainSlider.setValue(value)
        self.gainLabel.setText(f'{value:08d}')

        try:
            self.visionCamera.setGain(value)
        except Exception as e:
            dumpException(e)

if __name__ == '__main__':
    app = QApplication(sys.argv)
    # #print(QStyleFactory.keys())
    app.setStyle('Fusion')
    wnd = MainWindow()
    wnd.setGeometry(
        QStyle.alignedRect(
            Qt.LeftToRight,
            Qt.AlignCenter,
            wnd.size(),
            app.desktop().availableGeometry()
        )
    )
    wnd.show()
    sys.exit(app.exec_())
