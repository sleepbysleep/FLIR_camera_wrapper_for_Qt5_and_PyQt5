#pragma once

#include <iostream>
#include <string>
#include <QObject>

#include <Spinnaker.h>
#include <SpinGenApi/SpinnakerGenApi.h>

#include <opencv2/opencv.hpp>
//#include <boost/circular_buffer.hpp>

namespace FLIRVision {
  int initSystem(void);
  void deinitSystem(void);

  // Trigger type
  enum TriggerType {
      NONE = 0,
      SOFTWARE = 1,
      HARDWARE = 2
  };

  // Acquistion status
  enum AcquisitionStatus {
      IDLE = 0,
      DOING = 1
  };
  
  // This class defines the properties, parameters, and the event itself. Take a
  // moment to notice what parts of the class are mandatory, and what have been
  // added for demonstration purposes. First, any class used to define image events
  // must inherit from ImageEvent. Second, the method signature of OnImageEvent()
  // must also be consistent. Everything else - including the constructor,
  // deconstructor, properties, body of OnImageEvent(), and other functions -
  // is particular to the example.
  class Camera : public QObject, public Spinnaker::ImageEventHandler {
    Q_OBJECT
  public:

    //explicit Camera(QObject *parent = nullptr);
    Camera(std::string const& serial = "");
    ~Camera();

    // This method defines an image event. In it, the image that triggered the
    // event is converted and saved before incrementing the count. Please see
    // Acquisition_CSharp example for more in-depth comments on the acquisition
    // of images.
    void OnImageEvent(Spinnaker::ImagePtr image);
    int getImageCount() const { return this->imageCount; }

    double getMinExposure() const { return this->objectPtr->ExposureTime.GetMin(); }
    double getMaxExposure() const { return this->objectPtr->ExposureTime.GetMax(); }
    double getExposure() const { return this->objectPtr->ExposureTime.GetValue(); }
    void setExposure(double const& exposure_time) const {
      double exposure_trimmed = std::max(this->getMinExposure(), exposure_time);
      exposure_trimmed = std::min(this->getMaxExposure(), exposure_trimmed);
      this->objectPtr->ExposureTime.SetValue(exposure_trimmed);
    }

    double getMinGain() const { return this->objectPtr->Gain.GetMin(); }
    double getMaxGain() const { return this->objectPtr->Gain.GetMax(); }
    double getGain() const { return this->objectPtr->Gain.GetValue(); }
    void setGain(double const& gain_value) const {
      double gain_trimmed = std::max(this->getMinGain(), gain_value);
      gain_trimmed = std::min(this->getMaxGain(), gain_trimmed);
      this->objectPtr->Gain.SetValue(gain_trimmed);
    }

    void setTrigger(TriggerType const trigger_type);
    void sendSwTrigger() {
      if (this->triggerType == TriggerType::SOFTWARE)
	this->objectPtr->TriggerSoftware.Execute();
    }
    void enableAcquisition() {
      if (this->acquisitionStatus == AcquisitionStatus::IDLE) {
	this->objectPtr->BeginAcquisition();
    this->acquisitionStatus = AcquisitionStatus::DOING;
      }
    }
    void disableAcquisition() {
      if (this->acquisitionStatus == AcquisitionStatus::DOING) {
	this->objectPtr->EndAcquisition();
    this->acquisitionStatus = AcquisitionStatus::IDLE;
      }
    }
    std::string const& getDeviceName() const { return this->deviceName; }

  signals:
    void imageArrived(cv::Mat*, int const, char const*);

  protected:

  private:
    TriggerType triggerType = TriggerType::NONE;
    AcquisitionStatus acquisitionStatus = AcquisitionStatus::IDLE;
    Spinnaker::CameraPtr objectPtr = nullptr;
    std::string serialNumber;
    std::string deviceName;
    int imageCount = 0;
    //boost::circular_buffer<cv::Mat> circularImages;
  };
}
