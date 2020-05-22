#include "stdafx.h"
#include "P2PHost.h"
#include "P2PPacket.h"

CP2PHost::CP2PHost(CHAR* ClientName, CHAR* Keyword, CHAR* ServerIP, WORD ServerTCPPort)
    : CP2PClient(ClientName, Keyword, ServerIP, ServerTCPPort)
{
    m_hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hStatusChange = CreateEvent(NULL, TRUE, FALSE, NULL);
}

CP2PHost::~CP2PHost()
{
    if (m_hStopEvent)
    {
        CloseHandle(m_hStopEvent);
    }

    if (m_hStatusChange)
    {
        CloseHandle(m_hStatusChange);
    }
}

DWORD CP2PHost::Run()
{
    HANDLE h[2] = {
        m_hStatusChange,
        m_hStopEvent,
    };

    if (!StartListening())
    {
        m_dwErrorCode = P2P_TCP_CONNECT_ERROR;
        return m_dwErrorCode;
    }

    while (TRUE)
    {
        DWORD Ret = WaitForMultipleObjects(2, h, FALSE, INFINITE);
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
                DBG("SWITCH TO P2P MODE\n");
                break;
            }
            case P2P_TCP_PROXY:
            {
                ResetEvent(m_hStatusChange);
                DBG("SWITCH TO TCP RELAY\n");
                break;
            }
            default:
            {
                break;
            }
        }
    }

    Clearup();

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

    if (Ret != WAIT_OBJECT_0)
    {
        return;
    }

    //if timeout, means udp failed, turn to tcp relay
    //otherwise, use main loop to deal with event
    if (timeout)
    {
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

    if (Ret != WAIT_OBJECT_0)
    {
        return;
    }

    //if timeout, means udp failed, turn to tcp relay
    //otherwise, use main loop to deal with event
    if (timeout)
    {
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
            SendUDPToPeer(UPT_KEEPALIVE);
            break;
        }

        case UPT_KEEPALIVE:
        {
            if (m_eStatus == P2P_PUNCHING)
            {
                m_eStatus = P2P_CONNECTED;
                SetEvent(m_hStatusChange);
                Ret = FALSE;
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
            DBG("Init tcpid = %d udpport = %d\r\n", m_dwTCPid, m_dwUDPPort);

            m_eStatus = P2P_TCP_LISTENING;
            SetEvent(m_hStatusChange);
            break;
        }
        case TPT_WAIT_RESULT:
        {
            TCP_RESULT_PACKET* Data = (TCP_RESULT_PACKET*)Packet->Data;
            if (Data->result != 0)
            {
                m_dwErrorCode = Data->result;
                SetEvent(m_hStopEvent);
            }
            else
            {
                //reset everything
                if (m_eStatus == P2P_TCP_LISTENING)
                {
                    m_eStatus = P2P_TCP_CONNECTED;
                    SetEvent(m_hStatusChange);
                }
                else
                {
                    m_dwErrorCode = P2P_PEER_RESET;
                    SetEvent(m_hStopEvent);
                }
            }
            break;
        }
        case TPT_START_UDP:
        {
            m_eStatus = P2P_UDP_LISTENING;
            SetEvent(m_hStatusChange);
        }
        case TPT_CLIENT_INFO:
        {
            //only set when state is udp listening
            if (m_eStatus == P2P_UDP_LISTENING)
            {
                m_eStatus = P2P_PUNCHING;
                TCP_START_PUNCHING_PACKET* Data = (TCP_START_PUNCHING_PACKET*)Packet->Data;
                m_dwPeerid = Data->Peerid;
                memcpy(&m_stRemoteClientInfo, &Data->ClientInfo, sizeof(CLIENT_INFO));
                SetEvent(m_hStatusChange);
            }
        }
        case TPT_PROXY_RESULT:
        {
            TCP_PROXY_RESULT_PACKET* Data = (TCP_PROXY_RESULT_PACKET*)Packet->Data;
            if (Data->Result != P2P_ERROR_NONE)
            {
                m_dwErrorCode = P2P_PEER_RESET;
                SetEvent(m_hStopEvent);
            }
            else
            {
                m_eStatus = P2P_TCP_PROXY;
                SetEvent(m_hStatusChange);
            }
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
    m_dwErrorCode = P2P_TCP_CONNECT_ERROR;
    SetEvent(m_hStopEvent);
}
