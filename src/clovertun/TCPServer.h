#pragma once

#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#ifdef WIN32
#include <WinSock2.h>
#include <windows.h>
#else
#include <winpr/wtypes.h>
#endif

#include "TCPBase.h"

class CTCPServer : public CTCPBase
{
public :
	CTCPServer(SOCKET socket, WORD ListenPort);
	virtual ~CTCPServer();

    BOOL Init();
    VOID Done();
};

typedef BOOL(*_SocketAcceptProcess)(SOCKET s, CBaseObject* pParam);

class CTCPService : public CBaseObject
{
public:
    CTCPService(WORD ListenPort);
    ~CTCPService();

    BOOL Init();
    VOID Done();

    VOID RegisterSocketAcceptProcess(_SocketAcceptProcess Process, CBaseObject* pParam);

private:
    static DWORD WINAPI ListenProc(void* pParam);
    BOOL ListenProcess(HANDLE StopEvent);

    WORD   m_dwListenPort;
    SOCKET m_hListenSock;

    HANDLE m_hStopEvent;
    HANDLE m_hListenThread;

    CRITICAL_SECTION m_csAcceptFunc;
    _SocketAcceptProcess m_pfnAcceptFunc;
    CBaseObject* m_pAcceptParam;
};

#endif