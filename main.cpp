#include "TGAImage.h"


const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

int main(void)
{
	TGAImage* testImage = new TGAImage(256, 256, TGAImage::RGBA);
	for (int i = 0; i < 256; i++)
	{
		for (int j = 0; j < 256; j++)
		{
			testImage->set(i, j, red);
		}
	}
	//testImage->flipVertically();
	testImage->writeTgaFile("out.tga",false);
	return 0;
}