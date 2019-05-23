#include "TGAImage.h"

#include <iostream>
#include <string.h>


TGAImage::TGAImage() : data(nullptr),width(0),height(0),bytespp(0){
}

TGAImage::TGAImage(int w, int h, int bpp) : width(w), height(h), bytespp(bpp){
	unsigned long nbytes = w * h * bpp;
	data = new unsigned char[nbytes];
	memset(data, 0, nbytes);
}

TGAImage::TGAImage(const TGAImage & img){
	width = img.width;
	height = img.height;
	bytespp = img.bytespp;
	unsigned long nbytes = width * height * bytespp;
	data = new unsigned char[nbytes];
	memcpy(data, img.data, nbytes);
}

TGAImage::~TGAImage()
{
	if (data) delete[] data;
}

bool TGAImage::readTgaFile(const char * filename)
{
	std::ifstream infile;
	infile.exceptions(std::ifstream::badbit | std::ifstream::failbit);

	try{
		infile.open(filename, std::ios::binary);
		TGAHeader tHeader;
		infile.read((char*)&tHeader, sizeof(TGAHeader));
		if (!infile.good())
		{
			std::cerr << "an error occured while reading the header!" << std::endl;
			return false;
		}

		width = tHeader.imageWidth;
		height = tHeader.imageHeight;
		bytespp = tHeader.perPixelBits >> 3;
		if (width < 0 || height < 0 || (bytespp != GRAYSCALE && bytespp != RGB && bytespp != RGBA))
		{
			infile.close();
			std::cerr << "bad bpp " << std::endl;
			return false;
		}

		unsigned long nbytes = width * height * bytespp;
		data = new unsigned char[nbytes];
		///< 0 no image data is present
		///< 1 uncompressed color - mapped image
		///< 2 uncompressed true - color image
		///< 3 uncompressed black - and-white(grayscale) image
		///< 9 run - length encoded color - mapped image
		///< 10 run - length encoded true - color image
		///< 11 run - length encoded black - and-white(grayscale) image
		if (3 == tHeader.imageType || 2 == tHeader.imageType)
		{
			infile.read((char*)data, nbytes);
			if (!infile.good())
			{
				infile.close();
				std::cerr << "read image data error!" << std::endl;
				return false;
			}
		}
		else if(10 == tHeader.imageType || 11 == tHeader.imageType){
			loadRleData(infile);
			if (!infile.good())
			{
				infile.close();
				std::cerr << "read rle data error!" << std::endl;
				return false;
			}
		}
		else {
			infile.close();
			std::cerr << "unknown file format!" << std::endl;
			return false;
		}

		///< bits 3-0 give the alpha channel depth, bits 5-4 give direction
		if (tHeader.descriptor & 0x20)
		{
			///< 数据是竖向扫描的
			flipVertically();
		}
		if (tHeader.descriptor & 0x10)
		{
			///< 数据是横向扫描的
			flipHorizontally();
		}
	}
	catch (std::ifstream::failure e)
	{
		infile.close();
		std::cout << "open file " << filename << " failed!" << std::endl;
		return false;
	}
	
	infile.close();

	return true;
}


bool TGAImage::loadRleData(std::ifstream & in)
{
	return false;
}

bool TGAImage::unloadRleData(std::ofstream & out)
{
	return false;
}

bool TGAImage::writeTgaFile(const char * filename, bool rle)
{
	return false;
}

bool TGAImage::flipHorizontally()
{
	return false;
}

bool TGAImage::flipVertically()
{
	return false;
}





