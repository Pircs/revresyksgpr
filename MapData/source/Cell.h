#pragma once

//-------------------------------------------------------------
// header files ...
#include "SharedBaseFunc.h"
#include "MyHeap.h"
//-------------------------------------------------------------

const int	LAYER_TOP			= -1;				// ȱʡɾ������LAYER�ı�־
//
const int	MASK_SEARCHED		= 0x80;				// searched flag
const int	MASK_ROLECOUNT		= 0x7F;				// count of role
const int	MASK_MASK			= 0x0001;			// layer mask
const int	MASK_ALTITUDE		= 0xFFFE;			// layer alt
const int	MAX_IDXLAYER		= 0xFFFF;			// ���Ӳ�CELL����
const int	IDXLAYER_NONE		= USHORT(-1);
const int	DEFAULT_FLOORATTR	= 0;				// ȱʡ�ĵر�����
const int	ALTITUDE_LIMIT		= 32767;			// ���޸߶�(���ڷǷ��߶�)

#pragma pack(push)
#pragma pack(1)
class CCell
{
private:
	SHORT		m_nAlt2Mask;						// alt*2 and m_bMasked
	UCHAR		m_nCountFlag;					// rule count and m_bSearched
public:
	CCell();
	~CCell();
	bool	Create(int nAlt, DWORD dwMask);

	void    Destory();

public: // 
	ULONG	Release()							{ delete this; return 0; }
public:
	int		GetFloorAttr();
	DWORD	GetFloorMask();
	int		GetFloorAlt();
	int		GetSurfaceAlt();
	void	FullMask()							{ m_nAlt2Mask |= MASK_MASK; }
	//...
	void	IncRole(int nVal = 1)				{ if ((m_nCountFlag&MASK_ROLECOUNT)+nVal<=MASK_ROLECOUNT) m_nCountFlag = (m_nCountFlag&MASK_ROLECOUNT) + nVal; }
	void	DecRole(int nVal = 1)				{ if ((m_nCountFlag&MASK_ROLECOUNT)>=nVal) m_nCountFlag = (m_nCountFlag&MASK_ROLECOUNT) - nVal; }
	int		GetRoleAmount()						{ return (m_nCountFlag&MASK_ROLECOUNT); }

public:
	BOOL            BeSearched(){return (m_nCountFlag&MASK_SEARCHED) != 0;}
	void            SetSearchFlag(bool bSearched){if(bSearched) m_nCountFlag |= MASK_SEARCHED; else m_nCountFlag &= (~MASK_SEARCHED);}
protected:
	MYHEAP_DECLARATION(s_heap)
};
#pragma pack(pop)
