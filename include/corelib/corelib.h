#ifndef __CORE_LIB_H__
#define __CORE_LIB_H__

#ifdef WIN32
#include <windows.h>
#else
#include <winpr/wtypes.h>
#endif

HANDLE CreateDataStreamBuffer();
void CloseDataStreamBuffer(HANDLE hBuffer);
void DataStreamBufferClearAllData(HANDLE hBuffer);
void DataStreamBufferAddData(HANDLE hBuffer,BYTE *pData, DWORD dwSize);
void DataStreamBufferAddDataFront(HANDLE hBuffer,BYTE *pData, DWORD dwSize);
BYTE *DataStreamBufferGetAllData(HANDLE hBuffer,DWORD *lpDataOut);
BOOL DataStreamBufferGetData(HANDLE hBuffer,BYTE *pBuf, DWORD dwBufSize, DWORD *lpNumOfGet);
DWORD DataStreamBufferGetCurrentDataSize(HANDLE hBuffer);

#endif