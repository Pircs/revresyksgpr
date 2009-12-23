#pragma once

#include "mycom.h"
#include "T_SingleMap.h"
#include "VarType.h"
#include "I_AiCenter.h"	// Added by ClassView
#include "TickOver.h"
#include "autoptr.h"

#include "ctype.h"
const char	LOGIC_NOT_CHAR				= '!';
const char	KEY_PARAM_CHAR				= '<';
const char	KEY_PARAM_CHAR2				= '>';
const char	DATA_PARAM_CHAR				= '(';
const char	DATA_PARAM_CHAR2			= ')';
const char	SEPARATOR_CHAR				= ',';
const char	NEGATE_CHAR					= '-';
const char	STRING_CHAR					= '"';
const char	UNDERLINE_CHAR				= '_';

const char	ANY_PARAM_STRING[]			= "_";


bool AnalyzeFact(CSymbolTable* pSymbolTable, StringRef strMode, CFact* pFact, CFact* pPattern=NULL);
bool AnalyzeFactSet(CSymbolTable* pSymbolTable, StringRef strMode, CFactArray* psetFacts);

bool ReplaceSymbol(CFact* pFact, ARGUMENT_SET_REF setArgument);