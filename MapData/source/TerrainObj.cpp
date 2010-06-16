//-------------------------------------
// scene.cpp
//-------------------------------------
#pragma warning(disable:4786)

#include "TerrainObj.h"
#include "common.h"
#include "string.h"
#include "LOGFILE.h"
//-------------------------------------

//-------------------------------------

CTerrainObj::~CTerrainObj()
{
	this->Destory();
}

//-------------------------------------
CTerrainObj* CTerrainObj::CreateNew(LPCTSTR pszFile, OBJID idOwner /*= ID_NONE*/)
{
	if(!pszFile)
		return NULL;

	FILE* fp = fopen(pszFile, "rb");
	if(!fp)
	{
		LOGERROR("�޷��򿪵�ͼ����ļ�[%s]", pszFile);
		return NULL;
	}

	CTerrainObj* ptr = new CTerrainObj;
	if(!ptr)
	{
		fclose(fp);
		return NULL;
	}

	ptr->m_idOwner = idOwner;

	// load scenepart amount...
	fclose(fp);

	return ptr;
}

//-------------------------------------
void CTerrainObj::SetPos(POINT posCell)
{
	m_posCell = posCell;
}

//-------------------------------------

void CTerrainObj::Destory()
{
}