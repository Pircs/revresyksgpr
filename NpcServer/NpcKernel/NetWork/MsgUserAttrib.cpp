#include "AllMsg.h"
#include "NpcWorld.h"

void CMsgUserAttrib::Process()
{
	IRole* pRole = UserManager()->QueryRole(m_idUser);
	if(!pRole)
		return ;

	for(int i = 0; i < m_setUserAttrib.GetSize(); i++)
	{
		switch(m_setUserAttrib[i].ucAttributeType)
		{
		case	_USERATTRIB_LIFE:
			pRole->SetLife(m_setUserAttrib[i].dwAttributeData);
			break;
		case	_USERATTRIB_MANA:
			pRole->SetPower(m_setUserAttrib[i].dwAttributeData);
			break;
		case	_USERATTRIB_KEEPEFFECT:
			{
				uint64 i64Effect = pRole->GetEffect() & 0xFFFFFFFF00000000;
				pRole->SetEffect(m_setUserAttrib[i].dwAttributeData +i64Effect);
			}
			break;
		case	_USERATTRIB_KEEPEFFECT2:
			{
				uint64 i64Effect = m_setUserAttrib[i].dwAttributeData;
				i64Effect = (i64Effect << 32) + (pRole->GetEffect() & 0x00000000FFFFFFFF);
				pRole->SetEffect(i64Effect);
			}
			break;
		case	_USERATTRIB_SIZEADD:
			pRole->SetSizeAdd(m_setUserAttrib[i].dwAttributeData);
			break;
		}
	}
}