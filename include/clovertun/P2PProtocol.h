#pragma once

#ifndef __P2P_PROTOCOL_H__
#define __P2P_PROTOCOL_H__

#ifdef WIN32
#include <WinSock2.h>
#include <windows.h>
#else
#include <winpr/wtypes.h>
#endif

#define UDP_PORT_SERVER_BASE 29000
#define UDP_PORT_CLIENT_BASE 30000

#define KEYWORD_SIZE 32
#define NAME_SIZE 128

#define UDP_DATA_MAX 247

#pragma pack(1)

typedef struct in_addr ADDRESS;

typedef enum {
    P2P_ERROR_NONE = 0,
    P2P_ERROR_TCPID_ERROR,
    P2P_TCP_CONNECT_ERROR,
    P2P_KEYWORD_DUPLICATE,
    P2P_KEYWORD_NOT_FOUND,
    P2P_STATE_MISMATCH,
    P2P_PUNCH_TIMEOUT,
    P2P_PROXY_FAIL,
    P2P_PEER_RESET,
    P2P_ERROR_UNKNOW,
} P2P_ERROR;

typedef enum
{
    P2P_STATUS_NONE = 0,
    P2P_TCP_LISTENING,
    P2P_TCP_CONNECTED,
    P2P_UDP_LISTENING,
    P2P_UDP_CONNECTED,
    P2P_PUNCHING,
    P2P_CONNECTED,
    P2P_TCP_PROXY_REQUEST,
    P2P_TCP_PROXY,
    P2P_STATUS_MAX,
} P2P_STATUS;

typedef struct
{
	ADDRESS ipaddr;
	WORD    port;
} CLIENT_INFO;

typedef struct
{
	DWORD type;
	DWORD length;
	BYTE  data[UDP_DATA_MAX];
	BYTE  checksum;
} UDPBASE_PACKET;

typedef struct
{
    UDPBASE_PACKET BasePacket;
    CLIENT_INFO    PacketInfo;
} UDP_PACKET;

typedef enum
{
    UPT_WAITING = 0,
    UPT_CONNECT,
    UPT_HANDSHAKE,
    UPT_KEEPALIVE,
    UPT_P2P_SUCCESS,
    UPT_KCP,
    UPT_MAX,
} UDPBASE_PACKET_TYPE;

typedef enum
{
    TPT_INIT = 0,
    TPT_WAITING,
    TPT_WAIT_RESULT,
    TPT_CONNECT,
    TPT_CONNECT_RESULT,
    TPT_START_UDP,
    TPT_CLIENT_INFO,
    TPT_P2P_RESULT,
    TPT_PROXY_REQUEST,
    TPT_PROXY_RESULT,
    TPT_DATA_PROXY,
    TPT_MAX,
} TCPBASE_PACKET_TYPE;

typedef struct
{
    DWORD tcpid;
    WORD UDPPort;
} TCP_INIT_PACKET;

typedef struct
{
    DWORD tcpid;
    BYTE  keyword[KEYWORD_SIZE];
    BYTE  clientName[NAME_SIZE];
} TCP_WAIT_PACKET, TCP_CONN_PACKET, UDP_WAIT_PACKET, UDP_CONN_PACKET, TCP_PROXY_REQUEST_PACKET, P2P_RESULT_PACKET;

typedef struct
{
    DWORD tcpid;
    DWORD result;
} TCP_RESULT_PACKET;

typedef struct
{
    DWORD tcpid;
    BYTE  keyword[KEYWORD_SIZE];
} TCP_START_UDP_PACKET;

typedef struct
{
    DWORD       tcpid;
    DWORD       Peerid;
    CLIENT_INFO ClientInfo;
} TCP_START_PUNCHING_PACKET;

typedef struct
{
    DWORD       Result;
    DWORD       tcpid;
    DWORD       Peerid; 
} TCP_PROXY_RESULT_PACKET;

typedef struct
{
    DWORD       Peerid;
    DWORD       Host2Guest;
    DWORD       Length;
    BYTE        Data[1];
} TCP_PROXY_DATA;

#define TCP_PROXY_DATA_HEADER_LEN (sizeof(TCP_PROXY_DATA) - 1)

#pragma pack()


#endif