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
uint32 ITEMPROTOTYPES_CACHE_VERSION = 5;
uint32 CREATURETEMPLATES_CACHE_VERSION = 1;
uint32 GOTEMPLATES_CACHE_VERSION = 1;

PlayerNameCache::~PlayerNameCache()
{
}

void PlayerNameCache::Add(uint64 guid, std::string name)
{
    _cache.erase(guid); // drop old data if present
    _cache[guid] = name;
}

bool PlayerNameCache::IsKnown(uint64 guid)
{
    PlayerNameMap::iterator it = _cache.find(guid);
    if(it != _cache.end())
        return true;
    return false;
}

std::string PlayerNameCache::GetName(uint64 guid)
{
    PlayerNameMap::iterator it = _cache.find(guid);
    if(it != _cache.end())
        return it->second;
    return "";
}

uint64 PlayerNameCache::GetGuid(std::string name)
{
    for(PlayerNameMap::iterator it = _cache.begin(); it != _cache.end(); it++)
        if(it->second == name)
            return it->first;
    return 0;
}

bool PlayerNameCache::SaveToFile(void)
{
    if(_cache.empty())
        return true; // no data to save, so we are fine

    logdebug("Saving PlayerNameCache...");
    const char *fn="./cache/playernames.cache";
    std::fstream fh;
    fh.open(fn, std::ios_base::out | std::ios_base::binary);
    if(!fh)
    {
        logerror("PlayerNameCache: Could not write to file '%s'!",fn);
        return false;
    }
    uint32 size = _cache.size();
    if(!size)
        return false;

    fh.write((char*)&size,sizeof(uint32));
    ByteBuffer bb;

    bb << (uint32)_cache.size();
    for(PlayerNameMap::iterator i=_cache.begin(); i!=_cache.end(); i++)
    {
        bb << i->first;
        bb << (uint8)i->second.length();
        bb.append(i->second.c_str(), i->second.length()); // do not append '\0'
    }
    fh.write((char*)bb.contents(), bb.size());
    fh.close();
    logdebug("PlayerNameCache saved successfully.");
    return true;
}

bool PlayerNameCache::ReadFromFile(void)
{
    const char *fn="./cache/playernames.cache";
    log("Loading PlayerNameCache...");
    bool success=true;
    std::fstream fh;
    uint32 size = GetFileSize(fn);
    if(!size)
    {
        logerror("PlayerNameCache: Could not open file '%s'!",fn);
        return false;
    }

    // do NOT use MemoryDataHolder, since the file can change during runtime and may be loaded again
    fh.open(fn, std::ios_base::in | std::ios_base::binary);
    ByteBuffer bb;
    bb.resize(size);
    fh.read((char*)bb.contents(), size);
    fh.close();

    uint8 len;
    uint32 count;
    uint64 guid;
    char namebuf[MAX_PLAYERNAME_LENGTH + 1];

    bb >> count; // entries count

    for(uint32 i = 0; i < count; i++)
    {
        bb >> guid;
        bb >> len;
        if(len > MAX_PLAYERNAME_LENGTH || len < MIN_PLAYERNAME_LENGTH || !guid)
        {
            logerror("PlayerNameCache data seem corrupt [namelength=%d, should be <=%u]",len,MAX_PLAYERNAME_LENGTH);
            log("-> Clearing cache, creating new.");
            _cache.clear();
            success = false;
            break;
        }
        memset(namebuf,0,MAX_PLAYERNAME_LENGTH + 1);
        bb.read((uint8*)namebuf, len);
        Add(guid, namebuf);
    }
    if(success)
        logdebug("PlayerNameCache successfully loaded.");
    return success;
}

uint32 PlayerNameCache::GetSize(void)
{
    return _cache.size();
}

void ItemProtoCache_InsertDataToSession(WorldSession *session)
{
    logdetail("ItemProtoCache: Loading...");
    const char* fn = "./cache/ItemPrototypes.cache";
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
        //DEBUG(logdebug("ItemProtoCache: (%u/%u) - datasize=%u",i,total,datasize));
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
        buf >> proto->Faction;
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
        buf >> proto->StatsCount;
        for(uint32 i = 0; i < proto->StatsCount; i++)
        {
            buf >> proto->ItemStat[i].ItemStatType;
            buf >> proto->ItemStat[i].ItemStatValue;
        }
        buf >> proto->ScalingStatDistribution;
        buf >> proto->ScalingStatValue;
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
        buf >> proto->RandomProperty;
        buf >> proto->RandomSuffix; // added in 2.0.3
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
		buf >> proto->RequiredDisenchantSkill;
		buf >> proto->ArmorDamageModifier;
        buf >> proto->Duration;
        buf >> proto->ItemLimitCategory;
        buf >> proto->HolidayId;

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

    const char* fn = "./cache/ItemPrototypes.cache";
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
        buf << proto->Faction;
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
        buf << proto->StatsCount;
        for(uint32 i = 0; i < proto->StatsCount; i++)
        {
            buf << proto->ItemStat[i].ItemStatType;
            buf << proto->ItemStat[i].ItemStatValue;
        }
        buf << proto->ScalingStatDistribution;
        buf << proto->ScalingStatValue;
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
        buf << proto->RandomProperty;
        buf << proto->RandomSuffix; // added in 2.0.3
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
		buf << proto->RequiredDisenchantSkill;
		buf << proto->ArmorDamageModifier;
        buf << proto->Duration;
        buf << proto->ItemLimitCategory;
        buf << proto->HolidayId;

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
    const char* fn = "./cache/CreatureTemplates.cache";
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
        //buf >> ct->SpellDataId;
        for(uint32 i = 0; i < MAX_KILL_CREDIT; i++)
            buf >> ct->killCredit[i];
        buf >> ct->displayid_A;
        buf >> ct->displayid_H;
        buf >> ct->displayid_AF;
        buf >> ct->displayid_HF;
        buf >> ct->RacialLeader;
        for(uint32 i = 0; i < 4; i++)
            buf >> ct->questItems[i];
        buf >> ct->movementId;

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

    const char* fn = "./cache/CreatureTemplates.cache";
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
        //buf << ct->SpellDataId;
        for(uint32 i = 0; i < MAX_KILL_CREDIT; i++)
            buf << ct->killCredit[i];
        buf << ct->displayid_A;
        buf << ct->displayid_H;
        buf << ct->displayid_AF;
        buf << ct->displayid_HF;
        buf << ct->RacialLeader;
        for(uint32 i = 0; i < 4; i++)
            buf << ct->questItems[i];
        buf << ct->movementId;

        uint32 size = buf.size();
        fh.write((char*)&size,sizeof(uint32));
        fh.write((char*)buf.contents(),buf.size());
        counter++;
    }
    fh.flush();
    fh.close();
    log("CreatureTemplateCache: Saved %u Creature Templates",counter);
}

void GOTemplateCache_InsertDataToSession(WorldSession *session)
{
    logdetail("GOTemplateCache: Loading...");
    const char* fn = "./cache/GOTemplates.cache";
    std::fstream fh;
    fh.open(fn, std::ios_base::in | std::ios_base::binary);
    if(!fh)
    {
        logerror("GOTemplateCache: Could not open file '%s'!",fn);
        return;
    }

    uint32 cacheversion, total, counter = 0;

    try
    {

        fh.read((char*)&cacheversion,4);
        if(cacheversion != GOTEMPLATES_CACHE_VERSION)
        {
            logerror("GOTemplateCache is outdated! Creating new cache.");
            fh.close();
            return;
        }
        fh.read((char*)&total,4);
        logdetail("GOTemplateCache: %u gameobject templates stored",total);

        uint32 datasize;
        ByteBuffer buf;
        for(uint32 i = 0; i < total && !fh.eof(); i++)
        {
            buf.clear();
            fh.read((char*)&datasize,sizeof(uint32));
            buf.resize(datasize);
            fh.read((char*)buf.contents(),datasize);
            GameobjectTemplate *go = new GameobjectTemplate();
            buf >> go->entry;
            buf >> go->type;
            buf >> go->displayId;
            buf >> go->name;
            buf >> go->castBarCaption;
            buf >> go->unk1;
            buf >> go->faction;
            buf >> go->flags;
            buf >> go->size;
            for(uint32 i = 0; i < GAMEOBJECT_DATA_FIELDS; i++)
                buf >> go->raw.data[i];
            buf >> go->size;
            for(uint32 i = 0; i < 4; i++)
                buf >> go->questItems[i];

            if(go->entry)
            {
                session->objmgr.Add(go);
                counter++;
            } else
                delete go;
        }

    }
    catch (ByteBufferException bbe)
    {
        logerror("ByteBuffer exception: attempt to \"%s\" %u bytes at position %u out of total %u bytes. (wpos=%u)",
            bbe.action, bbe.readsize, bbe.rpos, bbe.cursize, bbe.wpos);
    }

    fh.close();
    logdetail("GOTemplateCache: Loaded %u Gameobject Templates",counter);
}

void GOTemplateCache_WriteDataToCache(WorldSession *session)
{
    if (!session->objmgr.GetGOTemplateCount())
        return;

    const char* fn = "./cache/GOTemplates.cache";
    std::fstream fh;
    fh.open(fn, std::ios_base::out | std::ios_base::binary);
    if(!fh)
    {
        logerror("GOTemplateCache: Could not write to file '%s'!",fn);
        return;
    }
    uint32 total = session->objmgr.GetGOTemplateCount();
    fh.write((char*)&GOTEMPLATES_CACHE_VERSION,4);
    fh.write((char*)&total,4);
    uint32 counter=0;
    ByteBuffer buf;
    for(GOTemplateMap::iterator it = session->objmgr.GetGOTemplateStorage()->begin(); it != session->objmgr.GetGOTemplateStorage()->end(); it++)
    {
        buf.clear();
        GameobjectTemplate *go = it->second;
        buf << go->entry;
        buf << go->type;
        buf << go->displayId;
        buf << go->name;
        buf << go->castBarCaption;
        buf << go->unk1;
        buf << go->faction;
        buf << go->flags;
        buf << go->size;
        for(uint32 i = 0; i < GAMEOBJECT_DATA_FIELDS; i++)
            buf << go->raw.data[i];
        buf << go->size;
        for(uint32 i = 0; i < 4; i++)
            buf << go->questItems[i];

        uint32 size = buf.size();
        fh.write((char*)&size,sizeof(uint32));
        fh.write((char*)buf.contents(),buf.size());
        counter++;
    }
    fh.flush();
    fh.close();
    log("GOTemplateCache: Saved %u Gameobject Templates",counter);
}

