#pragma once
#include "FileLoader.h"
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <regex>
namespace fs = std::experimental::filesystem;
using namespace std;
using namespace wxGame;

const std::string szRootPath = "C:";
const std::string szCurrPath = "\\";

FileLoader::FileLoader()
{
	file_info.fileItself = nullptr;
	file_info.fileLength = 0;
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

void FileLoader::GetTitleBySuffix(std::vector<std::string>& return_title, std::string path, std::string match, std::string suffix)
{
	regex match_ogex(match);
	//match_results<string::const_iterator> result;
	for (auto& it : fs::directory_iterator(path))
	{
		auto itp = it.path();
		auto temp = itp.filename();
		bool may = std::regex_match(temp.string(), match_ogex);
		if (may)
		{
			std::string formatTitle = temp.string();
			size_t pos = formatTitle.find(suffix);
			std::string title = formatTitle.substr(0, pos - 0);
			return_title.push_back(title);
		}
	}
}

void FileLoader::GetNameByNameAndSuffix(std::vector<std::string>& return_title, std::string path, std::string match, std::string suffix, std::string name)
{
	std::string AppendStr = name + match;
	regex match_ogex(AppendStr);
	//match_results<string::const_iterator> result;
	for (auto& it : fs::directory_iterator(path))
	{
		auto itp = it.path();
		auto temp = itp.filename();
		bool may = std::regex_match(temp.string(), match_ogex);
		if (may)
		{
			return_title.push_back(temp.string());
		}
	}
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
