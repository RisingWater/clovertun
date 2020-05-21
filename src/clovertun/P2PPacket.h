#pragma once

#ifndef __P2P_PACKET_H__
#define __P2P_PACKET_H__

#ifdef WIN32
#include <windows.h>
#else
#include <winpr/wtypes.h>
#endif

#include "BasePacket.h"

BASE_PACKET_T* CreateTCPInit(DWORD tcpid);
BASE_PACKET_T* CreateTCPStartUDP(DWORD tcpid, CHAR* Keyword);
BASE_PACKET_T* CreateTCPResult(DWORD type, DWORD tcpid, DWORD result);

#endif