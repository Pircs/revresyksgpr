
#pragma once

#include "TutorData.h"
#include "Myheap.h"


class CTutor : public CTutorData  
{
public:
	CTutor();
	virtual ~CTutor();

	ULONG	ReleaseByOwner()		{ delete this; return 0; }

public:
	static CTutor*	CreateNewTutor(const TutorInfoStruct* pInfo);
	static CTutor*	CreateNew()		{ return new CTutor; }

protected:
	MYHEAP_DECLARATION(s_heap)
};


