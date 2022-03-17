#include "PinHoleResult.h"

int PinHoleResult::get_hole_num()
{
    return hole_num_;
}

vector<HolesInfo> PinHoleResult::get_holes_list()
{
    return holes_info_list;
}

void PinHoleResult::add_hole(HolesInfo hole_info)
{
    holes_info_list.push_back(hole_info);
}
