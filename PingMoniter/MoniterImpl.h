#pragma once
#ifndef __MONITERIMPL__H_
#define __MONITERIMPL__H_

#include "HttpRequestClientImpl.h"
#include "ThreadImpl.h"
#include <windows.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include "Json\json\json.h"

class MoniterImpl:public ThreadDelegate
{
public:
	typedef struct MoniterParameterDescribe
	{
		string WebHookURL;
		string WebHookDescribe;
		int MaxDelay;
		int MaxDelayPercent;
		int MaxTimeoutPercent;
		int WarningTimeoutSec;
		int StatisticsInterval;
		MoniterParameterDescribe() :
			MaxDelay(0),
			MaxDelayPercent(0),
			MaxTimeoutPercent(0),
			WarningTimeoutSec(0),
			StatisticsInterval(0)
		{
			WebHookURL = "";
			WebHookDescribe = "";
		}
	}MONITERPARAMETERDESCRIBE,*LPMONITERPARAMETERDESCRIBE;

	UINT_PTR m_WarningAnalysisTimerEventID;
	UINT_PTR m_NormalStatisticsTimerEventID;
public:
	MoniterImpl();

	~MoniterImpl();

	void SetMoniterParameterDescribe(LPMONITERPARAMETERDESCRIBE pMoniterParameterDescribe);

	MoniterParameterDescribe GetMoniterParameterDescribe();

	bool Start(string Address,int iInterval=1000);

	void Stop();

	void MoniterStatisticsTimer();

	void MoniterWarningAnalysisTimer();

private:
	const char *RESULTDB_DIRECTORY = "ResultDB";
	const char *ROOT_PATH = "./";

	typedef struct ResultInfoDescribe
	{
		string Address;
		int Delay;
		int TTL;
		int ErrorCode;
		string ErrorString;
		string ResultDate;
	};
private:

	static void __stdcall MoniterStatisticsTimer(HWND hWnd, UINT uMsg, UINT_PTR idTimer, DWORD dwTime);

	static void __stdcall MoniterWarningAnalysisTimer(HWND hWnd, UINT uMsg, UINT_PTR idTimer, DWORD dwTime);

	bool PingResult(string &Address, int iInterval = 1000);

	virtual bool ThreadProcess(void *pUserObject);

	bool WarningAnalysis();

	bool WarningPush();

	bool NormalStatistics();

	bool ResultSaveDB(ResultInfoDescribe ObjResultInfoDescribe);

private:
	HttpRequestClientImpl m_WarningAnalysisHttpRequestClientImpl;
	HttpRequestClientImpl m_NormalStatisticsHttpRequestClientImpl;
	WinThreadImpl m_WinThreadImpl;
	string m_Address;
	int m_Interval;
	MoniterParameterDescribe m_MoniterParameterDescribe;
	bool m_bIsNormalStatistics;
	bool m_bIsWarningAnalysis;
	std::vector<ResultInfoDescribe> m_vWarningAnalysisResultInfo;

	std::vector<ResultInfoDescribe> m_vNormalStatisticsResultInfo;
};

#endif