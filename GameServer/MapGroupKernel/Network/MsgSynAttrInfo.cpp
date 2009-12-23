#include "AllMsg.h"
#include "SynAttr.h"
#include "MapGroup.h"
#pragma	warning(disable:4786)

bool CMsgSynAttrInfo::Create(SynAttrInfoStruct* pInfo, CSyndicate* pSyn)		// pSyn may be null
{
	m_idSyn			= pInfo->idSyn;
	m_ucRankShow		= pInfo->ucRankShow;
	m_nProffer		= pInfo->nProffer;
	m_usMantle		= pInfo->usMantle;
	m_ucLevel		= pInfo->ucLevel;

	if(pSyn)
	{
		m_dwSyndicateFund	= pSyn->GetInt(SYNDATA_MONEY);
		m_dwSyndicatePopulation	= pSyn->GetSynAmount();
		m_dwSyndicateRepute	 = pSyn->GetInt(SYNDATA_REPUTE);
		m_ucSynRank = pSyn->GetInt(SYNDATA_RANK);
		SafeCopy(m_szLeader, pSyn->GetStr(SYNDATA_LEADERNAME), _MAX_NAMESIZE);
	}
	else
	{
		m_dwSyndicateFund		= 0;
		m_dwSyndicatePopulation	= 0;
		m_szLeader[0]			= 0;
		m_dwSyndicateRepute = 0;
		m_ucSynRank = 0;
	}
	return true;
}