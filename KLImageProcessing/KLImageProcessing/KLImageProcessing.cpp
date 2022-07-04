#include "KLImageProcessing.h"

ImageProcessingBase* get_image_processing(const char* config)
{
    return nullptr;
}

void KLImageProcessing::set_cameras(CameraDevicesBase* camera)
{
    _camera = camera;
}

bool KLImageProcessing::get_processed_image(HalconCpp::HImage& out)
{
    try {
        return true;
    }
    catch (...) {
        return false;
    }
}
