#pragma once
#include "FileLoader.h"
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <regex>
#include<stdio.h>
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
	if (size == 0)
	{
		file_info.fileItself = nullptr;
		file_info.fileLength = 0;
	}
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

void FileLoader::GetNameByNameAndSuffix(std::vector<std::string>& return_title, std::string path, std::vector<std::string>& match, std::string suffix, std::string name)
{
	//int count = 0;
	char* p_decade = new char[10];
	char* p_unit = new char[10];
	for (std::vector<std::string>::iterator itr = match.begin(); itr!= match.end(); itr++)
	{
		std::string AppendStr = name + *itr;
		regex match_regex(AppendStr);
		int typeCount = 0;
		for (auto& it : fs::directory_iterator(path))
		{
			auto itp = it.path();
			auto temp = itp.filename();
			bool may = std::regex_match(temp.string(), match_regex);
			if (may)
			{
				char decade = *(temp.string().rbegin() + suffix.size() + 1);
				char unit = *(temp.string().rbegin() + suffix.size());
				int num = atoi(&decade) * 10 + atoi(&unit);
				return_title.push_back(temp.string());
				typeCount++;
				if (num > typeCount) //文件的后缀数字num如果大于了typeCount(实际的位置)，说明之前有某个位置没有对应的文件，这时应该补全
				{
					for (size_t i = 0; i < num - typeCount; i++) //
					{
						std::string defaultInsert = temp.string();
						_itoa_s(typeCount / 10, p_decade, 10, 10);//typeCount 是push次数，正好等于需要插入的文件名的后缀数字
						_itoa_s(typeCount % 10, p_unit,10 ,10);
						*(defaultInsert.rbegin() + suffix.size() + 1) = *p_decade;
						*(defaultInsert.rbegin() + suffix.size()) = *p_unit;
						return_title.insert(return_title.end() - 1, defaultInsert);
						typeCount++;
					}
				}
			}
		}
		//if (typeCount > count)//某一GeometryObject所需的资源文件数量大于先前的资源文件数量，则将先前的资源insert数量差个空字符串。
		//						//作用是将资源数量统一，SetGraphicsRootShaderResourceView时正确匹配到对应的着色器寄存器位置
		//{
		//	if (count > 0)
		//	{
		//		for (size_t i = count; i != typeCount - count; i++)
		//		{

		//			for (size_t j = 0; j != itr - match.end(); j++)
		//				return_title.insert(return_title.begin() + j * count + j, "");//
		//		}
		//	}
		//	count = typeCount;
		//}
		//if ( count > typeCount)
		//{
		//	for (size_t i = 0; i != count - typeCount; i++)
		//	{
		//		return_title.push_back("");
		//	}
		//}
	}
	delete[] p_decade;
	delete[] p_unit;
	p_decade = nullptr;
	p_unit = nullptr;
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
