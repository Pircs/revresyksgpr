
#pragma once

#include <windows.h>
#include <stdio.h>

#include "define.h"
#include "SharedBaseFunc.h"
#include "common.h"

char	szID[];

class CGameObj  
{
public:
	CGameObj();
	virtual ~CGameObj();
	
	virtual	OBJID	GetID()						PURE_VIRTUAL_FUNCTION_0

	virtual int		GetObjType()			{return m_nObjType;}
	virtual void	SetObjType(int nType)	{m_nObjType=nType;}

	static BOOL SafeCheck	(CGameObj* pObj);

private:
	int		m_nObjType;
};



