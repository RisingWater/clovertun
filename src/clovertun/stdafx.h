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

#define DBG printf

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#endif