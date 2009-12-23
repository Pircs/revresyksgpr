#include "windows.h"
#include "define.h"
#include "GameBlock.h"

CGameBlock::CGameBlock()
{
	m_pSet = NULL;
}

CGameBlock::~CGameBlock()
{
	if(m_pSet)
		m_pSet->Release();
}

bool CGameBlock::Create()
{
	if(m_pSet)
		return true;

	m_pSet = CThingSet::CreateNew(false);
	CHECKF(m_pSet);

	m_bDormancy = true;

	return true;
}