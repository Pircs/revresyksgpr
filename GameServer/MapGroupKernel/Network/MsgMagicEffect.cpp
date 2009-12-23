#include "AllMsg.h"


bool CMsgMagicEffect::Create(OBJID idUser, int nType, int nLevel, OBJID idTarget, UCHAR ucDir)
{
	CHECKF(idUser != ID_NONE && idTarget != ID_NONE);

	m_idUser		= idUser;
	m_usType		= nType;
	m_usLevel		= nLevel;
	m_idTarget		= idTarget;
	m_ucDir			= ucDir;
	return true;
}

bool CMsgMagicEffect::Create(OBJID idUser, int nType, int nLevel, OBJID idTarget, DWORD dwData, UCHAR ucDir)
{
	Create(idUser, nType, nLevel, idTarget, ucDir);
//	if(dwData)
	{
		AppendRole(idTarget, dwData);
	}
	return true;
}

bool CMsgMagicEffect::CreateByPos(OBJID idUser, int nType, int nLevel, int x, int y, UCHAR ucDir, OBJID idTarget/*=ID_NONE*/, DWORD dwData/*=0*/)
{
	CHECKF(idUser != ID_NONE);

	m_idUser		= idUser;
	m_usType		= nType;
	m_usLevel		= nLevel;
	m_usPosX		= x;
	m_usPosY		= y;
	m_ucDir			= ucDir;
	if(idTarget != ID_NONE)
	{
		AppendRole(idTarget, dwData);
	}
	return true;
}

bool CMsgMagicEffect::CreateCollide(OBJID idUser, int nType, int nLevel, OBJID idTarget, DWORD dwData, int nCollideDir, UCHAR ucDir)
{
	CHECKF(idUser != ID_NONE && idTarget != ID_NONE);

	m_idUser		= idUser;
	m_usType		= nType;
	m_usLevel		= nLevel;
	m_ucCollideDir	= nCollideDir;
	m_ucDir			= ucDir;
//	if(dwData)
	{
		AppendRole(idTarget, dwData);
	}
	return true;
}

bool CMsgMagicEffect::AppendRole(OBJID idRole, DWORD dwData)
{
	CHECKF(idRole != ID_NONE);

	EffectRoleStruct effectRole;
	effectRole.idRole	= idRole;
	effectRole.dwData	= dwData;
	m_setEffectRole.Append(effectRole);

	return true;
}