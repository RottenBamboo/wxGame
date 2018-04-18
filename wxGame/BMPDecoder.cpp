#pragma once
#include "BMPDecoder.h"
#include "Mathmatic.h"
#include "FileLoader.h"
using namespace Mathmatic;
ImageCommon BMPDecoder::BMPParser(DataBuffer& dataBuffer)
{
	const BITMAP_FILE_HEADER* pBitMapFileHeader = reinterpret_cast<BITMAP_FILE_HEADER*>(dataBuffer.GetData());
	const BITMAP_INFO_HEADER* pBitMapInfoHeader = reinterpret_cast<BITMAP_INFO_HEADER*>((unsigned char*)dataBuffer.GetData() + BIT_MAPFILEHEADER_SIZE);
	if (pBitMapFileHeader->bfType == 0x4D42 /* 'B''M' */)
	{
		std::cout << "Asset is Windows BMP file" << std::endl;
		std::cout << "BMP Header" << std::endl;
		std::cout << "----------------------------" << std::endl;
		std::cout << "File Size: " << pBitMapFileHeader->bfSize << std::endl;
		std::cout << "Data Offset: " << pBitMapFileHeader->bfOffBits << std::endl;
		std::cout << "Image Width: " << pBitMapInfoHeader->biWidth << std::endl;
		std::cout << "Image Height: " << pBitMapInfoHeader->biHeight << std::endl;
		std::cout << "Image Planes: " << pBitMapInfoHeader->biPlanes << std::endl;
		std::cout << "Image BitCount: " << pBitMapInfoHeader->biBitCount << std::endl;
		std::cout << "Image Compression: " << pBitMapInfoHeader->biCompression << std::endl;
		std::cout << "Image Size: " << pBitMapInfoHeader->biSizeImage << std::endl;

	}
	ImageCommon imgCommon;
	imgCommon.imWidth = pBitMapInfoHeader->biWidth;
	imgCommon.imHeight = pBitMapInfoHeader->biHeight;
	imgCommon.imBitCount = 32;
	unsigned int byteCount = imgCommon.imBitCount >> 3;//bitcount divide 8 is byte number
	imgCommon.imPitch = ((imgCommon.imWidth * byteCount) + 3)&~3;	//the size of each pitch should be multiply by 4, we add 3 and set the last two bit to zero.
	imgCommon.imDataSize = imgCommon.imPitch * imgCommon.imHeight;
	DataBuffer* pDataBuff = new DataBuffer(nullptr, imgCommon.imDataSize, imgCommon.imPitch);
	imgCommon.imData = pDataBuff->GetData();

	if (imgCommon.imBitCount < 24)
	{
		std::cout << "only true color format of BMP" << std::endl;
	}
	else
	{
		Vector<4, unsigned char> *destData;
		Vector<4, unsigned char> *srcData;
		const unsigned char* dataBegin = dataBuffer.GetData() + pBitMapFileHeader->bfOffBits;
		for (int itry = imgCommon.imHeight - 1; itry >= 0; itry--)
		{
			for (unsigned int itrx = 0; itrx < imgCommon.imWidth; itrx++)
			{
				destData = (Vector<4, unsigned char>*)((unsigned char*)(imgCommon.imData) + imgCommon.imPitch *(imgCommon.imHeight - itry - 1)+ itrx * byteCount);
				srcData = (Vector<4, unsigned char>*)(dataBegin + imgCommon.imPitch * itry + itrx * byteCount);
				destData->element[2] = srcData->element[0];
				destData->element[1] = srcData->element[1];
				destData->element[0] = srcData->element[2];
				destData->element[3] = srcData->element[3];
			}
		}
	}

	return imgCommon;
}

DataBuffer BMPDecoder::BMPLoader(const char * filename)
{
	FileLoader fileLoader;
	FileLoader::FileInfo fileInfo;
	fileInfo = fileLoader.LoadBinary(filename);
	DataBuffer dataBuffer((void*)fileInfo.fileItself, fileInfo.fileLength, 4);
	return dataBuffer;
}
