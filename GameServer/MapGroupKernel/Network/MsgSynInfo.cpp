#include "AllMsg.h"
#include "Syndicate.h"
#include "MapGroup.h"
#pragma	warning(disable:4786)

bool CMsgSynInfo::Create(CSyndicate* pSyn)		// pSyn may be null
{
	m_idSyn			= pSyn->GetID();
	m_idFealty		= pSyn->GetInt(SYNDATA_FEALTY);
	m_dwSyndicateFund		= pSyn->GetInt(SYNDATA_MONEY);
	m_dwSyndicatePopulation	= pSyn->GetSynAmount();
	m_ucRank			= pSyn->GetInt(SYNDATA_RANK);
	m_ucSaint		= pSyn->GetInt(SYNDATA_SAINT);
	SafeCopy(m_szName, pSyn->GetStr(SYNDATA_NAME), _MAX_NAMESIZE);
	SafeCopy(m_szLeader, pSyn->GetStr(SYNDATA_LEADERNAME), _MAX_NAMESIZE);
	return true;
}