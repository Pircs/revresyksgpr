
#pragma once

#include "Weather.h"

class CWeatherRegion : IWeatherOwner
{
protected:
	CWeatherRegion();
	virtual ~CWeatherRegion();
public:
	static CWeatherRegion* CreateNew()		{ return new CWeatherRegion; }
	void	ReleaseByOwner()				{ delete this; }
	OBJID	GetID()							{ return m_pData->GetID(); }
	bool	Create(CRegionData* pData, PROCESS_ID idProcess);
	CWeather* QueryWeather()				{ CHECKF(m_pWeather); return m_pWeather; }
	CRegionData* QueryRegion()				{ CHECKF(m_pData); return m_pData; }

public: // interface
	virtual void	BroadcastMsg(Msg* pMsg, CUser* pExclude=NULL);

protected:
	CWeather*		m_pWeather;
	CRegionData*	m_pData;

protected: // ctrl
	PROCESS_ID		m_idProcess;

	MYHEAP_DECLARATION(s_heap)
};

typedef	IGameObjSet<CWeatherRegion>		IWeatherSet;
typedef	CGameObjSet<CWeatherRegion>		CWeatherSet;


