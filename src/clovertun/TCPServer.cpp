#include "stdafx.h"
#include "TCPServer.h"

CTCPServer::CTCPServer(SOCKET socket, WORD ListenPort) : CTCPBase()
{
    strcpy(m_szSrcAddress, "127.0.0.1");
    m_dwSrcPort = ListenPort;
    m_hSock = socket;
}

CTCPServer::~CTCPServer()
{

}

BOOL CTCPServer::Init()
{
    struct sockaddr_in  SrcAddress;

#ifdef WIN32    
    int len = sizeof(struct sockaddr_in);
#else
    socklen_t len = sizeof(struct sockaddr_in);
#endif    
    if (getpeername(m_hSock, (struct sockaddr*)&SrcAddress, &len) >= 0)
    {
        m_dwDstPort = SrcAddress.sin_port;
        strcpy(m_szDstAddress, inet_ntoa(SrcAddress.sin_addr));
    }

    return InitBase();
}

VOID CTCPServer::Done()
{
    DoneBase();

    if (m_hSock != INVALID_SOCKET)
    {
        shutdown(m_hSock, SD_BOTH);
        closesocket(m_hSock);
        m_hSock = INVALID_SOCKET;
    }
}

CTCPService::CTCPService(WORD ListenPort)
{
    m_dwListenPort = ListenPort;
    m_hListenSock = INVALID_SOCKET;
    
    m_hListenThread = NULL;

    m_hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    m_pfnAcceptFunc = NULL;
    m_pAcceptParam = NULL;

    InitializeCriticalSection(&m_csAcceptFunc);
}

CTCPService::~CTCPService()
{
    if (m_hStopEvent)
    {
        CloseHandle(m_hStopEvent);
    }

    DeleteCriticalSection(&m_csAcceptFunc);
}

BOOL CTCPService::Init()
{
    int Ret;
    int flag;
    struct sockaddr_in sockAddr;
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock == INVALID_SOCKET)
    {
        return FALSE;
    }

    flag = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag, sizeof(int));

    sockAddr.sin_addr.s_addr = INADDR_ANY;
    sockAddr.sin_port = htons(m_dwListenPort);
    sockAddr.sin_family = AF_INET;
    Ret = bind(sock, (struct sockaddr *)&sockAddr, sizeof(sockAddr));

    if (Ret < 0)
    {
        closesocket(sock);
        return FALSE;
    }

    listen(sock, 0x10);

    m_hListenSock = sock;

    AddRef();
    m_hListenThread = CreateThread(NULL, 0, CTCPService::ListenProc, this, 0, NULL);

    return TRUE;
}

VOID CTCPService::Done()
{
    RegisterSocketAcceptProcess(NULL, NULL);

    if (m_hListenSock != INVALID_SOCKET)
    {
        shutdown(m_hListenSock, SD_BOTH);
        closesocket(m_hListenSock);
        m_hListenSock = INVALID_SOCKET;
    }

    SetEvent(m_hStopEvent);
}

VOID CTCPService::RegisterSocketAcceptProcess(_SocketAcceptProcess Process, CBaseObject* pParam)
{
    CBaseObject* pOldParam = NULL;

    EnterCriticalSection(&m_csAcceptFunc);
    m_pfnAcceptFunc = Process;

    pOldParam = m_pAcceptParam;
    m_pAcceptParam = pParam;
    if (m_pAcceptParam)
    {
        m_pAcceptParam->AddRef();
    }

    LeaveCriticalSection(&m_csAcceptFunc);

    if (pOldParam)
    {
        pOldParam->Release();
    }
}

BOOL CTCPService::ListenProcess(HANDLE StopEvent)
{
    BOOL Accept = FALSE;
    DWORD Ret = WaitForSingleObject(StopEvent, 0);
    if (Ret != WAIT_TIMEOUT)
    {
        return FALSE;
    }

    SOCKET s = accept(m_hListenSock, NULL, NULL);

    if (s == NULL || s == INVALID_SOCKET)
    {
        return FALSE;
    }

    EnterCriticalSection(&m_csAcceptFunc);

    if (m_pfnAcceptFunc)
    {
        Accept = m_pfnAcceptFunc(s, m_pAcceptParam);
    }

    LeaveCriticalSection(&m_csAcceptFunc);

    if (!Accept)
    {
        shutdown(s, SD_BOTH);
        closesocket(s);
    }

    return TRUE;
}

DWORD WINAPI CTCPService::ListenProc(void* pParam)
{
    CTCPService* service = (CTCPService*)pParam;
    while (TRUE)
    {
        if (!service->ListenProcess(service->m_hStopEvent))
        {
            break;
        }
    }

    service->Release();

	return 0;
}