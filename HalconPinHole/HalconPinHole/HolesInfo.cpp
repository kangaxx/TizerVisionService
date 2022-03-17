#include "HolesInfo.h"

HImage HolesInfo::get_image()
{
    return image_;
}

int HolesInfo::get_x()
{
    return x_;
}

int HolesInfo::get_y()
{
    return y_;
}

int HolesInfo::get_x_global()
{
    return x_global_;
}

int HolesInfo::get_y_global()
{
    return y_global_;
}

float HolesInfo::get_area()
{
    return area_;
}
