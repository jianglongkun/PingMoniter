#include "UtilsImpl.h"

UtilsImpl::UtilsImpl()
{

}

UtilsImpl::~UtilsImpl()
{

}

wstring UtilsImpl::UTF8_TO_UNICODE(string UTF8String)
{
	int UNICODESize = MultiByteToWideChar(CP_UTF8, 0, UTF8String.c_str(), -1, NULL, 0);
	if (UNICODESize == ERROR_NO_UNICODE_TRANSLATION)
	{
		printf("[%s][%d]:Invalid UTF8 sequence.\n", __FUNCTION__, __LINE__);
		return (wstring)NULL;
	}

	if (UNICODESize == 0)
	{
		printf("[%s][%d]:Error in conversion.\n", __FUNCTION__, __LINE__);
		return (wstring)NULL;
	}

	vector<WCHAR> ResultString(UNICODESize);
	int ConvertResult = MultiByteToWideChar(CP_UTF8, 0, UTF8String.c_str(), -1, &ResultString[0], UNICODESize);
	if (ConvertResult != UNICODESize)
	{
		printf("[%s][%d]:La falla.\n", __FUNCTION__, __LINE__);
		return (wstring)NULL;
	}
	return wstring(&ResultString[0]);
}

string UtilsImpl::UTF8_TO_ASCII(string UTF8String)
{
	string strRet = "";
	wstring Wstr = UTF8_TO_UNICODE(UTF8String);
	strRet = UNICODE_TO_ASCII(Wstr);
	return strRet;
}

string UtilsImpl::UNICODE_TO_ASCII(wstring UNICODEString)
{
	int ASCIISize = WideCharToMultiByte(CP_OEMCP, 0, UNICODEString.c_str(), -1, NULL, 0, NULL, NULL);
	if (ASCIISize == ERROR_NO_UNICODE_TRANSLATION)
	{
		printf("[%s][%d]:Invalid UTF-8 sequence.\n", __FUNCTION__, __LINE__);
		return (string)NULL;
	}

	if (ASCIISize == 0)
	{
		printf("[%s][%d]:Error in conversion.\n", __FUNCTION__, __LINE__);
		return (string)NULL;
	}

	std::vector<char> ResultString(ASCIISize);
	int convresult = ::WideCharToMultiByte(CP_OEMCP, 0, UNICODEString.c_str(), -1, &ResultString[0], ASCIISize, NULL, NULL);
	if (convresult != ASCIISize)
	{
		printf("[%s][%d]:La falla.\n", __FUNCTION__, __LINE__);
		return (string)NULL;
	}
	return std::string(&ResultString[0]);
}

string UtilsImpl::UNICODE_TO_UTF8(wstring UNICODEString)
{
	int UTF8Size = ::WideCharToMultiByte(CP_UTF8, 0, UNICODEString.c_str(), -1, NULL, 0, NULL, NULL);
	if (UTF8Size == 0)
	{
		printf("[%s][%d]:Error in conversion.\n", __FUNCTION__, __LINE__);
		return (string)NULL;
	}
	std::vector<char> resultstring(UTF8Size);
	int convresult = ::WideCharToMultiByte(CP_UTF8, 0, UNICODEString.c_str(), -1, &resultstring[0], UTF8Size, NULL, NULL);
	if (convresult != UTF8Size)
	{
		printf("[%s][%d]:La falla.\n", __FUNCTION__, __LINE__);
		return (string)NULL;
	}
	return std::string(&resultstring[0]);
}

wstring UtilsImpl::ASCII_TO_UNICODE(string ASCIIString)
{
	int UNICODESize = MultiByteToWideChar(CP_ACP, 0, (char*)ASCIIString.c_str(), -1, NULL, 0);
	if (UNICODESize == ERROR_NO_UNICODE_TRANSLATION)
	{
		printf("[%s][%d]:Invalid UTF-8 sequence.\n", __FUNCTION__, __LINE__);
		return (wstring)NULL;
	}
	if (UNICODESize == 0)
	{
		printf("[%s][%d]:Error in conversion.\n", __FUNCTION__, __LINE__);
		return (wstring)NULL;
	}

	std::vector<WCHAR> ResultString(UNICODESize);
	int convresult = MultiByteToWideChar(CP_ACP, 0, (char*)ASCIIString.c_str(), -1, &ResultString[0], UNICODESize);
	if (convresult != UNICODESize)
	{
		printf("[%s][%d]:La falla.\n", __FUNCTION__, __LINE__);
		return (wstring)NULL;
	}
	return std::wstring(&ResultString[0]);
}

string UtilsImpl::ASCII_TO_UTF8(string ASCIIString)
{
	string strRet("");
	wstring wstr = ASCII_TO_UNICODE(ASCIIString);
	strRet = UNICODE_TO_UTF8(wstr);
	return strRet;
}

int UtilsImpl::GetFolderFileDescriptionInfo(const char *pFolderPath, vector<FileDescriptionInfo> &vFileDescriptionInfo)
{
	int iFileCount = 0;
	if (pFolderPath == NULL)
		return -1;

	vFileDescriptionInfo.clear();

	struct _finddata_t fd;
	char FindDir[256] = { 0 };
	char cFolderPath[256] = { 0 };

	if ((pFolderPath[strlen(pFolderPath) - 1] != '/') || (pFolderPath[strlen(pFolderPath) - 1] != '\\'))
	{
		sprintf(FindDir, "%s/*.*", pFolderPath);
		sprintf(cFolderPath, "%s/", pFolderPath);
	}
	else
	{
		sprintf(FindDir, "%s*.*", pFolderPath);
		sprintf(cFolderPath, "%s", pFolderPath);
	}

	intptr_t pf = _findfirst(FindDir, &fd);
	if (pf == -1)
	{
		return -1;
	}

	while (!_findnext(pf, &fd))
	{
		if ((strcmp(fd.name, ".") == 0) || (strcmp(fd.name, "..") == 0))
		{
			continue;
		}

		if (fd.attrib & _A_SUBDIR)
		{
			continue;
		}
		else
		{
			FileDescriptionInfo stFileDescriptionInfo;
			stFileDescriptionInfo.FileName=fd.name;
			stFileDescriptionInfo.FilePath.append(cFolderPath);
			stFileDescriptionInfo.FilePath.append(fd.name);
			stFileDescriptionInfo.FileLength = fd.size;
			stFileDescriptionInfo.FileLastModify = fd.time_write;
			vFileDescriptionInfo.push_back(stFileDescriptionInfo);
			iFileCount++;
		}
	}
	_findclose(pf);
	return iFileCount;
}

bool UtilsImpl::GetFileVersionInformation(FileVersionInfo &FileVersionInfo)
{
	TCHAR szAppPath[MAX_PATH] = { 0 };

	if (GetModuleFileName(NULL, szAppPath, MAX_PATH))
	{
		DWORD dwVerionHandle = 0;
		DWORD dwVerionInfoSize = GetFileVersionInfoSize(szAppPath, &dwVerionHandle);

		if (dwVerionInfoSize)
		{
			HANDLE hMemory = NULL;
			void *pMemory = NULL;
			unsigned int uInfoSize = 0;
			VS_FIXEDFILEINFO *pFileInfo;

			hMemory = GlobalAlloc(GMEM_MOVEABLE, dwVerionInfoSize);
			pMemory = GlobalLock(hMemory);

			if (GetFileVersionInfo(szAppPath, dwVerionHandle, dwVerionInfoSize, pMemory) == false)
			{
				::GlobalUnlock(hMemory);
				::GlobalFree(hMemory);
				return false;
			}

			if (VerQueryValue(pMemory, _T("\\"), (void **)&pFileInfo, &uInfoSize) == false)
			{
				::GlobalUnlock(hMemory);
				::GlobalFree(hMemory);
				return false;
			}

			WORD wProductVersion[4] = { 0 };
			WORD wFileVersion[4] = { 0 };

			wProductVersion[0] = HIWORD(pFileInfo->dwProductVersionMS);
			wProductVersion[1] = LOWORD(pFileInfo->dwProductVersionMS);
			wProductVersion[2] = HIWORD(pFileInfo->dwProductVersionLS);
			wProductVersion[3] = LOWORD(pFileInfo->dwProductVersionLS);

			wFileVersion[0] = HIWORD(pFileInfo->dwFileVersionMS);
			wFileVersion[1] = LOWORD(pFileInfo->dwFileVersionMS);
			wFileVersion[2] = HIWORD(pFileInfo->dwFileVersionLS);
			wFileVersion[3] = LOWORD(pFileInfo->dwFileVersionLS);

			char szProductVersion[256] = { 0 };
			char szszFileVersion[256] = { 0 };

			sprintf(szProductVersion, "%d.%d.%d.%d", wProductVersion[0], wProductVersion[1], wProductVersion[2], wProductVersion[3]);
			sprintf(szszFileVersion, "%d.%d.%d.%d", wFileVersion[0], wFileVersion[1], wFileVersion[2], wFileVersion[3]);

			FileVersionInfo.szProductVersion = szProductVersion;
			FileVersionInfo.szFileVersion = szszFileVersion;

			FileVersionInfo.uMajorVersion = wProductVersion[0];
			FileVersionInfo.uMinorVersion = wProductVersion[2];
			FileVersionInfo.uBranchVersion = wProductVersion[3];

			::GlobalUnlock(hMemory);

			::GlobalFree(hMemory);

			return true;
		}
		else
		{

			return false;
		}
	}
	else
	{
		return false;
	}
}

bool UtilsImpl::EnumAllProcess(vector<ProcessInfo> &vProcessInfo)
{
	vProcessInfo.clear();
	PROCESSENTRY32 pe32;
	memset(&pe32, 0, sizeof(PROCESSENTRY32));
	pe32.dwSize = sizeof(PROCESSENTRY32);
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		DWORD ErrorCode = GetLastError();
		printf("CreateToolhelp32Snapshot fail ErrorCode:%d\n", ErrorCode);
		return false;
	}

	bool bMore = Process32First(hProcessSnap, &pe32);
	while (bMore)
	{
		ProcessInfo _ProcessInfo = { 0 };
		_ProcessInfo.ProcessName = UNICODE_TO_ASCII(pe32.szExeFile).c_str();
		_ProcessInfo.uParentProcessID = pe32.th32ParentProcessID;
		_ProcessInfo.uProcessID = pe32.th32ProcessID;
		vProcessInfo.push_back(_ProcessInfo);
		bMore = Process32Next(hProcessSnap, &pe32);
	}

	CloseHandle(hProcessSnap);
	return true;
}

bool UtilsImpl::FindProcess(string &ProcessName)
{
	return true;
}

bool UtilsImpl::KillProcess(uint32_t uProcessID)
{
	DWORD dwDesiredAccess = PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_TERMINATE;
	HANDLE hProcessHandle = OpenProcess(dwDesiredAccess, false, uProcessID);
	if (hProcessHandle == NULL)
	{
		DWORD ErrorCode = GetLastError();
		printf("OpenProcess fail ErrorCode:%d\n", ErrorCode);
		return false;
	}

	bool nRet = TerminateProcess(hProcessHandle, 0);
	if (!nRet)
	{
		DWORD ErrorCode = GetLastError();
		printf("TerminateProcess fail ErrorCode:%d\n", ErrorCode);
		return false;
	}

	return true;
}

time_t UtilsImpl::GetFileModifyTime(const char *pFilePath, long long *pFileLength)
{
	struct _finddata_t fd;
	intptr_t pf = _findfirst(pFilePath, &fd);
	if (pf == -1)
	{
		return -1;
	}
	time_t ModifyTime = 0;
	ModifyTime = fd.time_write;
	*pFileLength = fd.size;
	_findclose(pf);
	return ModifyTime;
}

bool UtilsImpl::SetFileModifyTime(const char *pFilePath, time_t tLastModifyTime)
{
	struct tm FileViewTimeStruct = { 0 };
	struct tm FileModifyTimeStruct = { 0 };

	struct utimbuf ut = { 0 };

	time_t LocalTime = time(NULL);
	FileViewTimeStruct = *localtime(&LocalTime);

	time_t FileModifyTime;
	FileModifyTime = tLastModifyTime;
	FileModifyTimeStruct = *localtime(&FileModifyTime);

	ut.modtime = mktime(&FileModifyTimeStruct);
	ut.actime = mktime(&FileViewTimeStruct);
	int nRet = utime(pFilePath, &ut);

	if (nRet == -1)
	{
		switch (errno)
		{
			case EACCES:
				printf("Insufficient permissions\n");
				break;
			case EINVAL:
				printf("Invalid time parameter\n");
				break;
			case EMFILE:
				printf("Too many open files\n");
				break;
			case ENOENT:
				printf("Local file does not exist\n");
				break;
		}
		return false;
	}
	return true;
}

bool UtilsImpl::ReadFileText(const char *pFilePath, string &szFileText)
{
	FILE *fp = fopen(pFilePath, "rb");
	if (fp == NULL)
		return false;

	fseek(fp, 0, SEEK_SET);
	fseek(fp, 0, SEEK_END);
	long lBytes=ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (lBytes <= 0)
	{
		fclose(fp);
		return false;
	}

	char *pText = (char *)malloc(lBytes + 1);
	if (pText == NULL)
	{
		fclose(fp);
		return false;
	}

	memset(pText, 0, lBytes + 1);

	fread(pText, lBytes, 1, fp);

	fclose(fp);

	szFileText = pText;
	free(pText);
	return true;
}

bool UtilsImpl::QueryInterfaceDomain(string &szLoginDomainName)
{
	char *pCFGFilePath = "./cfg.db";
	FILE *fp = fopen(pCFGFilePath, "r");
	if (fp == NULL)
		return false;

	fclose(fp);

	SqliteWrapperDB db;
	db.Open(pCFGFilePath);
	char QueryString[256] = { 0 };
	sprintf(QueryString, "SELECT ServerURL.URL as InterfaceUrl FROM InternationalCodeTable LEFT JOIN ServerURL ON ServerURL.ID = InternationalCodeTable.InterfaceUrl WHERE InternationalCodeTable.InternationalCode = %d;", 0);
	SqliteWrapperQuery Query = db.ExecQuery(QueryString);
	if (Query.Eof())
	{
		Query.Finalize();
		db.Close();
		return false;
	}
	szLoginDomainName = Query.FieldValue("InterfaceUrl");
	Query.Finalize();
	db.Close();
	return true;
}

bool UtilsImpl::CreateFileDirectory(const char *pDirectoryPath)
{
	char DirPathString[256] = { 0 };

	if (pDirectoryPath == NULL || strlen(pDirectoryPath) == 0)
	{
		return false;
	}

	if (pDirectoryPath[strlen(pDirectoryPath) - 1] == '/' || pDirectoryPath[strlen(pDirectoryPath) - 1] == '\\')
	{
		strcpy(DirPathString, pDirectoryPath);
	}
	else
	{
		sprintf(DirPathString, "%s/", pDirectoryPath);
	}
	char MaterialFolderPath[256] = { 0 };
	while (1)
	{
		char *pos = strstr(DirPathString, "/");
		if (pos == NULL)
			break;
		char CreateMaterialPath[256] = { 0 };
		memcpy(CreateMaterialPath, DirPathString, pos - DirPathString);
		strcat(MaterialFolderPath, CreateMaterialPath);
		if (access(MaterialFolderPath, 0) == -1)
		{
#ifdef _WIN32
			if (mkdir(MaterialFolderPath) == -1)
#else
			if (mkdir(MaterialFolderPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1)
#endif
			{
				return false;
			}
		}
		char TmpMaterialServerPath[256] = { 0 };
		memcpy(TmpMaterialServerPath, DirPathString + ((pos + 1) - DirPathString), strlen(pos));
		memset(DirPathString, 0, 256);
		strcpy(DirPathString, TmpMaterialServerPath);
		strcat(MaterialFolderPath, "/");
	}
	return true;
}

bool UtilsImpl::IsFilePathValid(const char *pFilePath)
{
	if (access(pFilePath, 0) == -1)
	{
		return  false;
	}
	return true;
}

string UtilsImpl::GetInstallPath()
{
	char szAppPath[MAX_PATH] = { 0 };
	char szDrive[MAX_PATH] = { 0 }, szDir[MAX_PATH] = { 0 }, szFileName[MAX_PATH] = { 0 }, szExt[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, szAppPath, MAX_PATH);
	_splitpath(szAppPath, szDrive, szDir, szFileName, szExt);
	string str(szDrive);
	str.append(szDir);
	return str;
}

long UtilsImpl::GetFileBytes(string FilePath)
{
	long lBytes = 0;

	FILE *fp = fopen(FilePath.c_str(), "r");

	if (fp == NULL)
		return lBytes;

	fseek(fp, 0, SEEK_SET);
	fseek(fp, 0, SEEK_END);
	lBytes = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fclose(fp);
	return lBytes;
}

long UtilsImpl::GetFileBytes(FILE *fp)
{
	long lBytes = 0;

	if (fp == NULL)
		return lBytes;

	fseek(fp, 0, SEEK_SET);
	fseek(fp, 0, SEEK_END);
	lBytes = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	return lBytes;
}