#pragma once

#include "define.h"
#include "VarType.h"
#include "Index.h"

const int	SYSTEM_FUNCTION_ID_BEGIN	=    1001;
const int	SYSTEM_FUNCTION_ID_END		=   10000;

const int	GAME_FUNCTION_ID_BEGIN		=  100001;
const int	GAME_FUNCTION_ID_END		=  200000;

const int	GAME_ACTION_ID_BEGIN		=  200001;
const int	GAME_ACTION_ID_END			=  300000;

const int	SYSTEM_FACT_ID_BEGIN		= 1000001;


class CSymbolTable  
{
	COM_STYLE(CSymbolTable)
public:
	CSymbolTable();
	virtual ~CSymbolTable();

public: // function symbol
	bool	AddSymbol(StringRef strSymbol, OBJID id)	{ LOGDUMP("Add Symbol: %u, %s.", id, (LPCTSTR)strSymbol); return m_setSymbol.Add(strSymbol, id); }

public: // fact symbol
	bool	Create(OBJID idSymbolBegin);
	OBJID	GetSymbolID(StringRef strSymbol)			{ AddSymbol(strSymbol); return m_setSymbol[strSymbol]; }
	LPCTSTR	FindSymbolByID(OBJID id);

protected:
	bool	AddSymbol(StringRef strSymbol);
	OBJID	SpawnSymbolID()								{ return m_idNext++; }

protected:
	typedef	Index<String, OBJID>	SYMBOL_SET;
	SYMBOL_SET		m_setSymbol;
	OBJID			m_idNext;
};