#pragma once
#ifndef __THREADIMPL__H_
#define __THREADIMPL__H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <windows.h>
#include <process.h>

class ThreadDelegate
{
public:
	virtual bool ThreadProcess(void *pUserObject) = 0;
};

class WinThreadImpl
{
public:
	WinThreadImpl();

	~WinThreadImpl();

	void SetThreadDelegate(ThreadDelegate *pThreadDelegate,void *pUserObject=NULL);

	bool Start();

	bool Stop();

	bool IsStop();

	bool IsStart();

	void Terminate();

private:
	static uint32_t __stdcall ThreadProcessFunction(void *lParam);
private:
	ThreadDelegate *m_ThreadDelegate;

	HANDLE m_ThreadHandle;

	uint32_t m_ThreadID;

	bool m_bIsRuning;

	void *m_pUserObject;
};

#endif