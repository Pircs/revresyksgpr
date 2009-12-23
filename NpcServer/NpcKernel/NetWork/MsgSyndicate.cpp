// ÏÉ½£ÐÞ£¬2002.1.8
#include "AllMsg.h"
#include "basefunc.h"
#include "NpcWorld.h"
#include "SynManager.h"

void CMsgSyndicate::Process(OBJID idNpc)
{
	OBJID idSyn = idNpc;		// npc id is syn id.
	CHECK(idSyn != ID_NONE);
	CSyndicate* pSyn = SynManager()->QuerySyndicate(idSyn);
	if(m_usAction == NPCMSG_CREATE_SYN)
	{
		CHECK(!pSyn);
	}
	else if(!( m_usAction == SET_WHITE_SYN || m_usAction == SET_BLACK_SYN ))
	{
		CHECK(pSyn);
	}

	switch(m_usAction)
	{
	////////////////////////////////////////////////////////////////////////////////////////////////
	case	SET_ALLY:
		{
			pSyn->QueryAllySet()->push_back(m_idTarget);
		}
		break;
	////////////////////////////////////////////////////////////////////////////////////////////////
	case	CLEAR_ALLY:
		{
			for(int i = 0; i < pSyn->QueryAllySet()->size(); i++)
			{
				if(pSyn->QueryAllySet()->at(i) == m_idTarget)
				{
					pSyn->QueryAllySet()->erase(pSyn->QueryAllySet()->begin() + i);
					break;		// mast out for
				}
			}
		}
		break;
	////////////////////////////////////////////////////////////////////////////////////////////////
	case	SET_ANTAGONIZE:
		{
			pSyn->QueryEnemySet()->push_back(m_idTarget);
		}
		break;
	////////////////////////////////////////////////////////////////////////////////////////////////
	case	CLEAR_ANTAGONIZE:
		{
			for(int i = 0; i < pSyn->QueryEnemySet()->size(); i++)
			{
				if(pSyn->QueryEnemySet()->at(i) == m_idTarget)
				{
					pSyn->QueryEnemySet()->erase(pSyn->QueryEnemySet()->begin() + i);
					break;		// mast out for
				}
			}
		}
		break;
	////////////////////////////////////////////////////////////////////////////////////////////////
	case	NPCMSG_CREATE_SYN:
		{
			CSyndicate* pSyn = CSyndicate::CreateNew();
			IF_OK(pSyn)
			{
				IF_OK(pSyn->Create(idSyn, m_idTarget))
					SynManager()->QuerySynSet()->AddObj(pSyn);
				else
					pSyn->ReleaseByOwner();
			}
		}
		break;
	////////////////////////////////////////////////////////////////////////////////////////////////
	case	NPCMSG_DESTROY_SYN:
		{
			SynManager()->QuerySynSet()->DelObj(idSyn);
		}
		break;
	default:
		return;
	}
}

