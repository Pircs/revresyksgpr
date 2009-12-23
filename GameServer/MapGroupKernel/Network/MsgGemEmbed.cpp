#include "AllMsg.h"
#include "User.h"
#include "MapGroup.h"


void CMsgGemEmbed::Process(CUser* pUser)
{
#ifdef _MSGDEBUG
	::LogSave("Process MsgItem, idUser:%u", 
								m_idUser);
#endif
	if(!pUser)
		return;

	if(!pUser->IsAlive())
	{
		pUser->SendSysMsg(STR_DIE);
		return ;
	}

	switch(m_usAction)
	{
	case GEM_EMBED:
		pUser->EmbedGem(m_idItem, m_idGem, m_usPos);
		break;
	case GEM_TAKEOUT:
		pUser->TakeOutGem(m_idItem, m_usPos);
		break;
	default:
		break;
	}
}