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
	if (data != nullptr) {
		delete[] data;
	}

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

bool TGAImage::writeTgaFile(const char * filename, bool rle)
{
	unsigned char developer_area_ref[4] = { 0, 0, 0, 0 };
	unsigned char extension_area_ref[4] = { 0, 0, 0, 0 };
	unsigned char footer[18] = { 'T','R','U','E','V','I','S','I','O','N','-','X','F','I','L','E','.','\0' };

	std::ofstream out;
	out.open(filename, std::ios::binary);
	if (!out.is_open()) {
		std::cerr << "can't open file" << filename << std::endl;
		out.close();
		return false;
	}

	TGAHeader header;
	memset((void*)&header, 0, sizeof(header));
	header.perPixelBits = bytespp << 3;
	header.imageWidth = width;
	header.imageHeight = height;
	header.imageType = (bytespp == GRAYSCALE ? (rle ? 11 : 3) : (rle ? 10 : 2));
	header.descriptor = 0x20;
	out.write((char*)&header, sizeof(header));
	if (!out.good())
	{
		std::cerr << "can't dump the tga file" << filename << std::endl;
		out.close();
		return false;
	}

	if (!rle) {
		out.write((char *)data, width*height*bytespp);
		if (!out.good()) {
			std::cerr << "can't unload raw data\n";
			out.close();
			return false;
		}
	}
	else {
		if (!unloadRleData(out)) {
			out.close();
			std::cerr << "can't unload rle data\n";
			return false;
		}
	}
	out.write((char *)developer_area_ref, sizeof(developer_area_ref));
	if (!out.good()) {
		std::cerr << "can't dump the tga file\n";
		out.close();
		return false;
	}
	out.write((char *)extension_area_ref, sizeof(extension_area_ref));
	if (!out.good()) {
		std::cerr << "can't dump the tga file\n";
		out.close();
		return false;
	}
	out.write((char *)footer, sizeof(footer));
	if (!out.good()) {
		std::cerr << "can't dump the tga file\n";
		out.close();
		return false;
	}
	
	out.close();
	return true;
}

bool TGAImage::loadRleData(std::ifstream & in)
{
	unsigned long pixelCount = width * height;
	unsigned long currentPixel = 0;
	unsigned long currentByte = 0;
	TGAColor pixel;
	do {
		///chunkHeader是一个字节大小，8bits中1bit表示是否为rle,7bits表示该chunk的pixel数量
		int chunkHeader = in.get();
		if (!in.good())
		{
			in.close();
			std::cerr << "An error occured while reading the data!" << std::endl;
			return false;
		}

		///< 最高位为0，非rle packet，每读一个packet，写到data中一个
		if (chunkHeader < 128)
		{
			chunkHeader++;
			for (int i = 0; i < chunkHeader; i++)
			{
				in.read((char*)pixel.raw, bytespp);
				for (int s = 0; s < bytespp; s++)
				{
					data[currentByte++] = pixel.raw[s];
				}
				currentPixel++;

				if (currentPixel > pixelCount)
				{
					std::cerr << " too many pixels read!" << std::endl;
					return false;
				}
			}
		}
		else///< 最高位为1，为RLE Packet，每读一个packet，赋值n个像素
		{
			///< 减去最高位的值，剩余的就是相同像素的数量
			chunkHeader -= 127;

			in.read((char*)pixel.raw, bytespp);
			for (int i = 0; i < chunkHeader; i++)
			{
				for (int s = 0; s < bytespp; s++)
				{
					data[currentByte++] = pixel.raw[s];
				}
				currentPixel++;
				if (currentPixel > pixelCount)
				{
					std::cerr << " too many pixels read!" << std::endl;
					return false;
				}
			}
		}
	} while (currentPixel < pixelCount);


	return false;
}

bool TGAImage::unloadRleData(std::ofstream & out)
{
	const unsigned char max_chunk_length = 128;
	unsigned long npixels = width*height;
	unsigned long curpix = 0;
	while (curpix<npixels) {
		unsigned long chunkstart = curpix*bytespp;
		unsigned long curbyte = curpix*bytespp;
		unsigned char run_length = 1;
		bool raw = true;
		while (curpix + run_length<npixels && run_length<max_chunk_length) {
			bool succ_eq = true;
			for (int t = 0; succ_eq && t<bytespp; t++) {
				succ_eq = (data[curbyte + t] == data[curbyte + t + bytespp]);
			}
			curbyte += bytespp;
			if (1 == run_length) {
				raw = !succ_eq;
			}
			if (raw && succ_eq) {
				run_length--;
				break;
			}
			if (!raw && !succ_eq) {
				break;
			}
			run_length++;
		}
		curpix += run_length;
		out.put(raw ? run_length - 1 : run_length + 127);
		if (!out.good()) {
			std::cerr << "can't dump the tga file\n";
			return false;
		}
		out.write((char *)(data + chunkstart), (raw ? run_length*bytespp : bytespp));
		if (!out.good()) {
			std::cerr << "can't dump the tga file\n";
			return false;
		}
	}

	return true;
}

bool TGAImage::flipHorizontally()
{
	if (data == nullptr) return false;

	int half = width >> 1;
	for (int i = 0; i < half; i++)
	{
		for (int j = 0; j < height; j++)
		{
			TGAColor c1 = get(i, j);
			TGAColor c2 = get(width - 1 - i, j);
			set(i, j, c2);
			set(width - 1 - i, j, c1);
		}
	}

	return true;
}

bool TGAImage::flipVertically()
{
	if (data == nullptr) return false;

	unsigned long half = height >> 1;
	int bytesInLine = width * bytespp;
	unsigned char* line = new unsigned char[bytesInLine];
	for (int i = 0; i < half; i++)
	{
		unsigned long line1 = i * bytesInLine;
		unsigned long line2 = (height - 1 - i) * bytesInLine;

		memmove(line, data + line1, bytesInLine);
		memmove(data + line1, data + line2, bytesInLine);
		memmove(data + line2, line, bytesInLine);
	}
	delete[] line;

	return true;
}

bool TGAImage::scale(int w, int h)
{
	return false;
}

TGAColor TGAImage::get(int x, int y)
{
	if(data == nullptr || x < 0 || y < 0 || x > width || y > height)
		return TGAColor();
	return TGAColor(data + (x + y * width) * bytespp, bytespp);
}

bool TGAImage::set(int x, int y, TGAColor c)
{
	if( data == nullptr || x < 0 || y < 0|| x > width || y > height)
		return false;

	memcpy(data + (x + y * width)*bytespp, c.raw, bytespp);
	return true;
}

int TGAImage::getWidth()
{
	return width;
}

int TGAImage::getHeight()
{
	return height;
}

unsigned char * TGAImage::raw()
{
	return data;
}

void TGAImage::clear()
{
	if (data != nullptr)
		memset(data, 0, bytespp * width * height);
}





