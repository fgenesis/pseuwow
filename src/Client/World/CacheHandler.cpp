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
uint32 ITEMPROTOTYPES_CACHE_VERSION = 3;
uint32 CREATURETEMPLATES_CACHE_VERSION = 0;

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
        {
            delete cacheItem;
            return false;
        }
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
    if(!fh.is_open())
    {
        logerror("PlayerNameCache: Could not open file '%s'!",fn);
        return false;
    }
    if(fh.eof())
    {
        logdetail("PlayerNameCache: Can't load empty file '%s'",fn);
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
        if(len > MAX_PLAYERNAME_LENGTH || len < MIN_PLAYERNAME_LENGTH)
        {
            logerror("PlayerNameCache data seem corrupt [namelength=%d, should be <=%u]",len,MAX_PLAYERNAME_LENGTH);
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
    fh.flush();
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
    logdetail("ItemProtoCache: Loading...");
    char* fn = "./cache/ItemPrototypes.cache";
    std::fstream fh;
    fh.open(fn, std::ios_base::in | std::ios_base::binary);
    if(!fh)
    {
        logerror("ItemProtoCache: Could not open file '%s'!",fn);
        return;
    }

	uint32 cacheversion, total;

    try
    {

	fh.read((char*)&cacheversion,4);
	if(cacheversion != ITEMPROTOTYPES_CACHE_VERSION)
	{
		logerror("ItemProtoCache is outdated! Creating new cache.");
		fh.close();
		return;
	}
    fh.read((char*)&total,4);
    logdetail("ItemProtoCache: %u item prototypes stored",total);

    uint32 datasize, counter = 0;
    ByteBuffer buf;
    for(uint32 i = 0; i < total && !fh.eof(); i++)
    {
        buf.clear();
        fh.read((char*)&datasize,sizeof(uint32));
        DEBUG(logdebug("ItemProtoCache: (%u/%u) - datasize=%u",i,total,datasize));
        buf.resize(datasize);
        fh.read((char*)buf.contents(),datasize);
        ItemProto *proto = new ItemProto();
        buf >> proto->Id;
        buf >> proto->Class;
        buf >> proto->SubClass;
        buf >> proto->Name;
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

        buf >> proto->RangedModRange;
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
        buf >> proto->RequiredArenaRank;
		buf >> proto->RequiredDisenchantSkill;
		buf >> proto->ArmorDamageModifier;

        if(proto->Id)
        {
            //DEBUG(logdebug("ItemProtoCache: Loaded %u [%s]",proto->Id, proto->Name[0].c_str()));
            session->objmgr.Add(proto);
            counter++;
        } else
            delete proto;
    }

    }
    catch (ByteBufferException bbe)
    {
        logerror("ByteBuffer exception: attempt to \"%s\" %u bytes at position %u out of total %u bytes. (wpos=%u)",
            bbe.action, bbe.readsize, bbe.rpos, bbe.cursize, bbe.wpos);
    }

    fh.close();
    logdetail("ItemProtoCache: Loaded %u Item Prototypes",total);
}

void ItemProtoCache_WriteDataToCache(WorldSession *session)
{
	if (!session->objmgr.GetItemProtoCount())
		return;

    char* fn = "./cache/ItemPrototypes.cache";
    std::fstream fh;
    fh.open(fn, std::ios_base::out | std::ios_base::binary);
    if(!fh)
    {
        logerror("ItemProtoCache: Could not write to file '%s'!",fn);
        return;
    }

    uint32 total = session->objmgr.GetItemProtoCount();
	fh.write((char*)&ITEMPROTOTYPES_CACHE_VERSION,4);
    fh.write((char*)&total,4);

    uint32 counter=0;
    ByteBuffer buf;
    for(ItemProtoMap::iterator it = session->objmgr.GetItemProtoStorage()->begin(); it != session->objmgr.GetItemProtoStorage()->end(); it++)
    {
        buf.clear();
        ItemProto *proto = it->second;
        buf << proto->Id;
        buf << proto->Class;
        buf << proto->SubClass;
        buf << proto->Name;
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
        buf << proto->RequiredArenaRank;
		buf << proto->RequiredDisenchantSkill;
		buf << proto->ArmorDamageModifier;

        //DEBUG(logdebug("ItemProtoCache: Saved %u [%s]",proto->Id, proto->Name[0].c_str()));
        uint32 size = buf.size();
        fh.write((char*)&size,sizeof(uint32));
        fh.write((char*)buf.contents(),buf.size());
        counter++;
    }
    fh.flush();
    fh.close();
    log("ItemProtoCache: Saved %u Item Prototypes",counter);
}

void CreatureTemplateCache_InsertDataToSession(WorldSession *session)
{
    logdetail("CreatureTemplateCache: Loading...");
    char* fn = "./cache/CreatureTemplates.cache";
    std::fstream fh;
    fh.open(fn, std::ios_base::in | std::ios_base::binary);
    if(!fh)
    {
        logerror("CreatureTemplateCache: Could not open file '%s'!",fn);
        return;
    }

    uint32 cacheversion, total, counter = 0;

    try
    {

    fh.read((char*)&cacheversion,4);
    if(cacheversion != CREATURETEMPLATES_CACHE_VERSION)
    {
        logerror("CreatureTemplateCache is outdated! Creating new cache.");
        fh.close();
        return;
    }
    fh.read((char*)&total,4);
    logdetail("CreatureTemplateCache: %u creature templates stored",total);

    uint32 datasize;
    ByteBuffer buf;
    for(uint32 i = 0; i < total && !fh.eof(); i++)
    {
        buf.clear();
        fh.read((char*)&datasize,sizeof(uint32));
        buf.resize(datasize);
        fh.read((char*)buf.contents(),datasize);
        CreatureTemplate *ct = new CreatureTemplate();
        buf >> ct->entry;
        buf >> ct->name;
        buf >> ct->subname;
        buf >> ct->flag1;
        buf >> ct->type;
        buf >> ct->family;
        buf >> ct->rank;
        buf >> ct->SpellDataId;
        buf >> ct->displayid_A;
        buf >> ct->displayid_H;
        buf >> ct->displayid_AF;
        buf >> ct->displayid_HF;
        buf >> ct->RacialLeader;

        if(ct->entry)
        {
            session->objmgr.Add(ct);
            counter++;
        } else
            delete ct;
    }

    }
    catch (ByteBufferException bbe)
    {
        logerror("ByteBuffer exception: attempt to \"%s\" %u bytes at position %u out of total %u bytes. (wpos=%u)",
            bbe.action, bbe.readsize, bbe.rpos, bbe.cursize, bbe.wpos);
    }

    fh.close();
    logdetail("CreatureTemplateCache: Loaded %u Creature Templates",counter);
}

void CreatureTemplateCache_WriteDataToCache(WorldSession *session)
{
    if (!session->objmgr.GetCreatureTemplateCount())
        return;

    char* fn = "./cache/CreatureTemplates.cache";
    std::fstream fh;
    fh.open(fn, std::ios_base::out | std::ios_base::binary);
    if(!fh)
    {
        logerror("CreatureTemplateCache: Could not write to file '%s'!",fn);
        return;
    }
    uint32 total = session->objmgr.GetCreatureTemplateCount();
    fh.write((char*)&CREATURETEMPLATES_CACHE_VERSION,4);
    fh.write((char*)&total,4);
    uint32 counter=0;
    ByteBuffer buf;
    for(CreatureTemplateMap::iterator it = session->objmgr.GetCreatureTemplateStorage()->begin(); it != session->objmgr.GetCreatureTemplateStorage()->end(); it++)
    {
        buf.clear();
        CreatureTemplate *ct = it->second;
        buf << ct->entry;
        buf << ct->name;
        buf << ct->subname;
        buf << ct->flag1;
        buf << ct->type;
        buf << ct->family;
        buf << ct->rank;
        buf << ct->SpellDataId;
        buf << ct->displayid_A;
        buf << ct->displayid_H;
        buf << ct->displayid_AF;
        buf << ct->displayid_HF;
        buf << ct->RacialLeader;

        uint32 size = buf.size();
        fh.write((char*)&size,sizeof(uint32));
        fh.write((char*)buf.contents(),buf.size());
        counter++;
    }
    fh.flush();
    fh.close();
    log("CreatureTemplateCache: Saved %u Creature Templates",counter);
}
