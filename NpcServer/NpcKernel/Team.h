

#pragma once

#include "define.h"
#include <windows.h>
#include "Myheap.h"

#include <vector>
#include <algorithm>
#include "Msg.h"
using namespace std;

#define		_MAX_TEAMAMOUNT			5

class Msg;
class IRole;
class CTeam  
{
public:
	CTeam();
	virtual ~CTeam();

public: // constuction
	static CTeam*	CreateNew	();
	void	Release				()								{ delete this; }
	BOOL	Create				(CAgent* pAgent);
protected:
	void	Destroy				();

public: // const
	bool	IsValid				()								{ return GetMemberAmount() > 0; }
	OBJID	GetLeaderID			();

public: // application
	bool	AddMember			(int nAmount, const TeamMemberInfo* setInfo);
	BOOL	DelMember			(OBJID idMember);
	void	Dismiss				(OBJID idLeader)				{ Init(); }
	BOOL	Apply				(OBJID idUser);

protected:
	BOOL	AddMember			(OBJID idMember);
	void	Open				();
	void	Close				();
	BOOL	IsClosed			();

	TeamMemberInfo*	GetMemberByIndex	(int nIndex);
	int		GetMemberAmount		();

	BOOL	IsTeamMember		(OBJID idMember);
	BOOL	IsTeamWithNewbie	(OBJID idKiller, IRole* pTarget);


protected:
	void	AddMemberOnly		(TeamMemberInfo* pMember);
	void	DelMemberOnly		(OBJID idMember);
	void	Init				();

protected:
	typedef	vector<TeamMemberInfo>		USERIDSET;
	USERIDSET					m_setMember;
	CAgent*						m_pOwner;

private:
	BOOL						m_fClosed;

protected: // ctrl
	MYHEAP_DECLARATION(s_heap)
};


