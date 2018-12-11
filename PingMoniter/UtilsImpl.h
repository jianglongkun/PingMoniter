#pragma once
#ifndef __UTILSIMPL__H_
#define __UTILSIMPL__H_

#include <windows.h>
#include <tchar.h>
#include <string>
#include <string.h>
#include <vector>
#include <io.h>
#include <direct.h>
#include <time.h>
#include <iphlpapi.h>
#include <iostream>
#include <stdint.h>
#include <sys/utime.h>
#include <psapi.h>
#include <tlhelp32.h>
#include "Sqlite\SqliteWrapper.h"

using namespace std;

#pragma comment(lib,"Version.lib")

#ifdef UNICODE
typedef wstring TSTRING;
#else
typedef string TSTRING;
#endif

typedef struct FileDescriptionInfo
{
	string FilePath;
	string FileName;
	long long FileLength;
	time_t FileLastModify;
}FILEDESCRIPTIONINFO,*LPFILEDESCRIPTIONINFO;

typedef struct FileVersionInfo
{
	string szFileVersion;
	string szProductVersion;
	uint32_t uMajorVersion;
	uint32_t uMinorVersion;
	uint32_t uBranchVersion;
}FILEVERSIONINFO,*LPFILEVERSIONINFO;

typedef struct ProcessInfo
{
	string ProcessName;
	uint32_t uProcessID;
	uint32_t uParentProcessID;
}PROCESSINFO,*LPPROCESSINFO;

class UtilsImpl
{
public:
	UtilsImpl();

	~UtilsImpl();

	static wstring UTF8_TO_UNICODE(string UTF8String);

	static string UTF8_TO_ASCII(string UTF8String);

	static string UNICODE_TO_ASCII(wstring UNICODEString);

	static string UNICODE_TO_UTF8(wstring UNICODEString);

	static wstring ASCII_TO_UNICODE(string ASCIIString);

	static string ASCII_TO_UTF8(string ASCIIString);

	static int GetFolderFileDescriptionInfo(const char *pFolderPath, vector<FileDescriptionInfo> &vFileDescriptionInfo);

	static bool GetFileVersionInformation(FileVersionInfo &FileVersionInfo);

	static bool EnumAllProcess(vector<ProcessInfo> &vProcessInfo);

	static bool FindProcess(string &ProcessName);

	static bool KillProcess(uint32_t uProcessID);

	static time_t GetFileModifyTime(const char *pFilePath, long long *pFileLength);

	static bool SetFileModifyTime(const char *pFilePath, time_t tLastModifyTime);

	static bool ReadFileText(const char *pFilePath, string &szFileText);

	static bool QueryInterfaceDomain(string &szLoginDomainName);

	static bool CreateFileDirectory(const char *pDirectoryPath);

	static bool IsFilePathValid(const char *pFilePath);

	static string GetInstallPath();

	static long GetFileBytes(string FilePath);

	static long GetFileBytes(FILE *fp);

};

#endif