#include "stdafx.h"
#include "UDPBase.h"

#ifdef WIN32
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)
#endif

static BOOL IsCheckSumVaild(UDPBASE_PACKET Packet)
{
	BYTE checkS = 0;
	for (int i = 0; i < 247; i++)
	{
		checkS += Packet.data[i];
	}

	if (checkS != Packet.checksum)
	{
		return FALSE;
	}

    return TRUE;
}

static VOID CalcCheckSum(UDPBASE_PACKET* Packet)
{
	BYTE checkS = 0;
	for (int i = 0; i < 247; i++)
	{
		checkS += Packet->data[i];
	}

    Packet->checksum = checkS;
}

CUDPBase::CUDPBase()
{
	m_hSock = socket(PF_INET, SOCK_DGRAM, 0);

    m_hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hSendEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hRecvThread = NULL;
    m_hSendThread = NULL;

    m_pfnRecvFunc = NULL;
    m_pRecvParam = NULL;

    m_dwLocalPort = 0;

    InitializeCriticalSection(&m_csSendLock);
    InitializeCriticalSection(&m_csRecvFunc);
}

CUDPBase::~CUDPBase()
{
    if (m_hRecvThread)
    {
        CloseHandle(m_hRecvThread);
    }

    if (m_hSendThread)
    {
        CloseHandle(m_hSendThread);
    }

    if (m_hSendEvent)
    {
        CloseHandle(m_hSendEvent);
    }

    DeleteCriticalSection(&m_csSendLock);
    DeleteCriticalSection(&m_csRecvFunc);
}

DWORD WINAPI CUDPBase::RecvProc(void* pParam)
{
    CUDPBase* udp = (CUDPBase*)pParam;
    while (TRUE)
    {
        if (!udp->RecvProcess(udp->m_hStopEvent))
        {
            break;
        }
    }

    udp->Release();

	return 0;
}

DWORD WINAPI CUDPBase::SendProc(void* pParam)
{
    CUDPBase* udp = (CUDPBase*)pParam;
    while (TRUE)
    {
        if (!udp->SendProcess(udp->m_hStopEvent))
        {
            break;
        }
    }

    udp->Release();

	return 0;
}

BOOL CUDPBase::Init(WORD UdpPort)
{
    m_dwLocalPort = UdpPort;

    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(struct sockaddr_in));
    servAddr.sin_family = PF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(m_dwLocalPort);

    int res = bind(m_hSock, (SOCKADDR*)&servAddr, sizeof(SOCKADDR));

    if (res < 0)
    {
        return FALSE;
    }

#ifdef WIN32
    BOOL bNewBehavior = FALSE;
    DWORD dwBytesReturned = 0;
    WSAIoctl(m_hSock, SIO_UDP_CONNRESET, &bNewBehavior, sizeof(bNewBehavior), NULL, 0, &dwBytesReturned, NULL, NULL);
#endif

    AddRef();
    m_hSendThread = CreateThread(NULL, 0, CUDPBase::SendProc, this, 0, NULL);

    AddRef();
    m_hRecvThread = CreateThread(NULL, 0, CUDPBase::RecvProc, this, 0, NULL);

    return TRUE;
}

VOID CUDPBase::Done()
{
    SetEvent(m_hStopEvent);

    RegisterRecvProcess(NULL, NULL);

    if (m_hSock != INVALID_SOCKET)
    {
        closesocket(m_hSock);
        m_hSock = INVALID_SOCKET;
    }
}

VOID CUDPBase::RegisterRecvProcess(_UDPRecvPacketProcess Process, CBaseObject* Param)
{
    CBaseObject* pOldParam = NULL;

    EnterCriticalSection(&m_csRecvFunc);
    m_pfnRecvFunc = Process;

    pOldParam = m_pRecvParam;
    m_pRecvParam = Param;
    if (m_pRecvParam)
    {
        m_pRecvParam->AddRef();
    }

    LeaveCriticalSection(&m_csRecvFunc);

    if (pOldParam)
    {
        pOldParam->Release();
    }
}

VOID CUDPBase::SendPacket(UDP_PACKET* Packet)
{
    CalcCheckSum(&Packet->BasePacket);

    EnterCriticalSection(&m_csSendLock);
    m_SendList.push_back(Packet);
    SetEvent(m_hSendEvent);
    LeaveCriticalSection(&m_csSendLock);
    return;
}

BOOL CUDPBase::RecvProcess(HANDLE StopEvent)
{
    BOOL Ret = FALSE;
    sockaddr_in otherInfo;
    int otherInfoSize = sizeof(struct sockaddr);

    UDP_PACKET Packet;
    memset(&Packet, 0, sizeof(UDP_PACKET));

    int length = recvfrom(m_hSock, (char*)&Packet.BasePacket, sizeof(UDPBASE_PACKET), 0, (struct sockaddr*)&otherInfo, &otherInfoSize);

    if (length < 0)
    {
        DBG("recv failed last error %d\n", GetLastError());
        return FALSE;
    }

    if (!IsCheckSumVaild(Packet.BasePacket))
    {
        DBG("check sum error\n");
        return TRUE;
    }

    memcpy(&Packet.PacketInfo.ipaddr, &otherInfo.sin_addr, sizeof(ADDRESS));
    Packet.PacketInfo.port = otherInfo.sin_port;

    EnterCriticalSection(&m_csRecvFunc);

    if (m_pfnRecvFunc)
    {
        Ret = m_pfnRecvFunc(&Packet, this, m_pRecvParam);
    }

    LeaveCriticalSection(&m_csRecvFunc);

    if (!Ret)
    {
        DBG("Process Packet failed, stop recv thread\n");
    }

    return Ret;
}

BOOL CUDPBase::SendProcess(HANDLE StopEvent)
{
    HANDLE h[2] = {
        m_hSendEvent,
        m_hStopEvent,
    };

    DWORD Ret = WaitForMultipleObjects(2, h, FALSE, INFINITE);
    if (Ret != WAIT_OBJECT_0)
    {
        return FALSE;
    }

    EnterCriticalSection(&m_csSendLock);
    if (!m_SendList.empty())
    {
        UDP_PACKET* Packet = m_SendList.front();
        m_SendList.pop_front();

        LeaveCriticalSection(&m_csSendLock);

        struct sockaddr_in servAddr;
        memset(&servAddr, 0, sizeof(struct sockaddr_in));
        servAddr.sin_family = AF_INET;
        memcpy(&servAddr.sin_addr, &Packet->PacketInfo.ipaddr, sizeof(ADDRESS));
        servAddr.sin_port = Packet->PacketInfo.port;

		sendto(m_hSock, (char*)&Packet->BasePacket, sizeof(UDPBASE_PACKET), 0, (struct sockaddr*)&servAddr, sizeof(struct sockaddr_in));

        free(Packet);
    }
    else
    {
        ResetEvent(m_hSendEvent);
        LeaveCriticalSection(&m_csSendLock);
    }

    return TRUE;
}
