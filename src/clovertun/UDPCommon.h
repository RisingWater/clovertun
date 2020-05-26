#pragma once

#ifndef __SOCKET_HELP_H__
#define __SOCKET_HELP_H__

#ifdef WIN32
#include <WinSock2.h>
#include <windows.h>
#else
#include "Windef.h"
#endif

BOOL UDPSocketRecvFrom(SOCKET s, BYTE* pBuffer, DWORD dwBufferSize, DWORD* RecvLength, struct sockaddr* addr, DWORD* addrLen, HANDLE hStopEvent);
BOOL UDPSocketSendTo(SOCKET s, BYTE* pBuffer, DWORD dwBufferSize, struct sockaddr* addr, DWORD addrLen, HANDLE hStopEvent);

#endif