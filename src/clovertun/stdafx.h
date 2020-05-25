#pragma once

#ifndef __STDAFX_H__
#define __STDAFX_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef WIN32
#include <WinSock2.h>
#include <windows.h>
#pragma warning(disable:4127)
#else
#define closesocket close
#endif

#define DBG_ERROR printf
#define DBG_WARN  printf
#define DBG_INFO  printf
#define DBG_TRACE printf

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#endif