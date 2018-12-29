#pragma once
#include "FileLoader.h"
#include <fstream>
using namespace wxGame;

const std::string szRootPath = "C:";
const std::string szCurrPath = "\\";

FileLoader::FileLoader()
{
}
FileLoader::FileInfo FileLoader::LoadBinary(const char* filename, bool pureBinary)
{
	std::ifstream fin(filename, std::ios::binary);
	fin.seekg(0, std::ios_base::end); // offset to end position
	std::ifstream::pos_type end = fin.tellg();
	size_t size = (int)end + 1; //end of the pBuffer is '\0', so size of the pBuffer should be one more
	char *pBuffer = new char[size];
	memset(pBuffer,0, size);
	fin.seekg(0, std::ios_base::beg);
	fin.read(pBuffer, size);

	if (!pureBinary)
	{
		pBuffer[size - 1] = '\0';
	}

	file_info.fileItself = pBuffer;
	file_info.fileLength = (int)fin.gcount();
	return file_info;
}
std::string GetFullPath(std::string strPathName, std::string longPathFileName)
{
	std::string TempPath;
	if (strPathName[1] == ':')
	{
		TempPath = longPathFileName;
		return TempPath;
	}

	if (strPathName[0] == '\\' || longPathFileName[0] == '/')
	{
		TempPath = szCurrPath + longPathFileName;
		return TempPath;
	}

	if (strPathName == "")
	{
		TempPath = szCurrPath + longPathFileName;
		return TempPath;
	}
	TempPath = "";
	return TempPath;
}

FileLoader::~FileLoader()
{
	if (file_info.fileItself != nullptr)
	{
		delete[] file_info.fileItself;
		file_info.fileItself = nullptr;
	}
}
