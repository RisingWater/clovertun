#pragma once

#ifndef __BASE_PACKET_T_H__
#define __BASE_PACKET_T_H__

#ifdef WIN32
#include <Windows.h>
#else
#include <winpr/wtypes.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#pragma warning(disable:4200)
#pragma pack(1)

typedef struct
{
    DWORD  Length;
    DWORD  Type;
    BYTE   Data[1];
} BASE_PACKET_T, *PBASE_PACKET_T;

#define BASE_PACKET_HEADER_LEN      (sizeof(BASE_PACKET_T) - 1)

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif
