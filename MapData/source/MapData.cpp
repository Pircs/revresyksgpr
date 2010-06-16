//----------------------------------------------------//
// InterActiveLayer.cpp
//----------------------------------------------------//
#include "MapData.h"
#include "TerrainObj.h"
#include "common.h"
#include "inifile.h"
#include "I_MapData.h"
#include "LOGFILE.h"
#include <math.h>

//----------------------------------------------------//
CMapData::CMapData()
{
	m_setPassage.clear();
}
//----------------------------------------------------//
CMapData::~CMapData()
{
	this->ClearNodeSet();
	this->ClearCell();
	this->ClearPassage();
	this->ClearMapObj();
}
//----------------------------------------------------//
CMapData* CMapData::CreateNew(LPCTSTR pszFileName)
{
	if(!pszFileName)
		return NULL;
	FILE* fp = fopen(pszFileName, "rb");
	if(!fp)
	{
		LOGERROR("无法打开地图文件[%s]", pszFileName);
		return NULL;
	}

	CMapData* ptr = new CMapData;
	if(!ptr)
	{
		fclose(fp);
		return NULL;
	}

	// get puzzle file name ...
	char szFileName[_MAX_PATH];
	fread(szFileName, sizeof(char), _MAX_PATH, fp);
	if(	!ptr->LoadSurfaceCellData(fp))
	{
		LOGERROR("地图文件[%s]数据装载错误！", pszFileName);

		S_DEL(ptr);
		fclose(fp);
		return NULL;
	}

	fclose(fp);

	return ptr;
}
//-------------------------------------------------------------
bool CMapData::LoadSurfaceCellData(FILE* fp)
{
	SIZE infoSize = {256, 256};
	m_sizeMap		= infoSize;
	m_setCell.resize(256*256);
	return true;
}
//----------------------------------------------------//
bool CMapData::LoadDataPassage(FILE* fp)
{
	if(!fp)
		return false;

	int nAmount;
	fread(&nAmount, sizeof(int), 1, fp);
	for(int i = 0; i < nAmount; i++)
	{
		PassageInfo info;

		fread(&info, sizeof(PassageInfo), 1, fp);
		AddPassage(&info);
	}

	return true;
}

//----------------------------------------------------//
void CMapData::ClearCell()
{
	m_setCell.clear();
}
//----------------------------------------------------//
void CMapData::ClearPassage()
{
	int nAmount =  m_setPassage.size();
	for(int i = 0; i < nAmount; i++)
	{
		PassageInfo* pInfo = m_setPassage[i];
		S_DEL(pInfo);
	}
	m_setPassage.clear();
}
//----------------------------------------------------//
void CMapData::ClearMapObj()
{
	int nAmount =  m_setMapObj.size();
	for(int i = 0; i < nAmount; i++)
	{
		CTerrainObj* pInfo = m_setMapObj[i];
		S_DEL(pInfo);
	}
	m_setMapObj.clear();
}

//----------------------------------------------------//
CCell*  CMapData::GetCell(int nIndex)
{
	int nAmount = m_setCell.size();
	if((nIndex < 0) || (nIndex >= nAmount))
		return NULL;
	return &m_setCell[nIndex];
}
//----------------------------------------------------//
CCell*  CMapData::GetCell(int nCellX, int nCellY)
{
	return GetCell(POS2INDEX(nCellX, nCellY, m_sizeMap.cx, m_sizeMap.cy));
}
//----------------------------------------------------//
void CMapData::AddPassage(const PassageInfo* pInfo)
{
	CHECK(pInfo);

	POINT	posCell;
	posCell.x	= pInfo->nPosX;
	posCell.y	= pInfo->nPosY;
	AddPassage(posCell, pInfo->nIndex);
}
//----------------------------------------------------//
void CMapData::AddPassage(POINT posMap, int nIndex)
{
	// 兼容于地图编辑器
	{
		int nAmount =  m_setPassage.size();
		for(int i = 0; i < nAmount; i++)
		{
			PassageInfo* pInfo = m_setPassage[i];
			if(pInfo)
			{
				if((pInfo->nPosX == posMap.x) && (pInfo->nPosY == posMap.y))
				{
					pInfo->nIndex = nIndex;
					return;
				}
			}
		}
	}

	PassageInfo* pInfo = new PassageInfo;
	if(pInfo)
	{
		pInfo->nPosX = posMap.x;
		pInfo->nPosY = posMap.y;
		pInfo->nIndex = nIndex;
		m_setPassage.push_back(pInfo);
	}
}
//----------------------------------------------------//
void CMapData::DelPassage(POINT posMap)
{
	int nAmount =  m_setPassage.size();
	for(int i = 0; i < nAmount; i++)
	{
		PassageInfo* pInfo = m_setPassage[i];
		if(pInfo)
		{
			if((pInfo->nPosX == posMap.x) && (pInfo->nPosY == posMap.y))
			{
				S_DEL(pInfo);
				m_setPassage.erase(m_setPassage.begin()+i);
				// 兼容于编辑器	return ;
			}
		}
	}
}
//----------------------------------------------------//
int  CMapData::GetExitIndex(POINT posCell)			// return -1 : error
{
	int nAmount =  m_setPassage.size();
	for(int i = 0; i < nAmount; i++)
	{
		PassageInfo* pInfo = m_setPassage[i];
		if(pInfo)
		{
			if((pInfo->nPosX == posCell.x) && (pInfo->nPosY == posCell.y))
			{
				return pInfo->nIndex;
			}
		}
	}
	return -1;
}
//----------------------------------------------------//
bool CMapData::AddMapObj(CTerrainObj* pObj)
{
	CHECKF(pObj);

	m_setMapObj.push_back(pObj);

	return true;
}

bool CMapData::DelMapObj(int idx)
{
	CHECKF(idx >= 0 && idx < m_setMapObj.size());

	CTerrainObj* pObj = m_setMapObj[idx];
	if(pObj)
	{
		S_DEL(pObj);
		m_setMapObj.erase(m_setMapObj.begin() + idx);
		return true;
	}

	return false;
}

/////////////////
// interface/////////////////
bool CMapData::AddTerrainItem(OBJID idOwner, int nCellX, int nCellY, OBJID idTerrainItemType)
{
	CHECKF(idTerrainItemType != ID_NONE);

	char	szKey[256];
	sprintf(szKey, "NpcType%u", idTerrainItemType/10);
	CIniFile	ini("ini\\TerrainNpc.ini", szKey);
	char	szField[256];
	sprintf(szField, "Dir%u", idTerrainItemType%10);
	char	szFileName[256];
	if(!ini.GetString(szFileName, szField, 256))
	{
		LOGERROR("Can't find TerrainNpc[%d] in ini\\TerrainNpc.ini", idTerrainItemType);
		return NULL;
	}

	CTerrainObj* pObj = CTerrainObj::CreateNew(szFileName, idOwner);
	if(!pObj)
		return false;

	POINT	posCell;
	posCell.x	= nCellX;
	posCell.y	= nCellY;
	pObj->SetPos(posCell);

	if(!AddMapObj(pObj))
	{
		S_DEL(pObj);
		return false;
	}

	return true;
}
/////////////////
bool CMapData::DelTerrainItem(OBJID idOwner)
{
	for(int i = m_setMapObj.size()-1; i >= 0; i--)
	{
		CTerrainObj* pObj = m_setMapObj[i];
		if(pObj && pObj->GetOwnerID() == idOwner)
		{
			if(!DelMapObj(i))		// 注意：如果一个OWNER有多个物件时，要先放关键物件，否则会部分删除。
				return false;
		}
	}

	return true;
}
/////////////////
int  CMapData::GetMapWidth()
{
	return m_sizeMap.cx;
}
/////////////////
int	 CMapData::GetMapHeight()
{
	return m_sizeMap.cy;
}
/////////////////
CCell* CMapData::QueryCell(int nCellX, int nCellY)
{
	CHECKF(nCellX >= 0 && nCellX < m_sizeMap.cx);
	CHECKF(nCellY >= 0 && nCellY < m_sizeMap.cy);

	return GetCell(nCellX, nCellY);
}
/////////////////
CCell* CMapData::QueryCell(POINT posCell)
{
	return QueryCell(posCell.x, posCell.y);
}
/////////////////
int CMapData::GetPassage(int x, int y)
{
	for(int i = 0; i < m_setPassage.size(); i++)
	{
		if(m_setPassage[i]->nPosX == x && m_setPassage[i]->nPosY == y)
		{
			return m_setPassage[i]->nIndex;
		}
	}

	return PASSAGE_NONE;
}
/////////////////
// global entry/////////////////
#include "CSVFile.h"
IMapData* IMapData::CreateNew(int nMapDoc)
{
	CHECKF(nMapDoc != ID_NONE);
	CCsvFile csv;
	if (csv.Open("csv/GameMap.csv"))
	{
		csv.Seek("ID",nMapDoc);
		std::string strMapFile	= csv.GetStr("File");
		IMapData*	pObj = CMapData::CreateNew(strMapFile.c_str());
		csv.Close();
		return pObj;
	}
	return NULL;
}

