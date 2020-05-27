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

DWORD CP2PHost::Listen()
{
    HANDLE h[3] = {
        m_hStatusChange,
        m_hStopEvent,
        m_hConnectedEvent,
    };

    if (!StartListening())
    {
        DBG_ERROR("Start Listening Failed\r\n");
        m_dwErrorCode = P2P_TCP_CONNECT_ERROR;
        return m_dwErrorCode;
    }

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
                UDPListeningEventProccess();
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
                UDPConnectEventProcess();
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

    return m_dwErrorCode;
}

BOOL CP2PHost::StartListening()
{
    m_pTCP->RegisterRecvProcess(CP2PHost::RecvTCPPacketProcessDelegate, this);
    m_pTCP->RegisterEndProcess(CP2PHost::TCPEndProcessDelegate, this);
    m_pUDP->RegisterRecvProcess(CP2PHost::RecvUDPPacketProcessDelegate, this);

    return Init();
}

VOID CP2PHost::Clearup()
{
    Done();
}

VOID CP2PHost::TCPListeningEventProcess()
{
    DBG_TRACE("Send %s Packet to Server\r\n", TCPTypeToString(TPT_WAITING));
    BASE_PACKET_T* Packet = CreateTCPWaitPkt(m_dwTCPid, m_szKeyword, m_szName);
    m_pTCP->SendPacket(Packet);
}

VOID CP2PHost::UDPListeningEventProccess()
{
    DWORD Ret = 0;
    DWORD Retry = 0;
    BOOL timeout = FALSE;

    HANDLE h[2] = {
        m_hStatusChange,
        m_hStopEvent,
    };

    while (TRUE)
    {
        //send udp to server
        DBG_TRACE("Send %s Packet to Server\r\n", UDPTypeToString(UPT_WAITING));
        SendUDPToServer(TRUE);

        //wait 500ms to retry
        Ret = WaitForMultipleObjects(2, h, FALSE, 200);
        if (Ret != WAIT_TIMEOUT)
        {
            break;
        }
        else
        {
            Retry++;
        }

        //wait for total 2s
        if (Retry >= 10)
        {
            timeout = TRUE;
            break;
        }
    }

    if (Ret != WAIT_OBJECT_0 || Ret != WAIT_TIMEOUT)
    {
        return;
    }

    //if timeout, means udp failed, turn to tcp relay
    //otherwise, use main loop to deal with event
    if (timeout)
    {
        DBG_INFO("udp handshake failed, turn to TCP Relay\r\n");
        SetState(P2P_TCP_PROXY_REQUEST);
        BASE_PACKET_T* Packet = CreateTCPProxyRequest(m_dwTCPid, m_szKeyword, m_szName);
        m_pTCP->SendPacket(Packet);
    }
}

VOID CP2PHost::UDPPunchEventProcess()
{
    DWORD Ret = 0;
    DWORD Retry = 0;
    BOOL timeout = FALSE;

    HANDLE h[2] = {
        m_hStatusChange,
        m_hStopEvent,
    };

    while (TRUE)
    {
        //send udp to peer
        DBG_TRACE("Send %s Packet to guest\r\n", UDPTypeToString(UPT_HANDSHAKE));
        SendUDPToPeer(UPT_HANDSHAKE);

        //wait 500ms to retry
        Ret = WaitForMultipleObjects(2, h, FALSE, 200);
        if (Ret != WAIT_TIMEOUT)
        {
            break;
        }
        else
        {
            Retry++;
        }

        //wait for total 2s
        if (Retry >= 10)
        {
            timeout = TRUE;
            break;
        }
    }

    if (Ret != WAIT_OBJECT_0 || Ret != WAIT_TIMEOUT)
    {
        return;
    }

    //if timeout, means udp failed, turn to tcp relay
    //otherwise, use main loop to deal with event
    if (timeout)
    {
        DBG_INFO("udp handshake failed, turn to TCP Relay\r\n");
        SetState(P2P_TCP_PROXY_REQUEST);
        BASE_PACKET_T* Packet = CreateTCPProxyRequest(m_dwTCPid, m_szKeyword, m_szName);
        m_pTCP->SendPacket(Packet);
    }
}

BOOL CP2PHost::RecvUDPPacketProcessDelegate(UDP_PACKET* Packet, CUDPBase* udp, CBaseObject* Param)
{
    UNREFERENCED_PARAMETER(udp);

    BOOL Ret = FALSE;
    CP2PHost* Host = dynamic_cast<CP2PHost*>(Param);

    if (Host)
    {
        Ret = Host->RecvUDPPacketProcess(Packet);
    }

    return Ret;
}

BOOL CP2PHost::RecvTCPPacketProcessDelegate(BASE_PACKET_T* Packet, CTCPBase* tcp, CBaseObject* Param)
{
    UNREFERENCED_PARAMETER(tcp);

    BOOL Ret = FALSE;
    CP2PHost* Host = dynamic_cast<CP2PHost*>(Param);

    if (Host)
    {
        Ret = Host->RecvTCPPacketProcess(Packet);
    }

    return Ret;
}

VOID CP2PHost::TCPEndProcessDelegate(CTCPBase* tcp, CBaseObject* Param)
{
    UNREFERENCED_PARAMETER(tcp);

    CP2PHost* Host = dynamic_cast<CP2PHost*>(Param);

    if (Host)
    {
        Host->TCPEndProcess();
    }

    return;
}


BOOL CP2PHost::RecvUDPPacketProcess(UDP_PACKET* Packet)
{
    BOOL Ret = TRUE;
    switch (Packet->BasePacket.type)
    {
        case UPT_HANDSHAKE:
        {
            DWORD Peerid;
            memcpy(&Peerid, Packet->BasePacket.data, sizeof(DWORD));

            DBG_TRACE("Recv %s packet from %s: %d\r\n", UDPTypeToString(UPT_HANDSHAKE), inet_ntoa(Packet->PacketInfo.ipaddr), ntohs(Packet->PacketInfo.port));
            DBG_TRACE("===> Peerid %d\r\n", Peerid);
            
            if (Peerid == m_dwPeerid)
            {
                DBG_TRACE("===> Send %s Packet to guest\r\n", UDPTypeToString(UPT_KEEPALIVE));
                SendUDPToPeer(UPT_KEEPALIVE);
            }
            else
            {
                DBG_ERROR("===> Peerid mismatch, skip this packet\r\n");
            }
            break;
        }

        case UPT_KEEPALIVE:
        {
            DWORD Peerid;
            memcpy(&Peerid, Packet->BasePacket.data, sizeof(DWORD));

            DBG_TRACE("Recv %s packet from %s: %d\r\n", UDPTypeToString(UPT_KEEPALIVE), inet_ntoa(Packet->PacketInfo.ipaddr), ntohs(Packet->PacketInfo.port));
            DBG_TRACE("===> Peerid %d\r\n", Peerid);

            if (Peerid == m_dwPeerid)
            {
                if (m_eStatus == P2P_PUNCHING)
                {
                    BASE_PACKET_T* P2PSuccess = CreateP2PSuccessPkt(m_dwTCPid, m_szKeyword);
                    m_pTCP->SendPacket(P2PSuccess);
                }
            }
            else
            {
                DBG_ERROR("===> Peerid mismatch, skip this packet\r\n");
            }
            break;
        }
    }

    return Ret;
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
                    DBG_ERROR("===> State is not P2P_TCP_LISTENING, Peer reset\r\n");
                    m_dwErrorCode = P2P_PEER_RESET;
                    SetEvent(m_hStopEvent);
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
        default:
        {
            break;
        }
    }
    return Ret;
}

VOID CP2PHost::TCPEndProcess()
{
    DBG_INFO("TCP Disconnect\r\n");
    m_dwErrorCode = P2P_TCP_CONNECT_ERROR;
    SetEvent(m_hStopEvent);
}
