#include "AllMsg.h"
#include "mapgroup.h"

bool CMsgAnnounceInfo::Create(CAnnounceData *pData,int type)
{
	m_idAnnounce = pData->GetInt(ANNOUNCEDATA_ID);
	m_user_id = pData->GetInt(ANNOUNCEDATA_USER_ID);
	m_level = pData->GetInt(ANNOUNCEDATA_LEVEL);        // 发布公告人的等级
	m_teacher_level = pData->GetInt(ANNOUNCEDATA_TEACHER_LEVEL);	 // 导师等级
	m_profession = pData->GetInt(ANNOUNCEDATA_PROFESSION);
	m_usType = type;
	BOOL bSucMake	=true;  	
	bSucMake    &=m_StrPacker.AddString((char*)pData->GetStr(ANNOUNCEDATA_USER_NAME)); // 发布公告人名字
	//bSucMake	&=m_StrPacker.AddString((char*)pData->GetStr(ANNOUNCEDATA_PROFESSION));// 导师职业
	bSucMake	&=m_StrPacker.AddString((char*)pData->GetStr(ANNOUNCEDATA_TITLE));     // 公告标题
	bSucMake	&=m_StrPacker.AddString((char*)pData->GetStr(ANNOUNCEDATA_CONTENT));   // 公告内容
	UPDATE_MSG_SIZE;
	return true;
}

bool CMsgAnnounceInfo::Create(int level,int teacher_level,const char* name,int profession,int type)
{
	m_level = level;
	m_teacher_level = teacher_level;
	m_usType = type;
	m_profession = profession;
	BOOL bSucMake = true;
	bSucMake    &=m_StrPacker.AddString((char*)name);
	//bSucMake    &=m_StrPacker.AddString(profession);
	UPDATE_MSG_SIZE;
	return true;
}

void CMsgAnnounceInfo::Process(CMapGroup& mapGroup, SOCKET_ID idSocket, OBJID idNpc)
{
	CUser* pUser = mapGroup.GetUserManager()->GetUser(idSocket, idNpc);
	//CUser* pUser = g_UserManager.GetUser(m_user_id);
	switch(m_usType)
	{
	case _ANNOUNCE_ISSUE:
		if(mapGroup.GetAnnounce()->QueryAnnounceDataByOwner(pUser->GetID()))
		{
			pUser->SendSysMsg(STR_DUPLICATE_ANNOUNCE);
			return ;
		}
		ST_ANNOUNCE_DATA	tAnnounceData;		
		tAnnounceData.level = pUser->GetLev();
		tAnnounceData.user_id = pUser->GetID();
		tAnnounceData.teacherlevel = pUser->GetTutorLevel();
		if(tAnnounceData.teacherlevel < 1)
		{
			pUser->SendSysMsg(STR_LOW_GRADE);
			return ;
		}
/*		switch(pUser->GetProfession())
		{
		case 10:
            strcpy(tAnnounceData.profession,"魔法师");// 魔法师  11,			// 魔法师转职
			break;
		case 20:
            strcpy(tAnnounceData.profession,"战士");// 战士  21,			// 战士转职
			break;
		case 30:
			strcpy(tAnnounceData.profession,"异能者");	// 异能者  31,			// 异能者转职	
			break;
		}*/
		tAnnounceData.profession = pUser->GetProfession();
		sprintf(tAnnounceData.name,"%s",pUser->GetName());

		m_StrPacker.GetString(0, tAnnounceData.title, 32);
		m_StrPacker.GetString(1, tAnnounceData.content, 128);
	/*	if(strlen(tAnnounceData.title) > 32)
		{
			pUser->SendSysMsg("标题过长");
		    return;
		}
		if(strlen(tAnnounceData.content) > 128 )
		{
			pUser->SendSysMsg("公告过长");
			return ;
		}*/
		if(mapGroup.GetAnnounce()->CreateNewAnnounce(&tAnnounceData,mapGroup.GetDatabase()))
			pUser->SendSysMsg(STR_ANNOUNCE_SUCCEED);
		break;
	case _ANNOUNCE_SLEF_CHECK:
		mapGroup.GetAnnounce()->SendAnnounceSelf(mapGroup.GetRoleManager()->QueryRole(m_user_id));
		break;
	case _ANNOUNCE_CHECK:
		mapGroup.GetAnnounce()->SendAnnounceInfo(mapGroup.GetRoleManager()->QueryRole(pUser->GetID()/*m_user_id*/),m_idAnnounce);
		break;
	case _ANNOUNCE_USER_INFO:
	/*	char nProfession[16];
		switch(pUser->GetProfession())
		{
		case 10:
            strcpy(nProfession,"魔法师");// 魔法师 
			break;
		case 20:
            strcpy(nProfession,"战士");// 战士 
			break;
		case 30:
			strcpy(nProfession,"异能者");	// 异能者
			break;
		}*/
        if(mapGroup.GetAnnounce()->QueryAnnounceDataByOwner(pUser->GetID()))
		{
			CMsgAnnounceInfo msg;
			IF_OK(msg.Create(pUser->GetLev(),pUser->GetTutorLevel(), pUser->GetName(), pUser->GetProfession(),_ANNOUNCE_USER_INFO))
				mapGroup.QueryIntraMsg()->SendClientMsg(idSocket, &msg);//SendMsg(&msg);
		}
		else 
			pUser->SendSysMsg(STR_NO_ANNOUNCE);
		break;
	case _ANNOUNCE_CANCEL:
        if(mapGroup.GetAnnounce()->DeleteAnnounceByObjID(m_idAnnounce))
			pUser->SendSysMsg(STR_DELETE_ANNOUNCE);
		else
			pUser->SendSysMsg(STR_NO_ANNOUNCE);
		break;
	}
}