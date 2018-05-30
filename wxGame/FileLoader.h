#pragma once
#include <string>
#include <d3dcommon.h>
#include <d3dcompiler.h>
class FileLoader
{
public:
	struct FileInfo {
		char* fileItself;
		int fileLength;
	};

	FileInfo LoadBinary(const char* filename);
	FileLoader();
	
	~FileLoader();
private:
	FileInfo file_info;
};

std::string GetFullPath(const char* longPathName, const char* longPathFileName);
