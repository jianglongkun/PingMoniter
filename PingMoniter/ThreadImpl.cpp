#include "ThreadImpl.h"

WinThreadImpl::WinThreadImpl()
{
	m_ThreadDelegate = NULL;

	m_ThreadHandle = NULL;

	m_ThreadID = NULL;

	m_bIsRuning = false;

	m_pUserObject = NULL;
}

WinThreadImpl::~WinThreadImpl()
{
	Stop();
}

void WinThreadImpl::SetThreadDelegate(ThreadDelegate *pThreadDelegate, void *pUserObject)
{
	m_ThreadDelegate = pThreadDelegate;
	m_pUserObject = pUserObject;
}

bool WinThreadImpl::Start()
{
	Stop();
	m_bIsRuning = true;

	m_ThreadHandle = (HANDLE)_beginthreadex(NULL, NULL, ThreadProcessFunction, this, NULL, &m_ThreadID);

	if (m_ThreadHandle == NULL)
	{
		m_bIsRuning = false;
		return false;
	}
	return true;
}

bool WinThreadImpl::Stop()
{
	if (m_bIsRuning != true)
		return true;

	m_bIsRuning = false;

	if (m_ThreadHandle)
	{
		WaitForSingleObject(m_ThreadHandle, INFINITE);
		CloseHandle(m_ThreadHandle);
		m_ThreadHandle = NULL;
		m_ThreadID = 0;
	}
	return false;
}

bool WinThreadImpl::IsStop()
{
	return m_bIsRuning;
}

bool WinThreadImpl::IsStart()
{
	return m_bIsRuning;
}

void WinThreadImpl::Terminate()
{
	if (m_bIsRuning != true)
		return;

	if (m_ThreadHandle)
	{
		TerminateThread(m_ThreadHandle, 0);
		CloseHandle(m_ThreadHandle);
		m_ThreadHandle = NULL;
		m_ThreadID = 0;
	}
	m_bIsRuning = false;
}

uint32_t __stdcall WinThreadImpl::ThreadProcessFunction(void *lParam)
{
	WinThreadImpl *pWinThreadImpl = (WinThreadImpl *)lParam;
	while (pWinThreadImpl->m_bIsRuning)
	{
		if (pWinThreadImpl->m_ThreadDelegate)
		{
			if (pWinThreadImpl->m_ThreadDelegate->ThreadProcess(pWinThreadImpl->m_pUserObject) == false)
			{
				break;
			}
		}
		Sleep(1);
	}
	pWinThreadImpl->m_bIsRuning = false;
	CloseHandle(pWinThreadImpl->m_ThreadHandle);
	pWinThreadImpl->m_ThreadHandle = NULL;
	pWinThreadImpl->m_ThreadID = 0;
	return 0;
}