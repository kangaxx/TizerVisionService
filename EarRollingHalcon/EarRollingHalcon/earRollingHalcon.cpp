#include "earRollingHalcon.h"

char** halconAction(int argc, char* in[], HImage image, char** out)
{
	**out = 'h';
	image.WriteImage("jpg", 0, "d:/grabs/libImageTest");
	return out;
}
