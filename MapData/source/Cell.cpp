///////////////////////////////////////
// cell.cpp
///////////////////////////////////////
#pragma warning(disable:4786)
#include "windows.h"
#include "cell.h" 

//-------------------------------------
// static 
MYHEAP_IMPLEMENTATION(CCell,s_heap);

//-------------------------------------

CCell::CCell()
		: m_nAlt2Mask( 0 )
{
	m_nCountFlag	= 0;
}

//-------------------------------------

CCell::~CCell()
{
	this->Destory();
	ASSERT((m_nCountFlag&MASK_ROLECOUNT) == 0);
}

//-------------------------------------

bool CCell::Create(int nAlt, DWORD dwMask)
{
	m_nAlt2Mask		= ((nAlt*2) & MASK_ALTITUDE) | (dwMask ? MASK_MASK : 0);
	m_nCountFlag	= 0;
	return true;
}

//-------------------------------------

void CCell::Destory()
{
}

//-------------------------------------

DWORD CCell::GetFloorMask()
{
	return (m_nAlt2Mask&MASK_MASK) != 0; 
}

//-------------------------------------

int CCell::GetFloorAttr()
{
	return DEFAULT_FLOORATTR;					// 服务器暂不用地表属性(但支持LAYER附加属性)
}

//-------------------------------------

int CCell::GetFloorAlt()
{
	return short(m_nAlt2Mask&MASK_ALTITUDE) / 2;
}

//-------------------------------------

int CCell::GetSurfaceAlt()
{
	return (m_nAlt2Mask&MASK_ALTITUDE) / 2;
}

