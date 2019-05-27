#ifndef _TGAIMAGE_H__
#define _TGAIMAGE_H__

#include <fstream>

///< no alignment
#pragma pack(push, 1)
typedef struct TGAHeader
{
	char idLength;
	char colorMapType;
	char imageType;
	short firstEntry;
	short numEntries;
	char depth;
	short xOrigin;
	short yOrigin;
	short imageWidth;
	short imageHeight;
	char perPixelBits;
	char descriptor;
}TGAHeader;
#pragma pack(pop)


///< TGA color struct

typedef struct TGAColor
{
	union
	{
		struct
		{
			unsigned char b, g, r, a;
		};
		unsigned char raw[4];
		unsigned int val;
	};
	int bytespp;

	TGAColor() : val(0), bytespp(0){
	}

	TGAColor(unsigned char R, unsigned char G, unsigned char B, unsigned char A) : r(R), g(G), b(B), a(A), bytespp(4) {
	}

	TGAColor(int v, int bpp) : val(v), bytespp(bpp) {
	}
	
	TGAColor(const TGAColor& c) : val(c.val), bytespp(c.bytespp) {
	}
	
	TGAColor(const unsigned char* p, int bpp) : val(0), bytespp(bpp) {
		for (int i = 0; i < bpp; i++) {
			raw[i] = p[i];
		}
	}

	TGAColor& operator =(const TGAColor& c) {
		if (this != &c) {
			bytespp = c.bytespp;
			val = c.val;
		}
		return *this;
	}
};


class TGAImage
{
protected:
	unsigned char* data;
	int width;
	int height;
	int bytespp;

	///< RLE是run-length encoding行程长度压缩算法
	bool loadRleData(std::ifstream &in);
	bool unloadRleData(std::ofstream &out);

public:
	enum Format {
		GRAYSCALE=1, RGB=3, RGBA=4
	};

	TGAImage();
	TGAImage(int w, int h, int bpp);
	TGAImage(const TGAImage &img);
	~TGAImage();
	
	bool readTgaFile(const char* filename);
	bool writeTgaFile(const char* filename, bool rle = true);

	bool flipHorizontally();
	bool flipVertically();
	bool scale(int w, int h);
	TGAColor get(int x, int y);
	bool set(int x, int y, TGAColor c);
	int getWidth();
	int getHeight();
	unsigned char* raw();
	void clear();
};





#endif
