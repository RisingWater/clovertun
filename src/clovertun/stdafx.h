#pragma once

#ifndef __STDAFX_H__
#define __STDAFX_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef WIN32
#include <windows.h>
#define closesocket close
#else

#endif

#define DBG printf

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#endif