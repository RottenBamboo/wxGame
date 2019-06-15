#pragma once
#include "stdafx.h"
#include <d3dcommon.h>
#include <d3dcompiler.h>
namespace wxGame {

	class FileLoader
	{
	public:
		struct FileInfo {
			char* fileItself;
			int fileLength;
		};

		FileInfo LoadBinary(const char* filename,bool pureBinary);
		void GetTitleBySuffix(std::vector<std::string>& assetTitle, std::string path, std::string match, std::string suffix);
		void GetNameByNameAndSuffix(std::vector<std::string>& return_title, std::string path, std::vector<std::string>& match, std::string suffix, std::string name);
		FileLoader();

		~FileLoader();
	private:
		FileInfo file_info;
	};

	std::string GetFullPath(const char* longPathName, const char* longPathFileName);
	extern FileLoader* g_pFileLoader;
}