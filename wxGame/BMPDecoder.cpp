#pragma once
#include "BMPDecoder.h"
#include "Mathmatic.h"
#include "FileLoader.h"
#include <algorithm>
using namespace Mathmatic;
using namespace wxGame;
void BMPDecoder::BMPParser(ImageCommon& imgCom)
{
	const BITMAP_FILE_HEADER* pBitMapFileHeader = reinterpret_cast<BITMAP_FILE_HEADER*>(BMPDataBuffer.GetData());
	const BITMAP_INFO_HEADER* pBitMapInfoHeader = reinterpret_cast<BITMAP_INFO_HEADER*>((unsigned char*)BMPDataBuffer.GetData() + BIT_MAPFILEHEADER_SIZE);
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
	imgCom.imWidth = pBitMapInfoHeader->biWidth;
	imgCom.imHeight = pBitMapInfoHeader->biHeight;
	imgCom.imBitCount = 32;
	unsigned int byteCount = imgCom.imBitCount >> 3;//bitcount divide 8 is equal with the byte count
	imgCom.imPitch = ((imgCom.imWidth * byteCount) + 3)&~3;	//the size of each pitch should be multiply by 4, we add 3 and set the last two bit to zero.
	imgCom.imDataSize = imgCom.imPitch * imgCom.imHeight;
	DataBuffer* pDataBuff = new DataBuffer(nullptr, imgCom.imDataSize, imgCom.imPitch);
	imgCom.imData = pDataBuff->GetData();

	if (pBitMapInfoHeader->biBitCount == 24)//make 24 bits format transform to 32 bits format
	{
		Vector<4, unsigned char> *destData;
		Vector<3, unsigned char> *srcData;
		const unsigned char* dataBegin = BMPDataBuffer.GetData() + pBitMapFileHeader->bfOffBits;
		int tempPitch = ((imgCom.imWidth * 3) + 3)&~3; //alignment
		for (int itry = imgCom.imHeight - 1; itry >= 0; itry--)
		{
			for (unsigned int itrx = 0; itrx < imgCom.imWidth; itrx++)
			{
				destData = (Vector<4, unsigned char>*)((unsigned char*)(imgCom.imData) + imgCom.imPitch *(imgCom.imHeight - itry - 1) + itrx * byteCount);
				srcData = (Vector<3, unsigned char>*)(dataBegin + tempPitch * itry + itrx * byteCount * 3 / 4);
				destData->element[2] = srcData->element[0];
				destData->element[1] = srcData->element[1];
				destData->element[0] = srcData->element[2];
				destData->element[3] = 0;
			}
		}
		imgCom.imBitCount = 32;
	}
	else if (pBitMapInfoHeader->biBitCount == 32)
	{
		Vector<4, unsigned char> *destData;
		Vector<4, unsigned char> *srcData;
		const unsigned char* dataBegin = BMPDataBuffer.GetData() + pBitMapFileHeader->bfOffBits;
		for (int itry = imgCom.imHeight - 1; itry >= 0; itry--)
		{
			for (unsigned int itrx = 0; itrx < imgCom.imWidth; itrx++)
			{
				destData = (Vector<4, unsigned char>*)((unsigned char*)(imgCom.imData) + imgCom.imPitch *(imgCom.imHeight - itry - 1) + itrx * byteCount);
				srcData = (Vector<4, unsigned char>*)(dataBegin + imgCom.imPitch * itry + itrx * byteCount);
				destData->element[2] = srcData->element[0];
				destData->element[1] = srcData->element[1];
				destData->element[0] = srcData->element[2];
				destData->element[3] = srcData->element[3];
			}
		}
	}
}

void BMPDecoder::BMPLoader(const char * filename)
{
	FileLoader fileLoader;
	FileLoader::FileInfo fileInfo;
	fileInfo = fileLoader.LoadBinary(filename, true);
	BMPDataBuffer = DataBuffer((void*)fileInfo.fileItself, fileInfo.fileLength, 4);
}

