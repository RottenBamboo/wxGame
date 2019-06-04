#pragma once
#include "stdafx.h"
#include <d3dcommon.h>
#include <d3dcompiler.h>
#define MATCH_OGEX "[\\S\\s]*\\.ogex$"
#define SUFFIX_OGEX ".ogex"
#define MATCH_TEXTURE "_texture_[0-9]*\\.bmp$"
#define SUFFIX_BMP ".bmp"
#define MATCH_NORMALMAP "_normal_map_[0-9]*\\.bmp$"
namespace wxGame {
	/*
	const std::string szFindOgex = "[\\S\\s]*\\.ogex$";
	const std::string szOgex = ".ogex";
	const std::string szOgexsBMP = "_texture_[0-9]*\\.bmp$";
	const std::string szBMP = ".bmp";
	*/
	class FileLoader
	{
	public:
		struct FileInfo {
			char* fileItself;
			int fileLength;
		};

		FileInfo LoadBinary(const char* filename,bool pureBinary);
		void GetTitleBySuffix(std::vector<std::string>& assetTitle, std::string path, std::string match, std::string suffix);
		void GetNameByNameAndSuffix(std::vector<std::string>& textureTitle, std::string assetTitle, std::string path, std::string match, std::string suffix);
		FileLoader();

		~FileLoader();
	private:
		FileInfo file_info;
	};

	std::string GetFullPath(const char* longPathName, const char* longPathFileName);
	extern FileLoader* g_pFileLoader;
}