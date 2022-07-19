#include "JmjImageProcessing.h"

ImageProcessingBase* get_image_processing(const char* config)
{
    g_image_processing = new JmjImageProcessing();
    return nullptr;
}

void JmjImageProcessing::set_cameras(CameraDevicesBase* camera)
{
    return;
}

bool JmjImageProcessing::get_processed_image(HalconCpp::HImage& out)
{
    return false;
}

void JmjImageProcessing::do_image_process(HalconCpp::HImage& result)
{
    return;
}
