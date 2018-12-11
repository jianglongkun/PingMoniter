#pragma once
#ifndef __HTTPREQUESTCLIENTIMPL__H_
#define __HTTPREQUESTCLIENTIMPL__H_

#include "UtilsImpl.h"
#include "ThreadImpl.h"
#include <winhttp.h>

using namespace std;

#pragma comment(lib,"winhttp.lib")

class HttpRequestClientDelegate
{
public:
	virtual void HttpRequestFailed(string szErrorString, string szRequestUrl, uint32_t uErrorID)=0;
	virtual void HttpRequestSucceed(string szRequestUrl,string szResponseData)=0;
};

class HttpRequestClientImpl:public ThreadDelegate
{
public:
	enum RequestMode
	{
		REQUEST_MODE_POST,
		REQUEST_MODE_GET,
	};

	typedef struct HttpRequestParameterDescribe
	{
		string szRequestUrl;
		string szPostData;
		uint32_t RequestTimeout;
		RequestMode eRequestMode;
		HttpRequestParameterDescribe() :RequestTimeout(20000), eRequestMode(RequestMode::REQUEST_MODE_GET){}
	};
public:
	HttpRequestClientImpl();

	~HttpRequestClientImpl();

	void SetHttpRequestClientDelegate(HttpRequestClientDelegate *pHttpRequestClientDelegate, void *pUserObject=NULL);

	bool SyncRequest(HttpRequestParameterDescribe *pHttpRequestParameterDescribe,string &szResponseData,string &szErrorString, uint32_t &uErrorID);

	bool AsyncRequest(HttpRequestParameterDescribe *pHttpRequestParameterDescribe);

	bool AsyncRequest(HttpRequestParameterDescribe *pHttpRequestParameterDescribe, HttpRequestClientDelegate *pHttpRequestClientDelegate, void *pUserObject=NULL);
protected:
	virtual bool ThreadProcess(void *pUserObject);
private:
	HttpRequestClientDelegate *m_pHttpRequestClientDelegate;
	void *m_pUserObject;
	HttpRequestParameterDescribe m_HttpRequestParameterDescribe;
	WinThreadImpl m_ThreadImpl;
};

#endif