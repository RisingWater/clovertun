#include "stdafx.h"
#include "P2PHost.h"
#include "P2PPacket.h"

CP2PHost::CP2PHost(CHAR* ClientName, CHAR* Keyword, CHAR* ServerIP, WORD ServerTCPPort)
    : CP2PClient(ClientName, Keyword, ServerIP, ServerTCPPort, P2P_CLIENT_HOST)
{

}

CP2PHost::~CP2PHost()
{

}

VOID CP2PHost::Close()
{
    SetEvent(m_hStopEvent);
    Done();
}

DWORD CP2PHost::Listen()
{
    if (!Init())
    {
        DBG_ERROR("Start Listening Failed\r\n");
        m_dwErrorCode = P2P_TCP_CONNECT_ERROR;
        return m_dwErrorCode;
    }

    return P2P_ERROR_NONE;
}

VOID CP2PHost::WaitForStop()
{
    DWORD Ret = WaitForSingleObject(m_hStopEvent, INFINITE);
    if (Ret != WAIT_OBJECT_0)
    {
        DBG_ERROR("Wait Error %d\r\n", Ret);
    }
}

DWORD CP2PHost::Accept()
{
    ResetEvent(m_hStopEvent);
    
    HANDLE h[3] = {
        m_hStatusChange,
        m_hStopEvent,
        m_hConnectedEvent,
    };

    m_dwErrorCode = P2P_ERROR_NONE;

    while (TRUE)
    {
        DWORD Ret = WaitForMultipleObjects(3, h, FALSE, INFINITE);
        if (Ret != WAIT_OBJECT_0)
        {
            break;
        }

        switch (m_eStatus)
        {
            case P2P_TCP_LISTENING:
            {
                ResetEvent(m_hStatusChange);
                TCPListeningEventProcess();
                break;
            }
            case P2P_TCP_CONNECTED:
            {
                ResetEvent(m_hStatusChange);
                break;
            }
            case P2P_UDP_LISTENING:
            {
                ResetEvent(m_hStatusChange);
                UDPInfoExchangeEventProccess();
                break;
            }
            case P2P_PUNCHING:
            {
                ResetEvent(m_hStatusChange);
                UDPPunchEventProcess();
                break;
            }
            case P2P_CONNECTED:
            {
                ResetEvent(m_hStatusChange);
                P2PConnectEventProcess();
                break;
            }
            case P2P_TCP_PROXY:
            {
                ResetEvent(m_hStatusChange);
                TCPProxyEventProcess();
                break;
            }
            default:
            {
                break;
            }
        }
    }

    ResetEvent(m_hConnectedEvent);
    
    return m_dwErrorCode;
}

VOID CP2PHost::TCPListeningEventProcess()
{
    DBG_TRACE("Send %s Packet to Server\r\n", TCPTypeToString(TPT_WAITING));
    BASE_PACKET_T* Packet = CreateTCPWaitPkt(m_dwTCPid, m_szKeyword, m_szName);

    EnterCriticalSection(&m_csTCPLock);
    if (m_pTCP)
    {
        m_pTCP->SendPacket(Packet);
    }
    else
    {
        free(Packet);
    }
    LeaveCriticalSection(&m_csTCPLock);
}

BOOL CP2PHost::RecvTCPPacketProcess(BASE_PACKET_T* Packet)
{
    BOOL Ret = TRUE;
    switch (Packet->Type)
    {
        case TPT_INIT:
        {
            TCP_INIT_PACKET* Data = (TCP_INIT_PACKET*)Packet->Data;
            m_dwTCPid = Data->tcpid;
            m_dwServerUDPPort = Data->UDPPort;

            m_dwPeerid = 0;
            m_eType = PCT_NONE;

            DBG_TRACE("Recv %s packet from server\r\n", TCPTypeToString(TPT_INIT));
            DBG_TRACE("===> Tcpid %d udpport %d\r\n", m_dwTCPid, m_dwUDPPort);

            SetState(P2P_TCP_LISTENING);
            SetEvent(m_hStatusChange);
            break;
        }
        case TPT_WAIT_RESULT:
        {
            TCP_RESULT_PACKET* Data = (TCP_RESULT_PACKET*)Packet->Data;

            DBG_TRACE("Recv %s packet from server\r\n", TCPTypeToString(TPT_WAIT_RESULT));
            DBG_TRACE("===> result %s\r\n", P2PErrorToString(Data->result));
            if (Data->result != 0)
            {
                m_dwErrorCode = Data->result;
                SetEvent(m_hStopEvent);
            }
            else
            {
                if (m_eStatus == P2P_TCP_LISTENING)
                {
                    SetState(P2P_TCP_CONNECTED);
                    SetEvent(m_hStatusChange);
                }
                else
                {
                    DBG_ERROR("===> State is not P2P_TCP_LISTENING, skip this packet\r\n");
                }
            }
            break;
        }
        case TPT_START_UDP:
        {
            DBG_TRACE("Recv %s packet from server\r\n", TCPTypeToString(TPT_START_UDP));
            SetState(P2P_UDP_LISTENING);
            SetEvent(m_hStatusChange);
            break;
        }
        case TPT_CLIENT_INFO:
        {
            DBG_TRACE("Recv %s packet from server\r\n", TCPTypeToString(TPT_CLIENT_INFO));
            if (m_eStatus == P2P_UDP_LISTENING)
            {
                TCP_START_PUNCHING_PACKET* Data = (TCP_START_PUNCHING_PACKET*)Packet->Data;
                m_dwPeerid = Data->Peerid;
                memcpy(&m_stRemoteClientInfo, &Data->ClientInfo, sizeof(CLIENT_INFO));

                DBG_TRACE("===> Peerid %d guset addr %s:%d %d\r\n", m_dwPeerid, inet_ntoa(m_stRemoteClientInfo.ipaddr), ntohs(m_stRemoteClientInfo.port));
                SetState(P2P_PUNCHING);
                SetEvent(m_hStatusChange);
            }
            else
            {
                DBG_ERROR("===> State is not TPT_CLIENT_INFO, skip this packet\r\n");
            }
            break;
        }
        case TPT_P2P_RESULT:
        {
            DBG_TRACE("Recv %s packet from server\r\n", TCPTypeToString(TPT_P2P_RESULT));
            if (m_eStatus == P2P_PUNCHING)
            {
                SetState(P2P_CONNECTED);
                SetEvent(m_hStatusChange);
            }
            else
            {
                DBG_ERROR("===> State is not P2P_PUNCHING, skip this packet\r\n");
            }
            break;
        }
        case TPT_PROXY_RESULT:
        {
            TCP_PROXY_RESULT_PACKET* Data = (TCP_PROXY_RESULT_PACKET*)Packet->Data;
            DBG_TRACE("Recv %s packet from server\r\n", TCPTypeToString(TPT_PROXY_RESULT));
            DBG_TRACE("===> result %s\r\n", P2PErrorToString(Data->Result));

            if (Data->Result != P2P_ERROR_NONE)
            {
                m_dwErrorCode = P2P_PEER_RESET;
                SetEvent(m_hStopEvent);
            }
            else
            {
                SetState(P2P_TCP_PROXY);
                SetEvent(m_hStatusChange);
            }
            break;
        }
        case TPT_DATA_PROXY:
        {
            if (m_eStatus == P2P_TCP_PROXY)
            {
                Ret = TCPProxyPacketProcess(Packet);
            }
            break;
        }
        case TPT_RESET:
        {
            DBG_TRACE("Recv %s packet from server\r\n", TCPTypeToString(TPT_RESET));
            SetEvent(m_hStopEvent);
        }
        default:
        {
            break;
        }
    }
    return Ret;
}

