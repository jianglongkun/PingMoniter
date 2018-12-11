#include "MoniterImpl.h"
#include "Sqlite\SqliteWrapper.h"
#include <algorithm>
#include <iostream>

MoniterImpl::MoniterImpl():
	m_WarningAnalysisTimerEventID(NULL),
	m_NormalStatisticsTimerEventID(NULL),
	m_bIsNormalStatistics(false),
	m_bIsWarningAnalysis(false)
{
	m_WinThreadImpl.SetThreadDelegate(this,this);
}

MoniterImpl::~MoniterImpl()
{
	Stop();
}

void MoniterImpl::SetMoniterParameterDescribe(LPMONITERPARAMETERDESCRIBE pMoniterParameterDescribe)
{
	m_MoniterParameterDescribe = *pMoniterParameterDescribe;
}

MoniterImpl::MoniterParameterDescribe MoniterImpl::GetMoniterParameterDescribe()
{
	return m_MoniterParameterDescribe;
}

bool MoniterImpl::Start(string Address, int iInterval)
{
	m_bIsNormalStatistics=false;
	m_bIsWarningAnalysis = false;
	m_Address = Address;
	m_Interval = iInterval;
	if (m_WinThreadImpl.IsStart() == false)
	{
		m_WinThreadImpl.Start();
	}

	m_WarningAnalysisTimerEventID=SetTimer(NULL, NULL, m_MoniterParameterDescribe.WarningTimeoutSec*1000, (TIMERPROC)this);
	m_NormalStatisticsTimerEventID=SetTimer(NULL, NULL, m_MoniterParameterDescribe.StatisticsInterval*1000, (TIMERPROC)this);

	return true;
}

void MoniterImpl::Stop()
{
	m_bIsNormalStatistics = false;
	m_bIsWarningAnalysis = false;
	m_WinThreadImpl.Terminate();
}

void MoniterImpl::MoniterStatisticsTimer()
{
	m_bIsNormalStatistics = true;
	NormalStatistics();
	m_bIsNormalStatistics = false;
}

void MoniterImpl::MoniterWarningAnalysisTimer()
{
	m_bIsWarningAnalysis = true;
	WarningAnalysis();
	m_bIsWarningAnalysis = false;
}

void __stdcall MoniterImpl::MoniterStatisticsTimer(HWND hWnd, UINT uMsg, UINT_PTR idTimer, DWORD dwTime)
{
	MoniterImpl *pMoniterImpl = (MoniterImpl *)idTimer;
	pMoniterImpl->m_bIsWarningAnalysis = true;
	pMoniterImpl->NormalStatistics();
	pMoniterImpl->m_bIsWarningAnalysis = false;
}

void __stdcall MoniterImpl::MoniterWarningAnalysisTimer(HWND hWnd, UINT uMsg, UINT_PTR idTimer, DWORD dwTime)
{
	MoniterImpl *pMoniterImpl = (MoniterImpl *)idTimer;
	pMoniterImpl->m_bIsNormalStatistics = true;
	pMoniterImpl->WarningAnalysis();
	pMoniterImpl->m_bIsNormalStatistics = false;
}

bool MoniterImpl::PingResult(string &Address, int iInterval)
{
	IN_ADDR InAddr;
	LPHOSTENT pHostEnt;

	InAddr.s_addr = inet_addr(Address.c_str());
	uint32_t IPaddress = INADDR_NONE;
	if (InAddr.s_addr == INADDR_NONE)
	{
		pHostEnt = gethostbyname(Address.c_str());
		memcpy(&IPaddress, pHostEnt->h_addr_list, pHostEnt->h_length);
	}
	else
	{
		IPaddress = InAddr.s_addr;
	}

	HANDLE hICMPHandle;

	DWORD dwRetVal = 0;

	char SendData[32] = { 0 };

	DWORD ReplySize = 0;

	hICMPHandle = IcmpCreateFile();

	if (hICMPHandle == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData);

	char szReplyBuffer[sizeof(ICMP_ECHO_REPLY) + sizeof(SendData)] = { 0 };

	dwRetVal = IcmpSendEcho(hICMPHandle, IPaddress, SendData, sizeof(SendData), NULL, szReplyBuffer, ReplySize, iInterval);

	int TTL = 0;
	int Delay = -1;
	string ErrorString = "";
	int ErrorCode = 0;

	PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)szReplyBuffer;

	switch (pEchoReply->Status)
	{
	case IP_SUCCESS:
	{
		Delay = (int)pEchoReply->RoundTripTime;
		TTL = (int)pEchoReply->Options.Ttl;
		printf("PING RESULT:[Address:%s]\t[Delay:%d]\t[TTL:%d]\n", m_Address.c_str(), (int)pEchoReply->RoundTripTime, (int)pEchoReply->Options.Ttl);
	}
	break;
	case IP_BUF_TOO_SMALL:
	{
		ErrorString = "�𸴻�����̫С";
		printf("PING RESULT:[Address:%s]\t[ErrorCode:%d]\t[ErrorString:�𸴻�����̫С]\n", m_Address.c_str(), pEchoReply->Status);
	}
	break;
	case IP_DEST_NET_UNREACHABLE:
	{
		ErrorString = "Ŀ�����粻�ɴ�";
		printf("[Address:%s]\t[ErrorCode:%d]\t[ErrorString:Ŀ�����粻�ɴ�]\n", m_Address.c_str(), pEchoReply->Status);
	}
	break;
	case IP_DEST_HOST_UNREACHABLE:
	{
		ErrorString = "Ŀ���������ɴ�";
		printf("[Address:%s]\t[ErrorCode:%d]\t[ErrorString:Ŀ���������ɴ�]\n", m_Address.c_str(), pEchoReply->Status);
	}
	break;
	case IP_DEST_PROT_UNREACHABLE:
	{
		ErrorString = "Ŀ�ĵص�Э����ң���ɼ�";
		printf("[Address:%s]\t[ErrorCode:%d]\t[ErrorString:Ŀ�ĵص�Э����ң���ɼ�]\n", m_Address.c_str(), pEchoReply->Status);
	}
	break;
	case IP_DEST_PORT_UNREACHABLE:
	{
		ErrorString = "Ŀ��˿ڲ��ɴ�";
		printf("[Address:%s]\t[ErrorCode:%d]\t[ErrorString:Ŀ��˿ڲ��ɴ�]\n", m_Address.c_str(), pEchoReply->Status);
	}
	break;
	case IP_NO_RESOURCES:
	{
		ErrorString = "IP��Դ�����ǿ��õ�";
		printf("[Address:%s]\t[ErrorCode:%d]\t[ErrorString:IP��Դ�����ǿ��õ�]\n", m_Address.c_str(), pEchoReply->Status);
	}
	break;
	case IP_BAD_OPTION:
	{
		ErrorString = "ָ���˴����IPѡ��";
		printf("[Address:%s]\t[ErrorCode:%d]\t[ErrorString:ָ���˴����IPѡ��]\n", m_Address.c_str(), pEchoReply->Status);
	}
	break;
	case IP_HW_ERROR:
	{
		ErrorString = "һ��Ӳ������";
		printf("[Address:%s]\t[ErrorCode:%d]\t[ErrorString:һ��Ӳ������]\n", m_Address.c_str(), pEchoReply->Status);
	}
	break;
	case IP_PACKET_TOO_BIG:
	{
		ErrorString = "��̫��";
		printf("[Address:%s]\t[ErrorCode:%d]\t[ErrorString:��̫��]\n", m_Address.c_str(), pEchoReply->Status);
	}
	break;
	case IP_REQ_TIMED_OUT:
	{
		ErrorString = "����ʱ";
		printf("[Address:%s]\t[ErrorCode:%d]\t[ErrorString:����ʱ]\n", m_Address.c_str(), pEchoReply->Status);
	}
	break;
	case IP_BAD_REQ:
	{
		ErrorString = "һ����������";
		printf("[Address:%s]\t[ErrorCode:%d]\t[ErrorString:һ����������]\n", m_Address.c_str(), pEchoReply->Status);
	}
	break;
	case IP_BAD_ROUTE:
	{
		ErrorString = "һ������·��";
		printf("[Address:%s]\t[ErrorCode:%d]\t[ErrorString:һ������·��]\n", m_Address.c_str(), pEchoReply->Status);
	}
	break;
	case IP_TTL_EXPIRED_TRANSIT:
	{
		ErrorString = "�ڴ�������е�����ʱ�䣨TTL���Ĺ���";
		printf("[Address:%s]\t[ErrorCode:%d]\t[ErrorString:�ڴ�������е�����ʱ�䣨TTL���Ĺ���]\n", m_Address.c_str(), pEchoReply->Status);
	}
	break;
	case IP_TTL_EXPIRED_REASSEM:
	{
		ErrorString = "����Ƭ��������е�����ʱ�����";
		printf("[Address:%s]\t[ErrorCode:%d]\t[ErrorString:����Ƭ��������е�����ʱ�����]\n", m_Address.c_str(), pEchoReply->Status);
	}
	break;
	case IP_PARAM_PROBLEM:
	{
		ErrorString = "һ������������";
		printf("[Address:%s]\t[ErrorCode:%d]\t[ErrorString:һ������������]\n", m_Address.c_str(), pEchoReply->Status);
	}
	break;
	case IP_SOURCE_QUENCH:
	{
		ErrorString = "���ݱ�����̫�죬��������ݱ����ܱ�����";
		printf("[Address:%s]\t[ErrorCode:%d]\t[ErrorString:���ݱ�����̫�죬��������ݱ����ܱ�����]\n", m_Address.c_str(), pEchoReply->Status);
	}
	break;
	case IP_OPTION_TOO_BIG:
	{
		ErrorString = "һ��IPѡ����̫����";
		printf("[Address:%s]\t[ErrorCode:%d]\t[ErrorString:һ��IPѡ����̫����]\n", m_Address.c_str(), pEchoReply->Status);
	}
	break;
	case IP_BAD_DESTINATION:
	{
		ErrorString = "һ������Ŀ�ĵ�";
		printf("[Address:%s]\t[ErrorCode:%d]\t[ErrorString:һ������Ŀ�ĵ�]\n", m_Address.c_str(), pEchoReply->Status);
	}
	break;
	case IP_GENERAL_FAILURE:
	{
		ErrorString = "һ�����";
		printf("[Address:%s]\t[ErrorCode:%d]\t[ErrorString:һ�����]\n", m_Address.c_str(), pEchoReply->Status);
	}
	break;
	default:
	{
		ErrorString = "δ֪����";
		printf("[Address:%s]\t[ErrorCode:%d]\t[ErrorString:δ֪����]\n", m_Address.c_str(), pEchoReply->Status);
	}
	break;
	}

	ErrorCode = pEchoReply->Status;

	time_t LocalTime = time(NULL);
	struct tm *ptr = localtime(&LocalTime);
	char RecordDate[256] = { 0 };
	if (ptr)
	{
		strftime(RecordDate, sizeof(RecordDate), "%Y-%m-%d %H:%M:%S", ptr);
	}

	ResultInfoDescribe ObjResultInfoDescribe;
	ObjResultInfoDescribe.Address = m_Address;
	ObjResultInfoDescribe.Delay = Delay;
	ObjResultInfoDescribe.TTL = TTL;
	ObjResultInfoDescribe.ErrorCode = ErrorCode;
	ObjResultInfoDescribe.ErrorString = ErrorString;
	ObjResultInfoDescribe.ResultDate = RecordDate;

	ResultSaveDB(ObjResultInfoDescribe);

	if (m_bIsNormalStatistics == false)
	{
		m_vNormalStatisticsResultInfo.push_back(ObjResultInfoDescribe);
	}

	if (m_bIsWarningAnalysis == false)
	{
		m_vWarningAnalysisResultInfo.push_back(ObjResultInfoDescribe);
	}

	IcmpCloseHandle(hICMPHandle);
	if (ObjResultInfoDescribe.ErrorCode != IP_REQ_TIMED_OUT)
	{
		Sleep(m_Interval);
	}
	return true;
}

bool MoniterImpl::ThreadProcess(void *pUserObject)
{
	PingResult(m_Address, m_Interval);
	return true;
}

bool MoniterImpl::WarningAnalysis()
{
	/*
	1.һ��������ƽ���ӳٳ���200
	2.һ������������ӳٳ���40%
	3.һ������������ʱ���������󳬹�10%
	*/

	int NormalDelayCount = 0;

	int MaxDelay = -1;

	int MinDelay = -1;

	int ValidDelayCount = 0;

	int ValidDelaySum = 0;

	int AverageDelay;

	int InvalidCount = 0;

	for (int i = 0; i < m_vWarningAnalysisResultInfo.size(); i++)
	{
		if (m_vWarningAnalysisResultInfo[i].Delay != -1)
		{
			if (MinDelay == -1)
				MinDelay = m_vWarningAnalysisResultInfo[i].Delay;
			if(MaxDelay == -1)
				MaxDelay = m_vWarningAnalysisResultInfo[i].Delay;
			if (m_vWarningAnalysisResultInfo[i].Delay > MaxDelay)
			{
				MaxDelay = m_vWarningAnalysisResultInfo[i].Delay;
			}

			if (m_vWarningAnalysisResultInfo[i].Delay < MinDelay)
			{
				MinDelay = m_vWarningAnalysisResultInfo[i].Delay;
			}

			if (m_vWarningAnalysisResultInfo[i].Delay >= m_MoniterParameterDescribe.MaxDelay)
				NormalDelayCount++;

			ValidDelayCount++;

			ValidDelaySum += m_vWarningAnalysisResultInfo[i].Delay;
		}
		else
		{
			InvalidCount++;
		}
	}

	if (ValidDelaySum == 0 || ValidDelayCount == 0)
		AverageDelay = 0;
	else
		AverageDelay = (int)(ValidDelaySum / ValidDelayCount);

	int NormalDelayPercent = 0;

	if (m_vWarningAnalysisResultInfo.size()==0 || NormalDelayCount==0)
		NormalDelayPercent = 0;
	else
		NormalDelayPercent = NormalDelayCount / m_vWarningAnalysisResultInfo.size() * 100;
	
	int InvalidPercent = 0;
	if (m_vWarningAnalysisResultInfo.size() == 0 || InvalidCount == 0)
		InvalidPercent = 0;
	else
		InvalidPercent = InvalidCount / m_vWarningAnalysisResultInfo.size() * 100;

	if ((AverageDelay >= m_MoniterParameterDescribe.MaxDelay) ||
		NormalDelayPercent >= m_MoniterParameterDescribe.MaxDelayPercent ||
		InvalidPercent >= m_MoniterParameterDescribe.MaxTimeoutPercent)
	{
		HttpRequestClientImpl::HttpRequestParameterDescribe ObjHttpRequestParameterDescribe;
		ObjHttpRequestParameterDescribe.eRequestMode = HttpRequestClientImpl::RequestMode::REQUEST_MODE_POST;
		ObjHttpRequestParameterDescribe.RequestTimeout = 20000;
		ObjHttpRequestParameterDescribe.szRequestUrl = m_MoniterParameterDescribe.WebHookURL;

		char Buffer[4096 * 2] = { 0 };
		sprintf(Buffer, "IP��ַ[%s]%d���쳣����ͳ����Ϣ\n\
					 ����ӳ�:%d\n\
					 ��С�ӳ�:%d\n\
					 ƽ���ӳ�:%d\n\
					 ��ͳ�ƴ���:%d\n\
					 ͳ��Ƶ��:%d\n\
					 �����쳣����:%d\n\
					 ����ӳ���:%d%%\n\
					 �����쳣��:%d%%\n", m_Address.c_str(), m_MoniterParameterDescribe.WarningTimeoutSec, MaxDelay, MinDelay, AverageDelay, m_vNormalStatisticsResultInfo.size(), m_Interval / 1000, InvalidCount, NormalDelayPercent, InvalidPercent);
		Json::FastWriter JsonWrite;
		Json::Value JsonValue;
		JsonValue["msgtype"] = "text";
		JsonValue["text"]["content"] = Buffer;
		JsonValue["at"]["isAtAll"] = true;

		ObjHttpRequestParameterDescribe.szPostData = UtilsImpl::ASCII_TO_UTF8(JsonWrite.write(JsonValue).c_str()).c_str();

		m_WarningAnalysisHttpRequestClientImpl.AsyncRequest(&ObjHttpRequestParameterDescribe);
	}

	m_vWarningAnalysisResultInfo.clear();
	return true;
}

bool MoniterImpl::WarningPush()
{
	HttpRequestClientImpl::HttpRequestParameterDescribe ObjHttpRequestParameterDescribe;
	ObjHttpRequestParameterDescribe.eRequestMode = HttpRequestClientImpl::RequestMode::REQUEST_MODE_POST;
	ObjHttpRequestParameterDescribe.RequestTimeout = 20000;
	ObjHttpRequestParameterDescribe.szRequestUrl = m_MoniterParameterDescribe.WebHookURL;

	char Buffer[1024] = { 0 };

	//sprintf(Buffer, "Site1:%s �ӳ�:%d", m_Address.c_str(), (int)pEchoReply->RoundTripTime);

	Json::FastWriter JsonWrite;
	Json::Value JsonValue;
	JsonValue["msgtype"] = "text";
	JsonValue["text"]["content"] = Buffer;
	JsonValue["at"]["isAtAll"] = true;

	ObjHttpRequestParameterDescribe.szPostData = UtilsImpl::ASCII_TO_UTF8(JsonWrite.write(JsonValue).c_str()).c_str();
	m_WarningAnalysisHttpRequestClientImpl.AsyncRequest(&ObjHttpRequestParameterDescribe);
	return true;
}

bool MoniterImpl::NormalStatistics()
{
	int MaxDelay= -1;

	int MinDelay= -1;

	int ValidDelayCount = 0;

	int ValidDelaySum = 0;

	int AverageDelay;

	int InvalidCount = 0;

	for (int i = 0; i < m_vNormalStatisticsResultInfo.size(); i++)
	{
		if (m_vNormalStatisticsResultInfo[i].Delay != -1)
		{
			if (MinDelay == -1)
				MinDelay = m_vNormalStatisticsResultInfo[i].Delay;
			if (MaxDelay == -1)
				MaxDelay = m_vNormalStatisticsResultInfo[i].Delay;
			if (m_vNormalStatisticsResultInfo[i].Delay > MaxDelay)
			{
				MaxDelay = m_vNormalStatisticsResultInfo[i].Delay;
			}

			if (m_vNormalStatisticsResultInfo[i].Delay < MinDelay)
			{
				MinDelay = m_vNormalStatisticsResultInfo[i].Delay;
			}

			ValidDelayCount++;

			ValidDelaySum += m_vNormalStatisticsResultInfo[i].Delay;
		}
		else
		{
			InvalidCount++;
		}
	}

	if (ValidDelaySum == 0 || ValidDelayCount == 0)
		AverageDelay = 0;
	else
		AverageDelay = (int)(ValidDelaySum / ValidDelayCount);
	
	HttpRequestClientImpl::HttpRequestParameterDescribe ObjHttpRequestParameterDescribe;
	ObjHttpRequestParameterDescribe.eRequestMode = HttpRequestClientImpl::RequestMode::REQUEST_MODE_POST;
	ObjHttpRequestParameterDescribe.RequestTimeout = 20000;
	ObjHttpRequestParameterDescribe.szRequestUrl = m_MoniterParameterDescribe.WebHookURL;

	char Buffer[4096*2] = { 0 };
	sprintf(Buffer, "IP��ַ[%s]%d�볣������ͳ����Ϣ\n\
					 ����ӳ�:%d\n\
					 ��С�ӳ�:%d\n\
					 ƽ���ӳ�:%d\n\
					 ��ͳ�ƴ���:%d\n\
					 ͳ��Ƶ��:%d\n\
					 �����쳣����:%d\n",m_Address.c_str(),m_MoniterParameterDescribe.StatisticsInterval, MaxDelay, MinDelay, AverageDelay, m_vNormalStatisticsResultInfo.size(),m_Interval/1000, InvalidCount);
	Json::FastWriter JsonWrite;
	Json::Value JsonValue;
	JsonValue["msgtype"] = "text";
	JsonValue["text"]["content"] = Buffer;
	JsonValue["at"]["isAtAll"] = false;

	ObjHttpRequestParameterDescribe.szPostData = UtilsImpl::ASCII_TO_UTF8(JsonWrite.write(JsonValue).c_str()).c_str();

	m_NormalStatisticsHttpRequestClientImpl.AsyncRequest(&ObjHttpRequestParameterDescribe);

	m_vNormalStatisticsResultInfo.clear();
	return true;
}

bool MoniterImpl::ResultSaveDB(ResultInfoDescribe ObjResultInfoDescribe)
{
	
	string ResultDBPath = ROOT_PATH;
	ResultDBPath += RESULTDB_DIRECTORY;

	if (UtilsImpl::IsFilePathValid(ResultDBPath.c_str()) == false)
	{
		if (UtilsImpl::CreateFileDirectory(ResultDBPath.c_str()) == false)
			return false;
	}

	time_t LocalTime = time(NULL);
	struct tm *ptr = localtime(&LocalTime);
	if (ptr == NULL)
		return false;
	char ResultDBFileName[256] = { 0 };
	strftime(ResultDBFileName, sizeof(ResultDBFileName), "%Y.%m.%d.db", ptr);
	string ResultDBFilePath = ResultDBPath;
	ResultDBFilePath += "/";
	ResultDBFilePath += ResultDBFileName;

	FILE *fp = fopen(ResultDBFilePath.c_str(), "r");
	if (fp == NULL)
	{
		fp = fopen(ResultDBFilePath.c_str(), "wb+");
		if (fp == NULL)
		{
			return false;
		}
	}
	fclose(fp);

	SqliteWrapperDB db;
	db.Open(ResultDBFilePath.c_str());

	if (!db.TableExists("PingResultInfo"))
	{
		if (db.ExecDML("CREATE TABLE PingResultInfo(ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,Address TEXT,Delay TEXT,RecordDate TEXT,TTL TEXT,ErrorCode TEXT,ErrorString TEXT);") == -1)
		{
			db.Close();
			return false;
		}
	}

	

	char InsertString[1024] = { 0 };
	sprintf(InsertString, "INSERT INTO PingResultInfo(Address,Delay,RecordDate,TTL,ErrorCode,ErrorString) VALUES('%s','%d','%s','%d','%d','%s');", ObjResultInfoDescribe.Address.c_str(), ObjResultInfoDescribe.Delay, ObjResultInfoDescribe.ResultDate.c_str(), ObjResultInfoDescribe.TTL, ObjResultInfoDescribe.ErrorCode, ObjResultInfoDescribe.ErrorString.c_str());
	if (db.ExecDML(InsertString) == -1)
	{
		db.Close();
		return false;
	}
	db.Close();
	return true;
}