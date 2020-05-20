#include "stdafx.h"
#include "UDPBase.h"

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
    m_hRecvEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hRecvThread = NULL;
    m_hSendThread = NULL;
    m_hDealThread = NULL;

    InitializeCriticalSection(&m_csSendLock);
    InitializeCriticalSection(&m_csRecvLock);
}

CUDPBase::~CUDPBase()
{
    if (m_hSock != INVALID_SOCKET)
    {
        closesocket(m_hSock);
        m_hSock = INVALID_SOCKET;
    }

    if (m_hRecvThread)
    {
        CloseHandle(m_hRecvThread);
    }

    if (m_hSendThread)
    {
        CloseHandle(m_hSendThread);
    }

    if (m_hDealThread)
    {
        CloseHandle(m_hDealThread);
    }

    if (m_hSendEvent)
    {
        CloseHandle(m_hSendEvent);
    }

    if (m_hRecvEvent)
    {
        CloseHandle(m_hSendEvent);
    }

    DeleteCriticalSection(&m_csSendLock);
    DeleteCriticalSection(&m_csRecvLock);
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

	return 0;
}

DWORD WINAPI CUDPBase::DealProc(void* pParam)
{
    CUDPBase* udp = (CUDPBase*)pParam;
    while (TRUE)
    {
        if (!udp->DealProcess(udp->m_hStopEvent))
        {
            break;
        }
    }

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

	return 0;
}

BOOL CUDPBase::Init()
{
    m_hSendThread = CreateThread(NULL, 0, CUDPBase::DealProc, this, 0, NULL);
    m_hRecvThread = CreateThread(NULL, 0, CUDPBase::RecvProc, this, 0, NULL);
    m_hDealThread = CreateThread(NULL, 0, CUDPBase::SendProc, this, 0, NULL);

    return TRUE;
}

VOID CUDPBase::Done()
{
    SetEvent(m_hStopEvent);
    if (m_hSock != INVALID_SOCKET)
    {
        closesocket(m_hSock);
        m_hSock = INVALID_SOCKET;
    }
}

VOID CUDPBase::SendPacket(UDP_PACKET Packet)
{
    EnterCriticalSection(&m_csSendLock);
    m_SendList.push_back(Packet);
    SetEvent(m_hSendEvent);
    LeaveCriticalSection(&m_csSendLock);
    return;
}

BOOL CUDPBase::RecvProcess(HANDLE StopEvent)
{
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
        DBG("UDP包数据检验位错误\n");
        return FALSE;
    }

    memcpy(&Packet.PacketInfo.ipaddr, &otherInfo.sin_addr, sizeof(ADDRESS));
    Packet.PacketInfo.port = otherInfo.sin_port;

    EnterCriticalSection(&m_csRecvLock);
    m_RecvList.push_back(Packet);
    SetEvent(m_hRecvEvent);
    LeaveCriticalSection(&m_csRecvLock);

    return TRUE;
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
        UDP_PACKET Packet = m_SendList.front();
        m_SendList.pop_front();

        LeaveCriticalSection(&m_csSendLock);

        struct sockaddr_in servAddr;
        memset(&servAddr, 0, sizeof(struct sockaddr_in));
        servAddr.sin_family = AF_INET;
        memcpy(&servAddr.sin_addr, &Packet.PacketInfo.ipaddr, sizeof(ADDRESS));
        servAddr.sin_port = Packet.PacketInfo.port;

        CalcCheckSum(&Packet.BasePacket);

		sendto(m_hSock, (char*)&Packet.BasePacket, sizeof(UDPBASE_PACKET), 0, (struct sockaddr*)&servAddr, sizeof(struct sockaddr_in));
    }
    else
    {
        ResetEvent(m_hSendEvent);
        LeaveCriticalSection(&m_csSendLock);
    }

    return TRUE;
}

BOOL CUDPBase::DealProcess(HANDLE StopEvent)
{
    HANDLE h[2] = {
        m_hRecvEvent,
        m_hStopEvent,
    };

    DWORD Ret = WaitForMultipleObjects(2, h, FALSE, INFINITE);
    if (Ret != WAIT_OBJECT_0)
    {
        return FALSE;
    }

    EnterCriticalSection(&m_csRecvLock);
    if (!m_RecvList.empty())
    {
        UDP_PACKET Packet = m_RecvList.front();
        m_RecvList.pop_front();

        LeaveCriticalSection(&m_csRecvLock);

        RecvPacketProcess(Packet);
    }
    else
    {
        ResetEvent(m_hRecvEvent);
        LeaveCriticalSection(&m_csRecvLock);
    }

    return TRUE;
}