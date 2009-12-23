#include "allmsg.h"
#include "Npc.h"
#include "NpcManager.h"
#include "GameMap.h"
#include "MapGroup.h"


bool CMsgNpcInfoEx::Create(CNpc* pNpc)
{
	m_id			= pNpc->GetID();
	m_usType		= pNpc->GetType();
	m_usSort		= pNpc->GetSort();
	m_usLook		= pNpc->GetLookFace();
	m_usCellX	= pNpc->GetPosX();
	m_usCellY	= pNpc->GetPosY();
	m_dwMaxLife	= pNpc->GetMaxLife();
	m_dwLife		= pNpc->GetLife();
	m_cLength	= pNpc->GetLength();
	m_cFat		= pNpc->GetFat();
//	m_StrPacker.AddString(pNpc->GetName());

	// with name
	if(pNpc->IsSynFlag())
	{
		m_StrPacker.AddString(pNpc->GetName());
	}
	UPDATE_MSG_SIZE
	return true;
}

bool CMsgNpcInfoEx::Create(OBJID id, DWORD dwMaxLife, DWORD dwLife, int nType, int nSort,
							int nLookFace, int nCellX, int nCellY, int nLength, int nFat, const char* pszName/*=NULL*/)
{
	m_id			= id;
	m_dwMaxLife	= dwMaxLife;
	m_dwLife		= dwLife;
	m_usType		= (unsigned short)nType;
	m_usSort		= (unsigned short)nSort;
	m_usLook		= (unsigned short)nLookFace;
	m_usCellX	= (unsigned short)nCellX;
	m_usCellY	= (unsigned short)nCellY;
	m_cLength	= (char)nLength;
	m_cFat		= (char)nFat;

	// with name
	if(pszName)
	{
		m_StrPacker.AddString(pszName);
	}
	UPDATE_MSG_SIZE
	return true;
}
