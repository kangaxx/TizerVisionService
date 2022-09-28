#include "KlTemplateHalcon.h"

void halconAction2(int argc, char* in[], vector<HImage*> images, char* out[]) {
    KlTemplateHalcon kl_halcon;
    strncpy_s(out[0], 15, "kl_to_zj", 10);
    strncpy_s(out[1], 2048, kl_halcon.call_halcon_action(*images.at(0)).c_str(), 2047);
    return;
}

string KlTemplateHalcon::call_halcon_action(HImage image)
{
    JsonHelper js;
    js.initial_new_root("guid", "A12345");
    js.append_child("image_path", "d:/images");
    int child_size = 3;
    ptree *children = new ptree[child_size];
    for (int i = 0; i < child_size; ++i) {
        children[i].put("x", i);
        children[i].put("y", i);
    }
    js.append_array("paths", children, child_size);
    return js.get_json_string();
}
