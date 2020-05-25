#pragma once

#ifndef __P2P_PACKET_H__
#define __P2P_PACKET_H__

#ifdef WIN32
#include <windows.h>
#else
#include <winpr/wtypes.h>
#endif

#include "BasePacket.h"
#include "P2PProtocol.h"

BASE_PACKET_T* CreateTCPInitPkt(DWORD tcpid, WORD UDPPort);
BASE_PACKET_T* CreateTCPStartUDPPkt(DWORD tcpid, CHAR* Keyword);
BASE_PACKET_T* CreateTCPResultPkt(DWORD type, DWORD tcpid, DWORD result);
BASE_PACKET_T* CreateTCPStartPunchingPkt(DWORD tcpid, DWORD PeerId, CLIENT_INFO* Info);

BASE_PACKET_T* CreateTCPWaitPkt(DWORD tcpid, CHAR* Keyword, CHAR* name);
BASE_PACKET_T* CreateTCPConnPkt(DWORD tcpid, CHAR* Keyword, CHAR* name);
BASE_PACKET_T* CreateTCPProxyRequest(DWORD tcpid, CHAR* Keyword, CHAR* name);
BASE_PACKET_T* CreateP2PSuccessPkt(DWORD tcpid, CHAR* Keyword);

BASE_PACKET_T* CreateTCPProxyResultPkt(DWORD tcpid, DWORD peerid, DWORD result);
BASE_PACKET_T* CreateTCPProxyData(TCP_PROXY_DATA* Data, DWORD Length);

char* TCPTypeToString(DWORD Type);
char* UDPTypeToString(DWORD Type);
char* P2PStatusToString(DWORD Type);
char* P2PErrorToString(DWORD Type);


#endif