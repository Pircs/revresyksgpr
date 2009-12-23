#pragma once

#include <windows.h>
#include <stdio.h>

#include "define.h"
#include "basefunc.h"

char	szID[];

class CGameObj  
{
public:
	CGameObj();
	virtual ~CGameObj();
	
	virtual	OBJID	GetID()	{::LogSave("Fatal error in CGameObj::GetID()."); return ID_NONE;}//=0;

	virtual int		GetObjType()	{return m_nObjType;}
	virtual void	SetObjType(int nType)	{m_nObjType=nType;}

	static BOOL SafeCheck	(CGameObj* pObj);

private:
	int		m_nObjType;
};



