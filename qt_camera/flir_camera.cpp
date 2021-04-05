#include <iostream>
#include <exception>
#include "flir_camera.h"

namespace FLIRVision {
  
  static Spinnaker::SystemPtr spinnakerSystem;
  static Spinnaker::LibraryVersion spinnakerLibraryVersion;
  static Spinnaker::CameraList cameraList;

  int initSystem(void)
  {
    /*
      if (cameraSystem != nullptr) {
      throw std::runtime_error("Camera system is already allocated.");   // throws
      return -1;
      }
    */

    // Retrieve singleton reference to system object
    spinnakerSystem = Spinnaker::System::GetInstance();

    // Get current library version
    spinnakerLibraryVersion = spinnakerSystem->GetLibraryVersion();
    std::cout << "Spinnaker library version: "
              << spinnakerLibraryVersion.major << "."
              << spinnakerLibraryVersion.minor << "."
              << spinnakerLibraryVersion.type << "."
              << spinnakerLibraryVersion.build << std::endl << std::endl;

    // Retrieve list of cameras from the system
    cameraList.Clear();
    cameraList = spinnakerSystem->GetCameras();
    std::cout << "Number of cameras detected: " << cameraList.GetSize() << std::endl << std::endl;

    if (cameraList.GetSize() <= 0) {
      cameraList.Clear();
      //delete cameraList;
      spinnakerSystem->ReleaseInstance();
      //delete spinnakerSystem;
      //throw std::runtime_error("No camera is found.");
    }

    return static_cast<int>(cameraList.GetSize());
  }

  void deinitSystem(void)
  {
    cameraList.Clear();
    spinnakerSystem->ReleaseInstance();
  }

  Camera::Camera(std::string const& serial)
    : objectPtr(nullptr), serialNumber(""), imageCount(0)
  {
    if (!spinnakerSystem.IsValid()) throw std::runtime_error("No spinnaker system is found.");
    if (cameraList.GetSize() <= 0) throw std::runtime_error("No camera is found.");

    if (serial.empty()) this->objectPtr = cameraList.GetByIndex(0);
    else this->objectPtr = cameraList.GetBySerial(serial);

    // print the device info
    Spinnaker::GenApi::INodeMap& nodeMap = this->objectPtr->GetTLDeviceNodeMap();
    Spinnaker::GenApi::FeatureList_t features;
    const Spinnaker::GenApi::CCategoryPtr category = nodeMap.GetNode("DeviceInformation");
    if (IsAvailable(category) && IsReadable(category)) {
        category->GetFeatures(features);

        for (auto it = features.begin(); it != features.end(); ++it) {
            const Spinnaker::GenApi::CNodePtr pfeatureNode = *it;
            Spinnaker::GenApi::CValuePtr pValue = static_cast<Spinnaker::GenApi::CValuePtr>(pfeatureNode);

            std::cout << pfeatureNode->GetName() << " : ";
            std::cout << (IsReadable(pValue) ? pValue->ToString() : "Node not readable");
            std::cout << std::endl;

            if (pfeatureNode->GetName() == "DeviceModelName") {
                this->deviceName = std::string(pValue->ToString().c_str());
            }
        }
    } else {
        std::cout << "Device control information not available." << std::endl;
    }

    if (!this->objectPtr->IsValid()) throw std::runtime_error("Invalid camera was selected.");
    if (this->objectPtr->IsInitialized()) throw std::runtime_error("Selected camera has already initialized.");
    this->objectPtr->Init();

    //this->objectPtr->RegisterEvent(dynamic_cast<Spinnaker::Event&>(*this));
    this->objectPtr->RegisterEventHandler(dynamic_cast<Spinnaker::ImageEventHandler&>(*this));
    this->objectPtr->ExposureAuto.SetValue(Spinnaker::ExposureAuto_Off);
    this->objectPtr->GainAuto.SetValue(Spinnaker::GainAuto_Off);
    this->objectPtr->BlackLevel.SetValue(0);
    this->objectPtr->OffsetX.SetValue(0);
    this->objectPtr->OffsetY.SetValue(0);
    //this->objectPtr->Width.SetValue(720);
    //this->objectPtr->Height.SetValue(540);
    this->setTrigger(this->triggerType);
    this->objectPtr->AcquisitionMode.SetValue(Spinnaker::AcquisitionMode_Continuous);

    this->serialNumber = this->objectPtr->TLDevice.DeviceSerialNumber.GetValue();
    this->imageCount = 0;

    //this->objectPtr->Width.GetValue();
    //this->objectPtr->Height.GetValue();
    //this->objectPtr->ExposureTime.GetValue();
    //this->objectPtr->PixelFormat.GetValue();
    std::cout <<"Initialized Spinnaker compatible camera ("
	      << this->objectPtr->Width.GetValue() << "x" << this->objectPtr->Height.GetValue() << "@"
	      << this->objectPtr->PixelFormat.GetCurrentEntry()->GetSymbolic() << ")" << std::endl;

    //Spinnaker::PixelFormatEnums this->pixelFormat = this->objectPtr->PixelFormat.GetValue();
  }

  Camera::~Camera()
  {
    if (this->objectPtr and this->objectPtr->IsValid()) {
      //this->objectPtr->EndAcquisition();
      this->disableAcquisition();
      this->objectPtr->AcquisitionMode.SetValue(Spinnaker::AcquisitionMode_Continuous);
      this->objectPtr->GainAuto.SetValue(Spinnaker::GainAuto_Continuous);
      this->objectPtr->ExposureAuto.SetValue(Spinnaker::ExposureAuto_Continuous);
      this->objectPtr->UnregisterEventHandler(dynamic_cast<Spinnaker::ImageEventHandler&>(*this));
      this->objectPtr->DeInit();
      this->objectPtr = nullptr;
    }
  }

  void Camera::OnImageEvent(Spinnaker::ImagePtr image)
  {
    if (!image->IsIncomplete()) {
      std::size_t XPadding = image->GetXPadding();
      std::size_t YPadding = image->GetYPadding();
      std::size_t rowsize = image->GetWidth();
      std::size_t colsize = image->GetHeight();

      Spinnaker::ImagePtr image_converted = image->Convert(Spinnaker::PixelFormat_RGB8,
							   Spinnaker::NEAREST_NEIGHBOR); // Spinnaker::HQ_LINEAR

      //image data contains padding. When allocating Mat container size, you need to account for the X,Y image data padding.
      cv::Mat* cvImage = new cv::Mat(colsize + XPadding, rowsize + YPadding, CV_8UC3, image_converted->GetData(), image_converted->GetStride());

      ++this->imageCount;
      emit this->imageArrived(cvImage, this->imageCount, this->serialNumber.c_str());
    } else {
      std::cout << "Image incomplete with image status" << image->GetImageStatus() << std::endl;
    }
  }

  void Camera::setTrigger(TriggerType const trigger_type)
  {
    this->triggerType = trigger_type;
    /* Select trigger source
     * The trigger source must be set to hardware or software while trigger
     * mode is off. */
    this->objectPtr->TriggerMode.SetValue(Spinnaker::TriggerMode_Off);
    if (this->objectPtr->TriggerSource.GetAccessMode() != Spinnaker::GenApi::EAccessMode::RW)
      throw std::runtime_error("Unable to get trigger source (node retrieval). Aborting...");

    if (trigger_type == TriggerType::SOFTWARE) {
      this->objectPtr->TriggerSource.SetValue(Spinnaker::TriggerSource_Software);
    } else if (trigger_type == TriggerType::HARDWARE) {
      this->objectPtr->TriggerSource.SetValue(Spinnaker::TriggerSource_Line0);
    } else if (trigger_type == TriggerType::NONE) {
      return;
    }

    /* Turn trigger mode on
       Once the appropriate trigger source has been set, turn trigger mode
       on in order to retrieve images using the trigger.    */
    this->objectPtr->TriggerMode.SetValue(Spinnaker::TriggerMode_On);
    //print('Trigger mode turned back on...')
  }
}
