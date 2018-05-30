#pragma once
#include "FileLoader.h"
#include <fstream>
const std::string szRootPath = "C:";
const std::string szCurrPath = "\\";

FileLoader::FileLoader()
{
}
FileLoader::FileInfo FileLoader::LoadBinary(const char* filename)
{
	std::ifstream fin(filename, std::ios::binary);
	fin.seekg(0, std::ios_base::end);
	std::ifstream::pos_type size = fin.tellg();

	char *pBuffer = new char[size];
	fin.seekg(0, std::ios_base::beg);
	fin.read(pBuffer, size);
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
		delete[] file_info.fileItself;
}
