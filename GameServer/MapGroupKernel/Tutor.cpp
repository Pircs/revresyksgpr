#include "Tutor.h"

MYHEAP_IMPLEMENTATION(CTutor, s_heap)

CTutor::CTutor()
{
}

CTutor::~CTutor()
{
}

CTutor*	CTutor::CreateNewTutor(const TutorInfoStruct* pInfo)
{
	CHECKF(pInfo);
	CTutor* pTutor	= CTutor::CreateNew();
	if (pTutor)
	{
		memcpy(&pTutor->m_Info, pInfo, sizeof(TutorInfoStruct));
	}
	return pTutor;
}