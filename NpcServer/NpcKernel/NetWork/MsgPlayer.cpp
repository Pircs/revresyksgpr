#include "AllMsg.h"
#include "NpcWorld.h"

void CMsgPlayer::Process(OBJID idNpc)
{
#ifdef _MSGDEBUG
	::LogMsg("Process CMsgPlayer, id:%u", m_id);
#endif

	if(UserManager()->QuerySet()->DelObj(m_id))		// 如果已经有此对象了，复盖
	{
	}

	char szName[_MAX_NAMESIZE]		="";

	m_StrPacker.GetString(0, szName, _MAX_NAMESIZE);

	ST_USERINFO	info;
	memset(&info, 0, sizeof(ST_USERINFO));
	info.id				= m_id;
	SafeCopy(info.szName, szName, _MAX_NAMESIZE);
	info.dwLookFace		= m_dwLookFace;
	info.usHair			= m_usHair;

	uint64	i64Effect		= m_dwStatus[1];//m_dwEffect[1];
	info.i64Effect		= (i64Effect << 32) + m_dwStatus[0];//m_dwEffect[0];

	info.idSyn			= m_dwSynID_Rank & MASK_SYNID;
	info.nRank			= m_dwSynID_Rank >> MASK_RANK_SHIFT;

	info.dwMantleType	= m_dwMantleType;
	info.dwArmorType	= m_dwArmorType;
	info.dwWeaponRType	= m_dwWeaponRType;
	info.dwMountType	= m_dwMountType;

	info.usPosX			= m_usPosX;
	info.usPosY			= m_usPosY;
	info.ucDir			= m_ucDir;
	info.ucPose			= m_ucPose;

	info.ucSizeAdd		= 0;
	int nTransLook	= info.dwLookFace / MASK_CHANGELOOK;
	if(nTransLook > 0)
		info.ucSizeAdd	= CUserManager::GetSizeAdd(nTransLook);

	INpc* pNpc = NpcManager()->QueryNpc(idNpc);
	IF_NOT(pNpc)
	{
		LOGWARNING("CMsgPlayer 没有找到接收消息的NPC，可能是game server重启动。");
		return;			// unknown map
	}

	if(NpcManager()->QueryNpc(info.id))
		return ;		// is not user

	OBJID idMap = pNpc->GetMapID();
	ASSERT(idMap);
	if(idMap == ID_NONE)
		return;

	CUser* pUser = CUser::CreateNew();		// VVVVVVVVVVVVVVVV
	ASSERT(pUser);
	if(pUser->Create(&info, idMap))
	{
		UserManager()->QuerySet()->AddObj(pUser);
		return ;
	}
	else
	{
		pUser->ReleaseByOwner();
		ASSERT(!"pUser->Create()");
	}
}