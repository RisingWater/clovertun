#include "stdafx.h"
#include "DataStreamBuf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CDataStreamBuf::CDataStreamBuf(void)
{
	m_pDataList = new list<PDATABLOCK>();
	InitializeCriticalSection(&m_DataSection);
	ZeroMemory(m_name, sizeof(m_name));
	ClearAll();
}

void CDataStreamBuf::SetName(char *name)
{
	sprintf_s(m_name, "%s", name);
}

CDataStreamBuf::~CDataStreamBuf(void)
{
	ClearAll();
	EnterCriticalSection(&m_DataSection);

	if (m_pDataList)
	{
		delete m_pDataList;
		m_pDataList = NULL;
	}

	LeaveCriticalSection(&m_DataSection);

	DeleteCriticalSection(&m_DataSection);
}

void CDataStreamBuf::AddData(BYTE *pData, DWORD dwSize)
{
	if (dwSize == 0)
	{
		return;
	}

	EnterCriticalSection(&m_DataSection);

	if (m_pDataList)
	{
		PDATABLOCK pDataBlock = new DATABLOCK();
		pDataBlock->m_dwIdx = 0;
		pDataBlock->m_dwTotalSize = dwSize;
		pDataBlock->pBuffer = new BYTE[dwSize];
		memcpy(pDataBlock->pBuffer, pData, dwSize);
		m_dwTotalSize += dwSize;
		m_pDataList->push_back(pDataBlock);
	}

	LeaveCriticalSection(&m_DataSection);
}

void CDataStreamBuf::ClearAll()
{
	EnterCriticalSection(&m_DataSection);
	m_dwTotalSize = 0;

	if (m_pDataList)
	{
		for (list<PDATABLOCK>::iterator i = m_pDataList->begin(); i != m_pDataList->end(); i ++)
		{
			PDATABLOCK pData = (PDATABLOCK)(*i);
			delete pData->pBuffer;
			delete pData;
		}

		m_pDataList->clear();
	}

	LeaveCriticalSection(&m_DataSection);
}

BYTE *CDataStreamBuf::GetAllData(DWORD *lpDataOut)
{
	BYTE *pRet = NULL;
	EnterCriticalSection(&m_DataSection);

	if (m_dwTotalSize == 0)
	{
		pRet = NULL;
	}
	else
	{
		DWORD dwread = 0;
		pRet = new BYTE[m_dwTotalSize];

		GetDataCore(pRet, m_dwTotalSize, &dwread);
		*lpDataOut = dwread;
	}

	LeaveCriticalSection(&m_DataSection);
	return pRet;
}

BOOL CDataStreamBuf::GetData(BYTE *pBuf, DWORD dwBufSize, DWORD *lpNum)
{
	BOOL ret;
	EnterCriticalSection(&m_DataSection);

	ret = GetDataCore(pBuf, dwBufSize, lpNum);

	LeaveCriticalSection(&m_DataSection);

	return ret;
}

BOOL CDataStreamBuf::GetDataCore(BYTE *pBuf, DWORD dwBufSize, DWORD *lpNum)
{
	DWORD dwtotalread = 0;
	DWORD dwcurread = 0;
	DWORD wantread = dwBufSize;
	BYTE *ptr = pBuf;

	if (m_pDataList)
	{
		while (!m_pDataList->empty() && wantread > 0)
		{
			PDATABLOCK pDataBlock = m_pDataList->front();
			dwcurread = min((pDataBlock->m_dwTotalSize - pDataBlock->m_dwIdx), wantread);
			memcpy(ptr, pDataBlock->pBuffer + pDataBlock->m_dwIdx, dwcurread);
			pDataBlock->m_dwIdx += dwcurread;
			ptr += dwcurread;
			wantread -= dwcurread;
			dwtotalread += dwcurread;

			if (pDataBlock->m_dwIdx >= pDataBlock->m_dwTotalSize)
			{
				delete pDataBlock->pBuffer;
				delete pDataBlock;
				m_pDataList->pop_front();
			}
			else
			{
				break;
			}
		}

		*lpNum = dwtotalread;
		m_dwTotalSize -= dwtotalread;
	}


	return TRUE;

}

DWORD CDataStreamBuf::GetCurrentDataSize()
{
	EnterCriticalSection(&m_DataSection);
	DWORD p = m_dwTotalSize;
	LeaveCriticalSection(&m_DataSection);
	return p;
}

void CDataStreamBuf::AddDataFront(BYTE *pData, DWORD dwSize)
{
	if (dwSize == 0)
	{
		return;
	}

	if (m_pDataList)
	{
		PDATABLOCK pDataBlock = new DATABLOCK();
		pDataBlock->m_dwIdx = 0;
		pDataBlock->m_dwTotalSize = dwSize;
		pDataBlock->pBuffer = new BYTE[dwSize];
		memcpy(pDataBlock->pBuffer, pData, dwSize);
		m_dwTotalSize += dwSize;
		m_pDataList->push_front(pDataBlock);
	}
}
