#pragma once

#ifndef __UDP_BASE_H__
#define __UDP_BASE_H__

#ifdef WIN32
#include <WinSock2.h>
#include <windows.h>
#else
#include <winpr/wtypes.h>
#endif

#include <list>

#define BUFF_SIZE 256

#pragma pack(1)

typedef struct in_addr ADDRESS;

typedef struct
{
	ADDRESS ipaddr;
	WORD    port;
} CLIENT_INFO;

typedef struct
{
	DWORD type;
	DWORD length;
	BYTE  data[247];
	BYTE  checksum;
} UDPBASE_PACKET;

typedef struct
{
    BYTE  keyword[32];
    BYTE  clientName[32];
} CONNECT_PACKET_DATA;

typedef struct
{
    UDPBASE_PACKET BasePacket;
    CLIENT_INFO    PacketInfo;
} UDP_PACKET;

typedef enum
{
    UPT_WAITING = 0,
    UPT_CONNECT,
    UPT_SERVER_RESPONSE,
    UPT_HANDSHAKE,
    UPT_KEEPALIVE,
} UDPBASE_PACKET_TYPE;

#pragma pack()

class CUDPBase
{
public :
	CUDPBase();
	~CUDPBase();

    BOOL Init();
    VOID Done();

    VOID SendPacket(UDP_PACKET Packet);
    virtual VOID RecvPacketProcess(UDP_PACKET Packet) = 0;

protected:
	SOCKET m_hSock;
	WORD  m_dwLocalPort;
	std::list<UDP_PACKET> m_RecvList;
	std::list<UDP_PACKET> m_SendList;

private:
    static DWORD WINAPI RecvProc(void* pParam);
    static DWORD WINAPI DealProc(void* pParam);
    static DWORD WINAPI SendProc(void* pParam);

    HANDLE m_hRecvThread;
    HANDLE m_hSendThread;
    HANDLE m_hDealThread;
    
    HANDLE m_hStopEvent;
    HANDLE m_hSendEvent;
    HANDLE m_hRecvEvent;

	BOOL SendProcess(HANDLE StopEvent);
	BOOL RecvProcess(HANDLE StopEvent);
    BOOL DealProcess(HANDLE StopEvent);

    CRITICAL_SECTION m_csRecvLock;
    CRITICAL_SECTION m_csSendLock;
};

#endif

