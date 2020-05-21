#pragma once

#ifndef __SOCKET_HELP_H__
#define __SOCKET_HELP_H__

#ifdef WIN32
#include <WinSock2.h>
#include <windows.h>
#else
#include "Windef.h"
#endif

BOOL SocketRead(SOCKET s, BYTE* pBuffer, DWORD dwBufferSize, DWORD* pdwReaded, HANDLE hStopEvent);
BOOL SocketWrite(SOCKET s, BYTE* pBuffer, DWORD dwBufferSize, DWORD* pdwWritten, HANDLE hStopEvent);

#ifndef WIN32
#define closesocket close
#endif

#endif