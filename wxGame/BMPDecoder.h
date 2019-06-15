#pragma once
#include "stdafx.h"
#include "DataBuffer.h"
#define BIT_MAPFILEHEADER_SIZE 14

typedef struct ImageCommon
{
	unsigned int imWidth;
	unsigned int imHeight;
	void* imData;
	unsigned int imBitCount;
	unsigned int imPitch;
	size_t  imDataSize;
	ImageCommon()
	{
		imWidth = 0;
		imHeight = 0;
		imData = nullptr;
		imBitCount = 0;
		imPitch = 0;
		imDataSize = 0;
	}
	~ImageCommon()
	{
		imWidth = 0;
		imHeight = 0;
		delete[] imData;
		imData = nullptr;
		imBitCount = 0;
		imPitch = 0;
		imDataSize = 0;
	}
	ImageCommon(const ImageCommon & icom)
	{
		copy(icom);
	}
	ImageCommon& operator=(const ImageCommon& icom)
	{
		copy(icom);
		return *this;
	}
	void copy(const ImageCommon & icom)
	{
		imWidth = icom.imWidth;
		imHeight = icom.imHeight;
		imDataSize = icom.imDataSize;
		imData = new unsigned char[icom.imDataSize];
		memcpy(imData, icom.imData, icom.imDataSize);
		imBitCount = icom.imBitCount;
		imPitch = icom.imPitch;
	}
} ImageCommon;


#pragma pack(push, 1)
struct BITMAP_FILE_HEADER
{
	unsigned short bfType;
	unsigned int bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned int bfOffBits;
};

struct BITMAP_INFO_HEADER	//the msdn document url:https://msdn.microsoft.com/en-us/library/windows/desktop/dd183376(v=vs.85).aspx
{
	unsigned int biSize;	// the number of bytes required by the structure
	int biWidth;	// the width of bitmap, in pixels.
	int biHeight;	// the height of bitmap, inpixels.
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned int biCompression;
	unsigned int biSizeImage;
	int biXPelsPerMeter;
	int biYPelsPerMeter;
	unsigned int biClrUsed;
	unsigned int biClrImportant;
};
#pragma pack(pop)

class BMPDecoder
{
public:
	BMPDecoder():BMPDataBuffer(){};
	~BMPDecoder() {};
	void BMPParser(ImageCommon& imgCom);
	void BMPLoader(const char* filename);

	DataBuffer BMPDataBuffer;
	//ImageCommon BMPtoImageCommon(const char* filename);
private:
	BITMAP_FILE_HEADER m_bitmapFileHeader;
	BITMAP_INFO_HEADER m_bitmapInfoHeader;
};

