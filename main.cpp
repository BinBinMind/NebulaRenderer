#include "TGAImage.h"

#define IMG_WIDTH 256
#define IMG_HEIGHT 256

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

///< Bresenham's line 
void drawLine(int x0, int y0, int x1, int y1, TGAImage* img, TGAColor color)
{
	///< Method 1
	///< 最简单的绘制方法，使用常量步长
	//for (float i = 0.0; i < 1.0; i+=0.001)
	//{
	//	int x = x0 + i * (x1 - x0);
	//	int y = x0 + i * (y1 - y0);
	//	img->set(x, y, color);
	//}

	///< Method 2
	///< 步长采用像素尺寸的增量
	//for (int x = x0; x < x1; x++)
	//{
	//	float t = (x - x0) / (float)(x1 - x0);
	//	int y = y0 * (1 - t) + y1 * t;
	//	img->set(x, y, color);
	//}

	///< Method 3
	///< 考虑输入坐标的大小，根据x和y的增量大小选择步长
	bool transpose = false;
	if (std::abs(x1 - x0) < std::abs(y1 - y0))
	{
		transpose = true;
		std::swap(x0, y0);
		std::swap(x1, y1);
	}
	if (x1 < x0)
	{
		std::swap(x1, x0);
		std::swap(y1, y0);
	}
	for (int x = x0; x < x1; x++)
	{
		float t = (x - x0) / (float)(x1 - x0);
		int y = y0 * (1 - t) + y1 * t;
		if (transpose)
		{
			img->set(y, x, color);
		}
		else
		{
			img->set(x, y, color);
		}
	}

}

int main(void)
{
	TGAImage* testImage = new TGAImage(IMG_WIDTH, IMG_HEIGHT, TGAImage::RGB);
	for (int x = 0; x < IMG_WIDTH; x++)
	{
		for (int y = 0; y < IMG_HEIGHT; y++)
		{
			testImage->set(x, y, white);
		}
	}

	drawLine(50, 50, 100, 100, testImage, red);
	drawLine(50, 50, 100, 250, testImage, red);
	drawLine(200, 200, 50, 100, testImage, red);
	testImage->flipVertically();
	testImage->writeTgaFile("out.tga",true);

	return 0;
}