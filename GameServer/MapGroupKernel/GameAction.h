
#pragma once


class CGameAction  
{
protected:
	CGameAction(PROCESS_ID idProcess)				{ m_idProcess = idProcess; }
	virtual ~CGameAction()							{}
public:
	static CGameAction* CreateNew(PROCESS_ID idProcess)		{ return new CGameAction(idProcess); }
	void	Release()								{ delete this; }

public:
	bool	ProcessAction	(OBJID idAction, CUser* pUser=NULL, IRole* pRole=NULL, CItem* pItem=NULL, LPCTSTR pszAccept=NULL);
protected:
	void	ReplaceAttrStr		(char* pszTarget, const char* pszSource, CUser* pUser, IRole* pRole, CItem* pItem, LPCTSTR pszAccept=NULL);
	bool	ProcessActionSys	(CActionData* pAction, LPCTSTR szParam, CUser* pUser, IRole* pRole, CItem* pItem, LPCTSTR pszAccept);
	bool	ProcessActionNpc	(CActionData* pAction, LPCTSTR szParam, CUser* pUser, IRole* pRole, CItem* pItem, LPCTSTR pszAccept);
	bool	ProcessActionMap	(CActionData* pAction, LPCTSTR szParam, CUser* pUser, IRole* pRole, CItem* pItem, LPCTSTR pszAccept);
	bool	ProcessActionItemOnly	(CActionData* pAction, LPCTSTR szParam, CUser* pUser, IRole* pRole, CItem* pItem, LPCTSTR pszAccept);
	bool	ProcessActionItem	(CActionData* pAction, LPCTSTR szParam, CUser* pUser, IRole* pRole, CItem* pItem, LPCTSTR pszAccept);
	bool	ProcessActionNpcOnly	(CActionData* pAction, LPCTSTR szParam, CUser* pUser, IRole* pRole, CItem* pItem, LPCTSTR pszAccept);
	bool	ProcessActionSyn	(CActionData* pAction, LPCTSTR szParam, CUser* pUser, IRole* pRole, CItem* pItem, LPCTSTR pszAccept);
	bool	ProcessActionUser	(CActionData* pAction, LPCTSTR szParam, CUser* pUser, IRole* pRole, CItem* pItem, LPCTSTR pszAccept);
	bool	ProcessActionEvent	(CActionData* pAction, LPCTSTR szParam, CUser* pUser, IRole* pRole, CItem* pItem, LPCTSTR pszAccept);
	bool	ProcessActionWanted	(CActionData* pAction, LPCTSTR szParam, CUser* pUser, IRole* pRole, CItem* pItem, LPCTSTR pszAccept);
	bool	ProcessActionMonster(CActionData* pAction, LPCTSTR szParam, CUser* pUser, IRole* pRole, CItem* pItem, LPCTSTR pszAccept);
	bool	ProcessActionMagic	(CActionData* pAction, LPCTSTR szParam, CUser* pUser, IRole* pRole, CItem* pItem, LPCTSTR pazAccept);

private:
	CUser*	m_pUser;
	IRole*	m_pRole;
	CItem*	m_pItem;

protected:
	PROCESS_ID	m_idProcess;

protected: // ctrl
	MYHEAP_DECLARATION(s_heap)
};


