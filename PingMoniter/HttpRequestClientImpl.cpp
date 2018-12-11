#include "HttpRequestClientImpl.h"

HttpRequestClientImpl::HttpRequestClientImpl()
{
	m_pHttpRequestClientDelegate = NULL;
	m_pUserObject = NULL;
}

HttpRequestClientImpl::~HttpRequestClientImpl()
{
	m_pHttpRequestClientDelegate = NULL;
	m_pUserObject = NULL;	
}

void HttpRequestClientImpl::SetHttpRequestClientDelegate(HttpRequestClientDelegate *pHttpRequestClientDelegate, void *pUserObject)
{
	m_pHttpRequestClientDelegate = pHttpRequestClientDelegate;
	m_pUserObject = pUserObject;
}

bool HttpRequestClientImpl::SyncRequest(HttpRequestParameterDescribe *pHttpRequestParameterDescribe, string &szResponseData, string &szErrorString, uint32_t &uErrorID)
{
	if (pHttpRequestParameterDescribe == NULL)
	{
		uErrorID = 87;
		szErrorString = "参数错误:pHttpRequestParameterDescribe=NULL";
		return false;
	}
	m_HttpRequestParameterDescribe = *pHttpRequestParameterDescribe;

	HINTERNET hSession = WinHttpOpen(0, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, NULL);

	char szErrorBuffer[1024] = { 0 };

	if (hSession == NULL)
	{
		uErrorID = GetLastError();
		switch (uErrorID)
		{
			case ERROR_WINHTTP_INTERNAL_ERROR:
				szErrorString += "An internal error has occurred";
				break;
			case ERROR_NOT_ENOUGH_MEMORY:
				szErrorString += "Not enough memory was available to complete the requested operation";
				break;
			default:
			{
				FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, uErrorID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), szErrorBuffer, 0, 0);
				szErrorString += szErrorBuffer;
			}
				break;
		}
		return false;
	}

	bool bHttpResult = WinHttpSetOption(hSession, WINHTTP_OPTION_CONNECT_TIMEOUT, &m_HttpRequestParameterDescribe.RequestTimeout, sizeof(m_HttpRequestParameterDescribe.RequestTimeout));

	WCHAR szHost[256] = { 0 };
	unsigned long ulOpenRequesetFlag = 0;
	URL_COMPONENTS urlComp;

	memset(&urlComp, 0, sizeof(URL_COMPONENTS));
	urlComp.dwStructSize = sizeof(URL_COMPONENTS);
	urlComp.lpszHostName = szHost;
	urlComp.dwHostNameLength = sizeof(szHost) / sizeof(szHost[0]);
	urlComp.dwUrlPathLength = -1;
	urlComp.dwSchemeLength = -1;

	wstring URL = UtilsImpl::ASCII_TO_UNICODE(m_HttpRequestParameterDescribe.szRequestUrl.c_str()).c_str();
	if (!WinHttpCrackUrl(URL.c_str(), 0, 0, &urlComp))
	{
		uErrorID = GetLastError();
		switch (uErrorID)
		{
			case ERROR_WINHTTP_INTERNAL_ERROR:
				szErrorString += "An internal error has occurred";
				break;
			case ERROR_WINHTTP_INVALID_URL:
				szErrorString += "The URL is invalid";
				break;
			case ERROR_WINHTTP_UNRECOGNIZED_SCHEME:
				szErrorString += "The URL scheme could not be recognized, or is not supported";
				break;
			case ERROR_NOT_ENOUGH_MEMORY:
				szErrorString += "Not enough memory was available to complete the requested operation";
				break;
			default:
			{
				FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, uErrorID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), szErrorBuffer, 0, 0);
				szErrorString += szErrorBuffer;
			}
			break;
		}

		if (hSession)
		{
			WinHttpCloseHandle(hSession);
			hSession = NULL;
		}
		return false;
	}

	HINTERNET hConnect = WinHttpConnect(hSession, szHost, urlComp.nPort, 0);
	if (hConnect == NULL)
	{
		uErrorID = GetLastError();

		switch (uErrorID)
		{
			case ERROR_WINHTTP_INCORRECT_HANDLE_TYPE:
				szErrorString += "The type of handle supplied is incorrect for this operation";
				break;
			case ERROR_WINHTTP_INTERNAL_ERROR:
				szErrorString += "An internal error has occurred";
				break;
			case ERROR_WINHTTP_INVALID_URL:
				szErrorString += "The URL is invalid";
				break;
			case ERROR_WINHTTP_OPERATION_CANCELLED:
				szErrorString += "The operation was canceled, usually because the handle on which the request was operating was closed before the operation completed";
				break;
			case ERROR_WINHTTP_UNRECOGNIZED_SCHEME:
				szErrorString += "The URL scheme could not be recognized, or is not supported";
				break;
			case ERROR_WINHTTP_SHUTDOWN:
				szErrorString += "The WinHTTP function support is being shut down or unloaded";
				break;
			case ERROR_NOT_ENOUGH_MEMORY:
				szErrorString += "Not enough memory was available to complete the requested operation";
				break;
			default:
			{
				FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, uErrorID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), szErrorBuffer, 0, 0);
				szErrorString += szErrorBuffer;
			}
			break;
		}

		if (hSession)
		{
			WinHttpCloseHandle(hSession);
			hSession = NULL;
		}
		return false;
	}

	ulOpenRequesetFlag = (INTERNET_SCHEME_HTTPS == urlComp.nScheme) ? WINHTTP_FLAG_SECURE : 0;

	HINTERNET hRequest = WinHttpOpenRequest(hConnect, m_HttpRequestParameterDescribe.eRequestMode == RequestMode::REQUEST_MODE_GET ? L"GET" : L"POST", urlComp.lpszUrlPath, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, ulOpenRequesetFlag);
	if (hRequest == NULL)
	{
		uErrorID = GetLastError();
		switch (uErrorID)
		{
			case ERROR_WINHTTP_INCORRECT_HANDLE_TYPE:
				szErrorString += "The type of handle supplied is incorrect for this operation";
				break;
			case ERROR_WINHTTP_INTERNAL_ERROR:
				szErrorString += "An internal error has occurred";
				break;
			case ERROR_WINHTTP_INVALID_URL:
				szErrorString += "The URL is invalid";
				break;
			case ERROR_WINHTTP_OPERATION_CANCELLED:
				szErrorString += "The operation was canceled, usually because the handle on which the request was operating was closed before the operation completed";
				break;
			case ERROR_WINHTTP_UNRECOGNIZED_SCHEME:
				szErrorString += "The URL specified a scheme other than http: or https:";
				break;
			case ERROR_NOT_ENOUGH_MEMORY:
				szErrorString += "Not enough memory was available to complete the requested operation";
				break;
			default:
			{
				FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, uErrorID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), szErrorBuffer, 0, 0);
				szErrorString += szErrorBuffer;
			}
			break;
		}

		if (hSession)
		{
			WinHttpCloseHandle(hSession);
			hSession = NULL;
		}

		if (hConnect)
		{
			WinHttpCloseHandle(hConnect);
			hConnect = NULL;
		}

		return false;
	}

	bool bSendRequest = false;

	if (m_HttpRequestParameterDescribe.eRequestMode == RequestMode::REQUEST_MODE_GET)
	{
		bSendRequest = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, NULL, 0, 0, 0);
	}
	else
	{
#if 0
		TCHAR *pos = _tcsstr(urlComp.lpszUrlPath, _T("?"));
		if (pos == NULL)
		{
			uErrorID = 8888;
			szErrorString = "不是有效的请求URL";
			if (hSession)
			{
				WinHttpCloseHandle(hSession);
				hSession = NULL;
			}

			if (hConnect)
			{
				WinHttpCloseHandle(hConnect);
				hConnect = NULL;
			}

			return false;
		}

		TSTRING SendRequestData = pos + 1;
		string Data = UtilsImpl::UNICODE_TO_UTF8(SendRequestData).c_str();
#if 0
		bSendRequest = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, (char *)SendRequestData.c_str(), SendRequestData.length(), SendRequestData.length(), 0);
#else
		bSendRequest = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, (char *)Data.c_str(), Data.length(), Data.length(), 0);
#endif
#else
		//bSendRequest = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, (char *)m_HttpRequestParameterDescribe.szPostData.c_str(), m_HttpRequestParameterDescribe.szPostData.length(), m_HttpRequestParameterDescribe.szPostData.length(), 0);
		bSendRequest = WinHttpSendRequest(hRequest, _T("Content-Type: application/json"), _tcslen(_T("Content-Type: application/json")), (char *)m_HttpRequestParameterDescribe.szPostData.c_str(), m_HttpRequestParameterDescribe.szPostData.length(), m_HttpRequestParameterDescribe.szPostData.length(), 0);
#endif
	}
	if(!bSendRequest)
	{
		uErrorID = GetLastError();
		switch (uErrorID)
		{
			case ERROR_WINHTTP_CANNOT_CONNECT:
				szErrorString += "Returned if connection to the server failed";
				break;
			case ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED:
				szErrorString += "The secure HTTP server requires a client certificate.\
                                                            The application retrieves the list of certificate issuers by calling WinHttpQueryOption with the WINHTTP_OPTION_CLIENT_CERT_ISSUER_LIST option.\
                                                            If the server requests the client certificate, but does not require it, the application can alternately call WinHttpSetOption with the WINHTTP_OPTION_CLIENT_CERT_CONTEXT option.\
                                                            In this case, the application specifies the WINHTTP_NO_CLIENT_CERT_CONTEXT macro in the lpBuffer parameter of WinHttpSetOption.\
                                                            For more information, see the WINHTTP_OPTION_CLIENT_CERT_CONTEXT option.\
                                                            Windows Server 2003 with SP1, Windows XP with SP2, and Windows 2000:This error is not supported";
				break;
			case ERROR_WINHTTP_CONNECTION_ERROR:
				szErrorString += "The connection with the server has been reset or terminated, or an incompatible SSL protocol was encountered. For example, WinHTTP version 5.1 does not support SSL2 unless the client specifically enables it";
				break;
			case ERROR_WINHTTP_INCORRECT_HANDLE_STATE:
				szErrorString += "The requested operation cannot be carried out because the handle supplied is not in the correct state";
				break;
			case ERROR_WINHTTP_INCORRECT_HANDLE_TYPE:
				szErrorString += "The type of handle supplied is incorrect for this operation";
				break;
			case ERROR_WINHTTP_INTERNAL_ERROR:
				szErrorString += "An internal error has occurred";
				break;
			case ERROR_WINHTTP_INVALID_URL:
				szErrorString += "The URL is invalid";
				break;
			case ERROR_WINHTTP_LOGIN_FAILURE:
				szErrorString += "The login attempt failed. When this error is encountered, the request handle should be closed with WinHttpCloseHandle. A new request handle must be created before retrying the function that originally produced this error";
				break;
			case ERROR_WINHTTP_NAME_NOT_RESOLVED:
				szErrorString += "The server name cannot be resolved";
				break;
			case ERROR_WINHTTP_OPERATION_CANCELLED:
				szErrorString += "The operation was canceled, usually because the handle on which the request was operating was closed before the operation completed";
				break;
			case ERROR_WINHTTP_RESPONSE_DRAIN_OVERFLOW:
				szErrorString += "Returned when an incoming response exceeds an internal WinHTTP size limit";
				break;
			case ERROR_WINHTTP_SECURE_FAILURE:
				szErrorString += "One or more errors were found in the Secure Sockets Layer (SSL) certificate sent by the server. To determine what type of error was encountered, verify through a WINHTTP_CALLBACK_STATUS_SECURE_FAILURE notification in a status callback function. For more information, see WINHTTP_STATUS_CALLBACK";
				break;
			case ERROR_WINHTTP_SHUTDOWN:
				szErrorString += "The WinHTTP function support is shut down or unloaded";
				break;
			case ERROR_WINHTTP_TIMEOUT:
				szErrorString += "The request timed out";
				break;
			case ERROR_WINHTTP_UNRECOGNIZED_SCHEME:
				szErrorString += "The URL specified a scheme other than http: or https:";
				break;
			case ERROR_NOT_ENOUGH_MEMORY:
				szErrorString += "Not enough memory was available to complete the requested operation. (Windows error code)Windows Server 2003, Windows XP, and Windows 2000 : The TCP reservation range set with the WINHTTP_OPTION_PORT_RESERVATION option is not large enough to send this request";
				break;
			case ERROR_INVALID_PARAMETER:
				szErrorString += "The content length specified in the dwTotalLength parameter does not match the length specified in the Content-Length header.\
                                                            The lpOptional parameter must be NULL and the dwOptionalLength parameter must be zero when the Transfer - Encoding header is present.\
                                                            The Content - Length header cannot be present when the Transfer - Encoding header is present";
				break;
			case ERROR_WINHTTP_RESEND_REQUEST:
				szErrorString += "The application must call WinHttpSendRequest again due to a redirect or authentication challenge.Windows Server 2003 with SP1, Windows XP with SP2, and Windows 2000 : This error is not supported";
				break;
			default:
			{
				FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, uErrorID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), szErrorBuffer, 0, 0);
				szErrorString += szErrorBuffer;
			}
			break;
		}

		if (hSession)
		{
			WinHttpCloseHandle(hSession);
			hSession = NULL;
		}

		if (hConnect)
		{
			WinHttpCloseHandle(hConnect);
			hConnect = NULL;
		}
		if (hRequest)
		{
			WinHttpCloseHandle(hRequest);
			hRequest = NULL;
		}
		
		return false;
	}

	bool bResult = WinHttpReceiveResponse(hRequest, 0);
	if (!bResult)
	{
		uErrorID = GetLastError();
		FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, uErrorID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), szErrorBuffer, 0, 0);
		szErrorString += szErrorBuffer;
		if (hSession)
		{
			WinHttpCloseHandle(hSession);
			hSession = NULL;
		}

		if (hConnect)
		{
			WinHttpCloseHandle(hConnect);
			hConnect = NULL;
		}
		if (hRequest)
		{
			WinHttpCloseHandle(hRequest);
			hRequest = NULL;
		}
		
		return false;
	}

	unsigned long ulSize = 0;
	unsigned long ulDownloaded = 0;
	string OutString = "";
	char *pOutBuffer = NULL;
	do
	{
		ulSize = 0;
		if (!WinHttpQueryDataAvailable(hRequest, &ulSize))
		{
			uErrorID = GetLastError();
			FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, uErrorID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), szErrorBuffer, 0, 0);
			szErrorString += szErrorBuffer;
			if (hSession)
			{
				WinHttpCloseHandle(hSession);
				hSession = NULL;
			}

			if (hConnect)
			{
				WinHttpCloseHandle(hConnect);
				hConnect = NULL;
			}
			if (hRequest)
			{
				WinHttpCloseHandle(hRequest);
				hRequest = NULL;
			}
			return false;
		}
		pOutBuffer = new char[ulSize + 1];
		if (!pOutBuffer)
		{
			uErrorID = GetLastError();
			szErrorString = "[pOutBuffer]:Out of memory";
			if (hSession)
			{
				WinHttpCloseHandle(hSession);
				hSession = NULL;
			}

			if (hConnect)
			{
				WinHttpCloseHandle(hConnect);
				hConnect = NULL;
			}
			if (hRequest)
			{
				WinHttpCloseHandle(hRequest);
				hRequest = NULL;
			}
			return false;
		}

		memset(pOutBuffer, 0, ulSize + 1);
		if (!WinHttpReadData(hRequest, pOutBuffer, ulSize, &ulDownloaded))
		{
			uErrorID = GetLastError();

			FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, uErrorID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), szErrorBuffer, 0, 0);
			szErrorString += szErrorBuffer;
			if (hSession)
			{
				WinHttpCloseHandle(hSession);
				hSession = NULL;
			}

			if (hConnect)
			{
				WinHttpCloseHandle(hConnect);
				hConnect = NULL;
			}
			if (hRequest)
			{
				WinHttpCloseHandle(hRequest);
				hRequest = NULL;
			}
			delete[]pOutBuffer;
			pOutBuffer = NULL;
			return false;
		}

		OutString += pOutBuffer;
		delete[]pOutBuffer;
		pOutBuffer = NULL;
	} while (ulSize > 0);

	if (hSession)
	{
		WinHttpCloseHandle(hSession);
		hSession = NULL;
	}

	if (hConnect)
	{
		WinHttpCloseHandle(hConnect);
		hConnect = NULL;
	}
	if (hRequest)
	{
		WinHttpCloseHandle(hRequest);
		hRequest = NULL;
	}
	if (pOutBuffer)
		delete[]pOutBuffer;
	
	szResponseData = OutString;
	return true;
}

bool HttpRequestClientImpl::AsyncRequest(HttpRequestParameterDescribe *pHttpRequestParameterDescribe)
{
	if (pHttpRequestParameterDescribe == NULL)
	{
		return false;
	}
	m_HttpRequestParameterDescribe = *pHttpRequestParameterDescribe;

	//if (m_pHttpRequestClientDelegate == NULL)
		//return false;

	if (m_ThreadImpl.IsStart() == true)
		return false;

	m_ThreadImpl.SetThreadDelegate(this);

	return m_ThreadImpl.Start();
}

bool HttpRequestClientImpl::AsyncRequest(HttpRequestParameterDescribe *pHttpRequestParameterDescribe, HttpRequestClientDelegate *pHttpRequestClientDelegate, void *pUserObject)
{
	if (pHttpRequestParameterDescribe == NULL)
	{
		return false;
	}

	m_HttpRequestParameterDescribe = *pHttpRequestParameterDescribe;

	m_pHttpRequestClientDelegate = pHttpRequestClientDelegate;

	m_pUserObject = pUserObject;

	if (m_ThreadImpl.IsStart() == true)
		return false;

	m_ThreadImpl.SetThreadDelegate(this);

	return m_ThreadImpl.Start();
}

bool HttpRequestClientImpl::ThreadProcess(void *pUserObject)
{

	string szResponseData="";
	string szRequestUrl="";
	string szErrorString="";
	uint32_t uErrorID=0;
	bool bRet = SyncRequest(&m_HttpRequestParameterDescribe, szResponseData, szErrorString, uErrorID);

	if (m_pHttpRequestClientDelegate == NULL)
		return false;

	if (bRet == false)
		m_pHttpRequestClientDelegate->HttpRequestFailed(szErrorString, m_HttpRequestParameterDescribe.szRequestUrl, uErrorID);
	else
		m_pHttpRequestClientDelegate->HttpRequestSucceed(m_HttpRequestParameterDescribe.szRequestUrl, szResponseData);

	return false;
}