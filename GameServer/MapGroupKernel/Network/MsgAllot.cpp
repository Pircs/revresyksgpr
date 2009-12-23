#include "AllMsg.h"
#include "mapgroup.h"

void CMsgAllot::Process(CMapGroup& mapGroup, SOCKET_ID idSocket, OBJID idNpc)
{
	// get obj
	IRole* pRole	= mapGroup.GetRoleManager()->QueryRole(idSocket, idNpc);
	if (!pRole)
		return;

	// get User
	CUser* pUser = NULL;
	pRole->QueryObj(OBJ_USER, IPP_OF(pUser));

	if (!pUser)
		return;

	if (pUser->IsAutoAllot() && pUser->GetLev() <= MASTER_USERLEV)
		return;

	int nSum	= m_ucForce + m_ucHealth + m_ucSoul + m_ucDexterity;
	if (pUser->GetAddPoint() < nSum)
	{
		pUser->SendSysMsg(STR_CHEAT);
		return;
	}

	pUser->IncAddPoint(-1*nSum);
	pUser->SetForce(m_ucForce+pUser->GetForce());
	pUser->SetHealth(m_ucHealth+pUser->GetHealth());
	pUser->SetSoul(m_ucSoul+pUser->GetSoul());
	pUser->SetDexterity(m_ucDexterity+pUser->GetDexterity());
}