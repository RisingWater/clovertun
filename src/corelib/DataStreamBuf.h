#ifndef _DATASTREAMBUF_H_
#define _DATASTREAMBUF_H_

#pragma once

#include <list>
#ifdef WIN32
#include <windows.h>
#else
#include <winpr/wtypes.h>
#endif
#define  BUFSIZE 1024 * 1024

using namespace std;

typedef struct DataBlock
{
	BYTE *pBuffer;
	DWORD m_dwTotalSize;
	DWORD m_dwIdx;
} DATABLOCK, *PDATABLOCK;

class CDataStreamBuf
{
public:
	CDataStreamBuf(void);
	~CDataStreamBuf(void);

	void ClearAll();

	void AddData(BYTE *pData, DWORD dwSize);
	void AddDataFront(BYTE *pData, DWORD dwSize);


	BYTE *GetAllData(DWORD *lpDataOut);
	BOOL GetData(BYTE *pBuf, DWORD dwBufSize, DWORD *lpNumOfGet);

	DWORD GetCurrentDataSize();

	void SetName(char *name);

private:
	DWORD m_dwTotalSize;
	list<PDATABLOCK>* m_pDataList;
	CRITICAL_SECTION m_DataSection;
	char m_name[MAX_PATH];

private:
	DWORD GetTotalDataSize();
	BOOL GetDataCore(BYTE *pBuf, DWORD dwBufSize, DWORD *lpNumOfGet);
};


#endif

