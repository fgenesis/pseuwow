#include <vector>
#include <fstream>
#include "common.h"
#include "PseuWoW.h"
#include "Opcodes.h"
#include "SharedDefines.h"
#include "Player.h"
#include "DefScript/DefScript.h"
#include "WorldSession.h"
#include "CacheHandler.h"
#include "Item.h"

// increase this number whenever you change something that makes old files unusable
uint32 ITEMPROTOTYPES_CACHE_VERSION = 0x00000001;

PlayerNameCache::~PlayerNameCache()
{
	for(std::vector<PlayerNameCacheItem*>::iterator i=_cache.begin(); i!=_cache.end(); i++)
	{
		delete *i;
	}
}

bool PlayerNameCache::AddInfo(uint64 guid, std::string name){
    PlayerNameCacheItem *cacheItem=new PlayerNameCacheItem;
    cacheItem->_name=name;
    cacheItem->_guid=guid;
    return AddInfo(cacheItem);
}

bool PlayerNameCache::IsKnown(uint64 guid)
{
    for(std::vector<PlayerNameCacheItem*>::iterator i=_cache.begin(); i!=_cache.end(); i++)
        if(guid==(*i)->_guid)
            return true;
    return false;
}

bool PlayerNameCache::AddInfo(PlayerNameCacheItem* cacheItem)
{
    for(std::vector<PlayerNameCacheItem*>::iterator i=_cache.begin(); i!=_cache.end(); i++)
        if(cacheItem->_guid==(*i)->_guid)
            return false;
    _cache.push_back(cacheItem);
    return true;
}

std::string PlayerNameCache::GetName(uint64 guid)
{
    for(std::vector<PlayerNameCacheItem*>::iterator i=_cache.begin(); i!=_cache.end(); i++)
        if(guid==(*i)->_guid)
            return (*i)->_name;
    return "";
}

uint64 PlayerNameCache::GetGuid(std::string name)
{
    for(std::vector<PlayerNameCacheItem*>::iterator i=_cache.begin(); i!=_cache.end(); i++)
        if(name==(*i)->_name)
            return (*i)->_guid;
    return 0;
}

bool PlayerNameCache::SaveToFile(void)
{
    log("Saving PlayerNameCache...");
    char *fn="./cache/playernames.cache";
    std::fstream fh;
    fh.open(fn, std::ios_base::out | std::ios_base::binary);
    if(!fh)
    {
        logerror("PlayerNameCache: Could not write to file '%s'!",fn);
        return false;
    }
    uint32 size=_cache.size();
    if(size==0)
        return false;
    uint8 len;
    fh.write((char*)&size,sizeof(uint32));

    for(std::vector<PlayerNameCacheItem*>::iterator i=_cache.begin(); i!=_cache.end(); i++)
    {
        fh.write( (char*)&((*i)->_guid),sizeof(uint64) );
        len=(*i)->_name.length();
        fh.write( (char*)&len,sizeof(uint8) );
        fh.write( (char*)(*i)->_name.c_str(),len );
        DEBUG(log( "PlayerNameCache << " I64FMT " -> %s", (*i)->_guid, (*i)->_name.c_str()););
    }
    fh.close();
    log("PlayerNameCache saved successfully.");
    return true;
}

bool PlayerNameCache::ReadFromFile(void)
{
    char *fn="./cache/playernames.cache";
    log("Loading PlayerNameCache...");
    bool success=true;
    std::fstream fh;
    fh.open(fn, std::ios_base::in | std::ios_base::binary);
    if(!fh)
    {
        logerror("PlayerNameCache: Could not open file '%s'!",fn);
        return false;
    }
    uint32 size;
    fh.read((char*)&size,sizeof(uint32));
    std::string tmp;
    uint8 len;
    char *nameptr=new char[13];
    for(unsigned int i=0;i<size;i++)
    {
        len=255;
        memset(nameptr,0,13);
        PlayerNameCacheItem *cacheItem=new PlayerNameCacheItem;
        fh.read((char*)&(cacheItem->_guid),sizeof(uint64));
        fh.read((char*)&len,sizeof(uint8));
        if(len>12 || len<2){
            logerror("PlayerNameCache data seem corrupt [namelength=%d, should be <=12]",len);
            log("-> Clearing cache, creating new.");
            _cache.clear();
            success=false;
            break;
        }
        fh.read(nameptr,len);
        cacheItem->_name=nameptr;
        AddInfo(cacheItem);
        printf("\rPlayerNameCache [ %u / %u ] items loaded",i+1,size);
        DEBUG(printf( " >> " I64FMT " -> %s\n", cacheItem->_guid, nameptr););
    }
    printf("\n");
    delete nameptr;
    fh.close();
    if(success)
        log("PlayerNameCache successfully loaded.");
    return success;
}

uint32 PlayerNameCache::GetSize(void)
{
    return _cache.size();
}

void ItemProtoCache_InsertDataToSession(WorldSession *session)
{
    char* fn = "./cache/ItemPrototypes.cache";
    std::fstream fh;
    fh.open(fn, std::ios_base::in | std::ios_base::binary);
    if(!fh)
    {
        logerror("ItemProtoCache: Could not open file '%s'!",fn);
        return;
    }

	uint32 cacheversion;
	fh.read((char*)&cacheversion,4);
	if(cacheversion != ITEMPROTOTYPES_CACHE_VERSION)
	{
		logerror("ItemProtoCache is outdated! Creating new cache.");
		fh.close();
		return;
	}

    uint32 datasize,counter=0,unk;
    ByteBuffer buf;
    while(!fh.eof())
    {
        buf.clear();
        fh.read((char*)&datasize,sizeof(uint32));
        buf.resize(datasize);
        fh.read((char*)buf.contents(),datasize);
        ItemProto *proto = new ItemProto;
        buf >> proto->Id;
        buf >> proto->Class;
        buf >> proto->SubClass;
		buf >> unk;
        for(uint8 i=0;i<4;i++)
            buf >> proto->Name[i];
        buf >> proto->DisplayInfoID;
        buf >> proto->Quality;
        buf >> proto->Flags;
        buf >> proto->BuyPrice;
        buf >> proto->SellPrice;
        buf >> proto->InventoryType;
        buf >> proto->AllowableClass;
        buf >> proto->AllowableRace;
        buf >> proto->ItemLevel;
        buf >> proto->RequiredLevel;
        buf >> proto->RequiredSkill;
        buf >> proto->RequiredSkillRank;
        buf >> proto->RequiredSpell;
        buf >> proto->RequiredHonorRank;
        buf >> proto->RequiredCityRank;
        buf >> proto->RequiredReputationFaction;
        buf >> proto->RequiredReputationRank;
        buf >> proto->MaxCount;
        buf >> proto->Stackable;
        buf >> proto->ContainerSlots;
        for(int i = 0; i < 10; i++)
        {
            buf >> proto->ItemStat[i].ItemStatType;
            buf >> proto->ItemStat[i].ItemStatValue;
        }
        for(int i = 0; i < 5; i++)
        {
            buf >> proto->Damage[i].DamageMin;
            buf >> proto->Damage[i].DamageMax;
            buf >> proto->Damage[i].DamageType;
        }
        buf >> proto->Armor;
        buf >> proto->HolyRes;
        buf >> proto->FireRes;
        buf >> proto->NatureRes;
        buf >> proto->FrostRes;
        buf >> proto->ShadowRes;
        buf >> proto->ArcaneRes;
        buf >> proto->Delay;
        buf >> proto->Ammo_type;

        buf >> (float)proto->RangedModRange;
        for(int s = 0; s < 5; s++)
        {
            buf >> proto->Spells[s].SpellId;
            buf >> proto->Spells[s].SpellTrigger;
            buf >> proto->Spells[s].SpellCharges;
            buf >> proto->Spells[s].SpellCooldown;
            buf >> proto->Spells[s].SpellCategory;
            buf >> proto->Spells[s].SpellCategoryCooldown;
        }
        buf >> proto->Bonding;
        buf >> proto->Description;
        buf >> proto->PageText;
        buf >> proto->LanguageID;
        buf >> proto->PageMaterial;
        buf >> proto->StartQuest;
        buf >> proto->LockID;
        buf >> proto->Material;
        buf >> proto->Sheath;
        buf >> proto->Extra;
		buf >> proto->Unk1; // added in 2.0.3
        buf >> proto->Block;
        buf >> proto->ItemSet;
        buf >> proto->MaxDurability;
		buf >> proto->Area;
        buf >> proto->Map;
        buf >> proto->BagFamily;
        buf >> proto->TotemCategory; // Added in 1.12.x client branch
		for(uint32 s = 0; s < 3; s++)
		{
			buf >> proto->Socket[s].Color;
			buf >> proto->Socket[s].Content;
		}
		buf >> proto->socketBonus;
		buf >> proto->GemProperties;
		buf >> proto->ExtendedCost;
		buf >> proto->RequiredDisenchantSkill;
		buf >> proto->ArmorDamageModifier;

        if(proto->Id)
        {
            //DEBUG(logdebug("ItemProtoCache: Loaded %u [%s]",proto->Id, proto->Name[0].c_str()));
            session->objmgr.Add(proto);
            counter++;
        }
    }
    fh.close();
    log("ItemProtoCache: Loaded %u Item Prototypes",counter);
}

void ItemProtoCache_WriteDataToCache(WorldSession *session)
{
	if (session->objmgr.GetItemProtoCount() <= 0)
		return;

    char* fn = "./cache/ItemPrototypes.cache";
    std::fstream fh;
    fh.open(fn, std::ios_base::out | std::ios_base::binary);
    if(!fh)
    {
        logerror("ItemProtoCache: Could not write to file '%s'!",fn);
        return;
    }
	fh.write((char*)&(uint32)ITEMPROTOTYPES_CACHE_VERSION,4);
    uint32 counter=0;
    ByteBuffer buf;
    for(uint32 i=0;i<session->objmgr.GetItemProtoCount();i++)
    {
        buf.clear();
        ItemProto *proto = session->objmgr.GetItemProtoByPos(i);
        buf << proto->Id;
        buf << proto->Class;
        buf << proto->SubClass;
        for(uint8 i=0;i<4;i++)
            buf << proto->Name[i];
        buf << proto->DisplayInfoID;
        buf << proto->Quality;
        buf << proto->Flags;
        buf << proto->BuyPrice;
        buf << proto->SellPrice;
        buf << proto->InventoryType;
        buf << proto->AllowableClass;
        buf << proto->AllowableRace;
        buf << proto->ItemLevel;
        buf << proto->RequiredLevel;
        buf << proto->RequiredSkill;
        buf << proto->RequiredSkillRank;
        buf << proto->RequiredSpell;
        buf << proto->RequiredHonorRank;
        buf << proto->RequiredCityRank;
        buf << proto->RequiredReputationFaction;
        buf << proto->RequiredReputationRank;
        buf << proto->MaxCount;
        buf << proto->Stackable;
        buf << proto->ContainerSlots;
        for(int i = 0; i < 10; i++)
        {
            buf << proto->ItemStat[i].ItemStatType;
            buf << proto->ItemStat[i].ItemStatValue;
        }
        for(int i = 0; i < 5; i++)
        {
            buf << proto->Damage[i].DamageMin;
            buf << proto->Damage[i].DamageMax;
            buf << proto->Damage[i].DamageType;
        }
        buf << proto->Armor;
        buf << proto->HolyRes;
        buf << proto->FireRes;
        buf << proto->NatureRes;
        buf << proto->FrostRes;
        buf << proto->ShadowRes;
        buf << proto->ArcaneRes;
        buf << proto->Delay;
        buf << proto->Ammo_type;

        buf << (float)proto->RangedModRange;
        for(int s = 0; s < 5; s++)
        {
            buf << proto->Spells[s].SpellId;
            buf << proto->Spells[s].SpellTrigger;
            buf << proto->Spells[s].SpellCharges;
            buf << proto->Spells[s].SpellCooldown;
            buf << proto->Spells[s].SpellCategory;
            buf << proto->Spells[s].SpellCategoryCooldown;
        }
        buf << proto->Bonding;
        buf << proto->Description;
        buf << proto->PageText;
        buf << proto->LanguageID;
        buf << proto->PageMaterial;
        buf << proto->StartQuest;
        buf << proto->LockID;
        buf << proto->Material;
        buf << proto->Sheath;
		buf << proto->Extra;
		buf << proto->Unk1; // added in 2.0.3
		buf << proto->Block;
		buf << proto->ItemSet;
		buf << proto->MaxDurability;
		buf << proto->Area;
		buf << proto->Map;
		buf << proto->BagFamily;
		buf << proto->TotemCategory; // Added in 1.12.x client branch
		for(uint32 s = 0; s < 3; s++)
		{
			buf << proto->Socket[s].Color;
			buf << proto->Socket[s].Content;
		}
		buf << proto->socketBonus;
		buf << proto->GemProperties;
		buf << proto->ExtendedCost;
		buf << proto->RequiredDisenchantSkill;
		buf << proto->ArmorDamageModifier;

        //DEBUG(logdebug("ItemProtoCache: Saved %u [%s]",proto->Id, proto->Name[0].c_str()));
        uint32 size = buf.size();
        fh.write((char*)&size,sizeof(uint32));
        fh.write((char*)buf.contents(),buf.size());
        counter++;
    }
    fh.close();
    log("ItemProtoCache: Saved %u Item Prototypes",counter);
}
