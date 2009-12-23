#include "AllMsg.h"
#include "mapgroup.h"
#include "transformation.h"
#include "Agent.h"
#include "Npc.h"
/*
BOOL CMsgDataArray::Append(int nType, DWORD dwData)
{
	CHECKF(nType >= 0 && nType < _MAX_MSGATRRIB);
	CHECKF(m_unMsgSize + sizeof(DataArrayStruct) < MAX_PACKETSIZE);

	// fill info now
	m_unMsgSize		+= sizeof(DataArrayStruct);
	m_setDataArray[m_dwAttributeNum].ucAttributeType	= nType;
	m_setDataArray[m_dwAttributeNum].dwAttributeData	= dwData;
	m_dwAttributeNum++;

	return true;
}*/

void CMsgDataArray::Process(CUser* pUser)
{
	CHECK(pUser) ;
	IF_NOT( pUser->IsAlive() )
	{
		pUser->SendSysMsg(STR_DIE) ;
		return ;
	}

	switch( m_ucAction )
	{
	case _DATAARRAY_ACTION_UPGRADE_QUALITY: //升级装备品质	
		{
			this->UpEquipQuality(pUser);
		}
		break;

	case _DATAARRAY_ACTION_UPGRADE_SUPERADDITION: //升级装备追加	
		{
			this->UpEquipSuperAddition(pUser);
		}
		break;
	case _DATAARRAY_ACTION_UPGRADE_EQUIPLEVEL: //升级装备等级
		{
			this->UpEquipLevel(pUser);
		}
		break;
	case _DATAARRAY_ACTION_EMBEDGEM://对装备嵌入宝石
		{
			this->EmbedGemToEquip(pUser);
		}
		break;
	case _DATAARRAY_ACTION_SETSYN_DRESS:	//设置帮派服装
		{
			this->SetSynDress(pUser);
		}
		break;
	default:
		break;

	}

}

int CMsgDataArray::RateSuccForGhostLevel(CItem* pEquipItem)
{
	return 50;
}

/*
通过灵魂晶石来锻造提高武器的品质，每个等级锻造的成功率不一样。
1--2，2--3，3--4，4--5的升级成功率都是100%。
普通升良（0--1）：30%成功
良升上（1--2）：12%成功
上升精（2--3）：6%成功
精升极（3--4）：4%成功

*/
int CMsgDataArray::RateSuccForQuality(CItem* pEquipItem)
{
	int iQuality = pEquipItem->GetQuality();
	
	if (iQuality==0)	return 30;
	else if(iQuality==1)	return 12;
	else if(iQuality==2)	return 6;
	else if(iQuality==3)	return 4;
	
	return 0;
}
//升级武器的等级
//该类物品TYPE的百位和十位实际上就是武器的等级，等级为0--22，一共可升级22次。
bool CMsgDataArray::UpEquipLevel(CUser* pUser)
{
	DEBUG_TRY

	OBJID idEquipment, idGem;

	IF_NOT(m_uLong.GetSize()==2) 
		return false;
	
	idEquipment = m_uLong[0] ;
	idGem = m_uLong[1] ;

	CItem* pEquipItem = pUser->GetItem(idEquipment) ;
	CItem* pGem = pUser->GetItem(idGem) ;

	IF_NOT(pGem && pEquipItem && pGem->IsGemForUpLevel())
		return false;
	
	if( pEquipItem->IsEquipment() || pEquipItem->IsWeapon() )//是否为装备或武器
	{
		//检查级别
		int nLevel = pEquipItem->GetLevel();
		if (nLevel>=22) {
			return false;
		}
		
		int nRateSucc =  RateSuccForEquipLevel(pEquipItem);
		if( RandGet(100) < nRateSucc ) //succ
		{
			IF_NOT(pEquipItem->UpLevel())
			{
				pUser->SendSysMsg(STR_UPEQUIPLEVEL_FAIL_NOTYPE);			
				return false;
			}
			pUser->EraseItem(idGem,SYNCHRO_TRUE) ; //通知对方将宝石删除 
			//通知客户端
			CMsgItemInfo msg;			
			if (msg.Create(pEquipItem,ITEMINFO_UPDATE))
				pUser->SendMsg(&msg);				
			pUser->SendSysMsg(STR_UPEQUIPLEVEL_SUCC);				
			return true;
		}
		else
		{
			pUser->EraseItem(idGem,SYNCHRO_TRUE) ; //通知对方将宝石删除 				
			pUser->SendSysMsg(STR_UPEQUIPLEVEL_FAIL);
			return false;
		}
	}
	DEBUG_CATCH("bool CMsgDataArray::UpEquipLevel()")
	return false;
}
//武器升级时的成功率
/*
武器类（包括武器，项链，手镯，戒指，鞋子等）：
该类物品TYPE的百位和十位实际上就是武器的等级，等级为0--22，一共可升级22次。但如果升级时在itemtype表中找不到对应的数据，则表示不能升级了（例如：450205，如果要升级则升级为450215，但因为数据库中没有450215的数据，所以表示该武器不能再升级了。）
升级成功率：
当等级<4时，升级成功率为100%
当等级<7时，升级成功率为：35%
当等级<10时，升级成功率为：20%
当等级<13时，升级成功率为：10%
当等级<16时，升级成功率为：7%
当等级<19时，升级成功率为：4%
当等级>=19时，升级成功率为：2%

装备类（包括衣服，帽子和盾牌）：
该类物品TYPE的十位表示等级，等级为0--9，一共可升级9次。但如果升级时在itemtype表中找不到对应的数据，则表示不能升级了。
当等级<2时，升级成功率100%
当等级<4时，升级成功率为：35%
当等级<6时，升级成功率为：20%
当等级<7时，升级成功率为：10%
当等级<8时，升级成功率为：7%
当等级<9时，升级成功率为：4%
*/
int CMsgDataArray::RateSuccForEquipLevel(CItem* pEquipItem)
{
	IF_NOT(pEquipItem)
		return 0;

	int nLevel = pEquipItem->GetLevel();
	if (pEquipItem->IsShield() || pEquipItem->IsArmor() || pEquipItem->IsHelmet())
	{
		if (nLevel>=0 && nLevel < 2)		 	    return 100;
		else if(nLevel>=2 && nLevel < 4)			return 35;
		else if(nLevel>=4 && nLevel < 6)			return 20;
		else if(nLevel>=6 && nLevel < 7)			return 10;
		else if(nLevel>=7 && nLevel < 8)			return 7;
		else if(nLevel>=8 && nLevel < 9)			return 4;		
	}
	else
	{
		if (nLevel>=0 && nLevel < 4)		 	    return 100;
		else if(nLevel>=4 && nLevel < 7)			return 35;
		else if(nLevel>=7 && nLevel < 10)			return 20;
		else if(nLevel>=10 && nLevel < 13)			return 10;
		else if(nLevel>=13 && nLevel < 16)			return 7;
		else if(nLevel>=16 && nLevel < 19)			return 4;
		else if(nLevel>=19&&nLevel<22) 			return 2;
	}	
	return 0;
}

bool CMsgDataArray::UpEquipQuality(CUser* pUser)
{
	DEBUG_TRY

	OBJID idEquipment, idGem;

	IF_NOT(m_uLong.GetSize()==2) 
		return false;
	
	idEquipment = m_uLong[0] ;
	idGem = m_uLong[1] ;

	CItem* pEquipItem = pUser->GetItem(idEquipment) ;
	CItem* pGem = pUser->GetItem(idGem) ;

	IF_NOT(pGem && pEquipItem && pGem->IsGemForUpQuality())
		return false;
	
	if( pEquipItem->IsEquipment() || pEquipItem->IsWeapon() )//是否为装备或武器
	{
		//检查品质
		int nQuality = pEquipItem->GetQuality();
		if (nQuality>=9) {
			return false;
		}
		
		//升级品质GEM
		int iRateSucc = RateSuccForQuality(pEquipItem);		
		
		if (iRateSucc==0) {//已经是最好的品质
			pUser->SendSysMsg(STR_QUALITY_UPGRADE_MAXLEVEL);
			return false;
		}
		if( RandGet(100) < iRateSucc ) //succ
		{
			IF_NOT(pEquipItem->UpQuality())
			{
				pUser->SendSysMsg(STR_ITEM_CHANGETYPE_FAILED);
				return false;
			}
			pUser->EraseItem(idGem,SYNCHRO_TRUE) ; //通知对方将宝石删除 
			//通知客户端
			CMsgItemInfo msg;			
			if (msg.Create(pEquipItem,ITEMINFO_UPDATE))
				pUser->SendMsg(&msg);
			pUser->SendSysMsg(STR_QUALITY_UPGRADE_SUCCESS);
			return true;	
		}
		else 
		{  
			//失败铸造
			pUser->EraseItem(idGem,SYNCHRO_TRUE) ; //通知对方将宝石删除 
			pUser->SendSysMsg(STR_QUALITY_UPGRADE_FAILED);
			return false;
		}		
	}	
	DEBUG_CATCH("bool CMsgDataArray::UpEquipQuality()")
	return false;
	
}
/*
2、魔魂等级：
通过魔魂晶石（魔魂晶石type：1037150）注入来提升魔魂等级，每次提升成功率为50%，如果成功，则魔魂等级+1，如果失败，则魔魂等级-1（如果目前魔魂等级为0，则不会降魔魂等级）。魔魂等级也是0--9级。

*/
bool CMsgDataArray::UpEquipSuperAddition(CUser* pUser)
{
	DEBUG_TRY

	OBJID idEquipment, idGem;

	IF_NOT(m_uLong.GetSize()==2) 
		return false;
	
	idEquipment = m_uLong[0] ;
	idGem = m_uLong[1] ;

	CItem* pEquipItem = pUser->GetItem(idEquipment) ;
	CItem* pGem = pUser->GetItem(idGem) ;

	IF_NOT(pGem && pEquipItem && pGem->IsGemForUpSuperAddition())
		return false;
	
	if( pEquipItem->IsEquipment() || pEquipItem->IsWeapon() )//是否为装备或武器
	{
		//检查追加属性
		int nSuperAddition = pEquipItem->GetSuperAddition();
		if (nSuperAddition>=9) {
			return false;
		}
		//升级追加
		int iRateSucc = RateSuccForGhostLevel(pEquipItem);		
		if( RandGet(100) < iRateSucc ) //succ
		{
			IF_NOT(pEquipItem->UpSuperAddition())
			{
				pUser->SendSysMsg(STR_GHOSTLEVEL_FAILED_NOTYPE);
				return false;
			}
			pUser->EraseItem(idGem,SYNCHRO_TRUE) ; //通知对方将宝石删除 
			//通知客户端
			CMsgItemInfo msg;			
			if (msg.Create(pEquipItem,ITEMINFO_UPDATE))
				pUser->SendMsg(&msg);
			pUser->SendSysMsg(STR_GHOSTLEVEL_SUCCESS);			
			return true;
		}
		else 
		{  //失败,降低追加属性
			if(pEquipItem->DecSuperAddition())
			{
				//通知客户端
				CMsgItemInfo msg;			
				if (msg.Create(pEquipItem,ITEMINFO_UPDATE))
					pUser->SendMsg(&msg);
			}
			pUser->EraseItem(idGem,SYNCHRO_TRUE) ; //通知对方将宝石删除 
			pUser->SendSysMsg(STR_GHOSTLEVEL_FAILED);
			return false;
		}		
	}
	DEBUG_CATCH("bool CMsgDataArray::UpEquipSuperAddition()")
	return false;

}
//向装备中嵌入宝石
bool CMsgDataArray::EmbedGemToEquip(CUser* pUser)
{
	DEBUG_TRY

	OBJID idEquipment, idGem;

	IF_NOT(m_uLong.GetSize()==2) 
		return false;
	
	idEquipment = m_uLong[0] ;
	idGem = m_uLong[1] ;

	CItem* pEquipItem = pUser->GetItem(idEquipment) ;
	CItem* pGem = pUser->GetItem(idGem) ;

	IF_NOT(pGem && pEquipItem && pGem->IsGemForEmbedEquip())
		return false;
	
	if( pEquipItem->IsEquipment() || pEquipItem->IsWeapon() )//是否为装备或武器
	{
		//得到魔魂等级和战魂等级
		int nSuperAdditionLevel = pEquipItem->GetSuperAddition();
		int nWarGhostLevel = pEquipItem->GetWarGhostLevel();

		if(nSuperAdditionLevel < MAX_LEVEL_SUPERADDITION || nWarGhostLevel < MAX_LEVEL_SUPERADDITION )
		{
			pUser->SendSysMsg(STR_EMBED_FAIL);
			return false;
		}

		DWORD dwGemType = pGem->GetInt(ITEMDATA_TYPE);		
		pUser->EraseItem(idGem,SYNCHRO_TRUE) ; //通知对方将宝石删除 
		pEquipItem->SetInt(ITEMDATA_GEMTYPE,dwGemType);
		CMsgItemInfo msg;			
		if (msg.Create(pEquipItem,ITEMINFO_UPDATE))
			pUser->SendMsg(&msg);		
		pUser->SendSysMsg(STR_EMBED_SUCC);

		return true;
	}
	DEBUG_CATCH("bool CMsgDataArray::EmbedGemToEquip(...)")
	return false;
}

void CMsgDataArray::SetSynDress(CUser* pUser)
{
	IF_NOT(m_uLong.GetSize()==4) 
		return ;
	OBJID idNpc = m_uLong[0] ;
	CNpc* pNpc; 
	if(!pUser->GetMap()->QueryObj(pUser->GetPosX(), pUser->GetPosY(), OBJ_NPC, idNpc, IPP_OF(pNpc)))
		return ;
	if(!pNpc->IsTaskNpc())
		return;
	
	int nLevel1 = m_uLong[1]+1 ;
	int nLevel2 = m_uLong[2]+1 ;
	int nLevel3 = m_uLong[3]+1 ;
	int nLevel4 = m_uLong[4]+1 ;
//	char szData1[255];
//	sprintf(szData1,"%d%d%d%d",nLevel1,nLevel2,nLevel3,nLevel4);
//	int nLevel = atoi(szData1);
	
	int nLevel	= nLevel1*1000 + nLevel2*100 + nLevel3*10 + nLevel4;

	pNpc->SetData(SYNDRESS_SETTING,nLevel,true);//设置帮派服装
	
	pUser->SendSysMsg(STR_SYN_SETDRESS_SUCC);		
	
}