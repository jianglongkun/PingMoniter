#include "MoniterImpl.h"
using namespace std;

#pragma comment(lib,"iphlpapi.lib")
#pragma comment(lib,"ws2_32.lib")

typedef struct MoniterInfo
{
	string Address;
	int Timeout;
	int Interval;
	MoniterInfo() :
		Timeout(1000),
		Interval(1000)
	{
		Address = "";
	}
};

typedef struct MoniterConfigureInfo
{
	MoniterImpl::MoniterParameterDescribe ObjMoniterParameterDescribe;
	vector<MoniterInfo> vMoniterInfo;
};

int main()
{
	FILE *fp = fopen("./Moniter.configure", "r");

	if (fp == NULL)
		return 0;

	long lBytes=UtilsImpl::GetFileBytes(fp);

	char *pConfigureString = new char[lBytes + 1];
	memset(pConfigureString, 0, lBytes + 1);

	fread(pConfigureString, 1, lBytes, fp);

	fclose(fp);


	Json::Reader Read;
	Json::Value Value;

	if (Read.parse(pConfigureString, Value, false) == false)
		return 0;

	MoniterConfigureInfo ObjMoniterConfigureInfo;
	ObjMoniterConfigureInfo.ObjMoniterParameterDescribe.WebHookURL = Value["WebHookURL"].asString();
	ObjMoniterConfigureInfo.ObjMoniterParameterDescribe.WebHookDescribe = Value["WebHookDescribe"].asString();
	ObjMoniterConfigureInfo.ObjMoniterParameterDescribe.MaxDelay = Value["MaxDelay"].asInt();
	ObjMoniterConfigureInfo.ObjMoniterParameterDescribe.MaxDelayPercent = Value["MaxDelayPercent"].asInt();
	ObjMoniterConfigureInfo.ObjMoniterParameterDescribe.MaxTimeoutPercent = Value["MaxTimeoutPercent"].asInt();
	ObjMoniterConfigureInfo.ObjMoniterParameterDescribe.WarningTimeoutSec = Value["WarningTimeoutSec"].asInt();
	ObjMoniterConfigureInfo.ObjMoniterParameterDescribe.StatisticsInterval = Value["StatisticsInterval"].asInt();

	for (int i = 0; i < Value["MoniterInfo"].size(); i++)
	{
		MoniterInfo ObjMoniterInfo;
		ObjMoniterInfo.Address = Value["MoniterInfo"][i]["Address"].asString();
		ObjMoniterInfo.Timeout = Value["MoniterInfo"][i]["Timeout"].asInt();
		ObjMoniterInfo.Interval = Value["MoniterInfo"][i]["Interval"].asInt();
		ObjMoniterConfigureInfo.vMoniterInfo.push_back(ObjMoniterInfo);
	}

	for (int i = 0; i < ObjMoniterConfigureInfo.vMoniterInfo.size(); i++)
	{
		MoniterImpl *pMoniterImpl = new MoniterImpl;
		pMoniterImpl->SetMoniterParameterDescribe(&ObjMoniterConfigureInfo.ObjMoniterParameterDescribe);
		pMoniterImpl->Start(ObjMoniterConfigureInfo.vMoniterInfo[i].Address, ObjMoniterConfigureInfo.vMoniterInfo[i].Interval);
	}

	BOOL bRet;
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) != 0)
	{
		//TranslateMessage(&msg);
		//DispatchMessage(&msg);
		if (msg.message == WM_TIMER)
		{
			MoniterImpl *pMoniterImpl = (MoniterImpl *)msg.lParam;
			if (msg.wParam == pMoniterImpl->m_WarningAnalysisTimerEventID)
			{
				pMoniterImpl->MoniterWarningAnalysisTimer();
			}

			if (msg.wParam == pMoniterImpl->m_NormalStatisticsTimerEventID)
			{
				pMoniterImpl->MoniterStatisticsTimer();
			}
		}
	}
	return 0;
}