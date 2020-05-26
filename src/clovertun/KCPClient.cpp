#include "stdafx.h"
#include "KCPClient.h"
#include "UDPCommon.h"
#include "corelib.h"
#include "packetunit.h"

CKCPClient::CKCPClient(SOCKET socket, DWORD PeerId, CLIENT_INFO info)
{
	m_hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    m_hMainThread = NULL;

    m_pfnRecvFunc = NULL;
    m_pRecvParam = NULL;

    m_pKcp = NULL;

    m_hSock = socket;
    m_dwPeerId = PeerId;
    memcpy(&m_stPeerInfo, &info, sizeof(CLIENT_INFO));

    m_hDataStreamBuffer = CreateDataStreamBuffer();

#ifdef WIN32
    m_hSocketEvent = NULL;
#endif

	InitializeCriticalSection(&m_csSendLock);
    InitializeCriticalSection(&m_csRecvFunc);
}

CKCPClient::~CKCPClient()
{
	if (m_hStopEvent)
	{
		CloseHandle(m_hStopEvent);
	}

	if (m_hMainThread)
	{
		CloseHandle(m_hMainThread);
	}

    if (m_hDataStreamBuffer)
    {
		CloseDataStreamBuffer(m_hDataStreamBuffer);
        m_hDataStreamBuffer = NULL;
    }

#ifdef WIN32
    if (m_hSocketEvent)
    {
        CloseHandle(m_hSocketEvent);
    }
#endif

    DeleteCriticalSection(&m_csSendLock);
    DeleteCriticalSection(&m_csRecvFunc);
}

BOOL CKCPClient::Init()
{
#ifdef WIN32
    m_hSocketEvent = WSACreateEvent();
    WSAEventSelect(m_hSock, m_hSocketEvent, FD_READ);
#endif

    AddRef();
	m_hMainThread = CreateThread(NULL, 0, CKCPClient::MainProc, this, 0, NULL);

    return TRUE;
}

VOID CKCPClient::Done()
{
	SetEvent(m_hStopEvent);
}

int CKCPClient::KCPOutputDelegate(const char *buf, int len, struct IKCPCB *kcp, void *user)
{
    UNREFERENCED_PARAMETER(kcp);

    CKCPClient* tcp = (CKCPClient*)user;
    if (tcp)
    {
        tcp->KCPOutput(buf, len);
    }

    return 0;
}

VOID CKCPClient::KCPOutput(const char* buf, int len)
{
    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(struct sockaddr_in));
    servAddr.sin_family = AF_INET;
    memcpy(&servAddr.sin_addr, &m_stPeerInfo.ipaddr, sizeof(ADDRESS));
    servAddr.sin_port = m_stPeerInfo.port;

    UDPSocketSendTo(m_hSock, (PBYTE)buf, len, (struct sockaddr*)&servAddr, sizeof(struct sockaddr_in), m_hStopEvent);
}

VOID CKCPClient::SendPacket(PBYTE Data, DWORD Length)
{
    BASE_PACKET_T* Packet = (BASE_PACKET_T*)malloc(BASE_PACKET_HEADER_LEN + Length);
    Packet->Type = UPT_KCP;
    Packet->Length = BASE_PACKET_HEADER_LEN + Length;
    memcpy(Packet->Data, Data, Length);

	EnterCriticalSection(&m_csSendLock);
	m_SendList.push_back(Packet);
	LeaveCriticalSection(&m_csSendLock);
}

VOID CKCPClient::RegisterRecvProcess(_KCPRecvPacketProcess Process, CBaseObject* Param)
{
    EnterCriticalSection(&m_csRecvFunc);
	m_pfnRecvFunc = Process;
	m_pRecvParam = Param;
    LeaveCriticalSection(&m_csRecvFunc);
}

DWORD WINAPI CKCPClient::MainProc(void* pParam)
{
	CKCPClient* tcp = (CKCPClient*)pParam;

    tcp->KCPInit();

	while (TRUE)
	{
		if (!tcp->KCPProcess(tcp->m_hStopEvent))
		{
			break;
		}
	}

    tcp->KCPDone();

    DBG_INFO("CKCPClient: Main Thread Stop\r\n");

	tcp->Release();

	return 0;
}

VOID CKCPClient::KCPInit()
{
    m_pKcp = ikcp_create(m_dwPeerId, this);
    m_pKcp->output = KCPOutputDelegate;
    ikcp_nodelay(m_pKcp, 1, 10, 1, 1);
}

VOID CKCPClient::KCPDone()
{
    ikcp_release(m_pKcp);
    m_pKcp = NULL;
}

BOOL CKCPClient::KCPProcess(HANDLE StopEvent)
{
    ikcp_update(m_pKcp, GetTickCount());

    EnterCriticalSection(&m_csSendLock);
    while (!m_SendList.empty())
    {
        BASE_PACKET_T* Packet = m_SendList.front();
        m_SendList.pop_front();

        ikcp_send(m_pKcp, (const char*)Packet, Packet->Length);

        free(Packet);
    }
	LeaveCriticalSection(&m_csSendLock);

#ifdef WIN32
    HANDLE h[2] = {
        m_hSocketEvent,
        StopEvent,
    };

    DWORD Ret = WSAWaitForMultipleEvents(2, h, FALSE, 0, FALSE);

    if (Ret == WAIT_OBJECT_0)
    {
        WSANETWORKEVENTS networkEvents;

        if (WSAEnumNetworkEvents(m_hSock, m_hSocketEvent, &networkEvents) == 0)
        {
            if (networkEvents.lNetworkEvents & FD_READ)
            {
                if (networkEvents.iErrorCode[FD_READ_BIT] == 0)
                {
                    BYTE buffer[4096];
                    struct sockaddr addr;
                    int addrLen = sizeof(struct sockaddr_in);
                    int recvlen = recvfrom(m_hSock, (char*)buffer, 4096, 0, &addr, &addrLen);

                    if (recvlen > 0)
                    {
                        ikcp_input(m_pKcp, (const char*)buffer, recvlen);
                    }
                }
            }
        }
    } 
    else if (Ret != WAIT_TIMEOUT)
    {
        return FALSE;
    }
#endif

    BYTE buffer[4096];
    int len = ikcp_recv(m_pKcp, (char*)buffer, 4096);

    if (len > 0)
    {
        DataStreamBufferAddData(m_hDataStreamBuffer, buffer, len);
    }

    while (TRUE)
	{
		BASE_PACKET_T* Packet = GetPacketFromBuffer(m_hDataStreamBuffer);
		if (Packet != NULL)
		{
			EnterCriticalSection(&m_csRecvFunc);
            if (m_pfnRecvFunc)
			{
				m_pfnRecvFunc(Packet->Data, Packet->Length - BASE_PACKET_HEADER_LEN, this, m_pRecvParam);
			}
			LeaveCriticalSection(&m_csRecvFunc);

            free(Packet);
		}
		else
		{
            break;
		}
	}

    return TRUE;
}

