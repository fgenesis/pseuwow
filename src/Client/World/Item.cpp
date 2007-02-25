#include "WorldSession.h"
#include "UpdateFields.h"

#include "Item.h"

void WorldSession::_HandleItemQuerySingleResponseOpcode(WorldPacket& recvPacket)
{
    ItemProto *proto = new ItemProto;
    recvPacket >> proto->Id;
    uint8 field[64];
    memset(field,0,64);
    if(memcmp(recvPacket.contents()+sizeof(uint32),field,64))
    {
        recvPacket >> proto->Class;
        recvPacket >> proto->SubClass;
        for(uint8 i=0;i<4;i++)
            recvPacket >> proto->Name[i];
        recvPacket >> proto->DisplayInfoID;
        recvPacket >> proto->Quality;
        recvPacket >> proto->Flags;
        recvPacket >> proto->BuyPrice;
        recvPacket >> proto->SellPrice;
        recvPacket >> proto->InventoryType;
        recvPacket >> proto->AllowableClass;
        recvPacket >> proto->AllowableRace;
        recvPacket >> proto->ItemLevel;
        recvPacket >> proto->RequiredLevel;
        recvPacket >> proto->RequiredSkill;
        recvPacket >> proto->RequiredSkillRank;
        recvPacket >> proto->RequiredSpell;
        recvPacket >> proto->RequiredHonorRank;
        recvPacket >> proto->RequiredCityRank;
        recvPacket >> proto->RequiredReputationFaction;
        recvPacket >> proto->RequiredReputationRank;
        recvPacket >> proto->MaxCount;
        recvPacket >> proto->Stackable;
        recvPacket >> proto->ContainerSlots;
        for(int i = 0; i < 10; i++)
        {
            recvPacket >> proto->ItemStat[i].ItemStatType;
            recvPacket >> proto->ItemStat[i].ItemStatValue;
        }
        for(int i = 0; i < 5; i++)
        {
            recvPacket >> proto->Damage[i].DamageMin;
            recvPacket >> proto->Damage[i].DamageMax;
            recvPacket >> proto->Damage[i].DamageType;
        }
        recvPacket >> proto->Armor;
        recvPacket >> proto->HolyRes;
        recvPacket >> proto->FireRes;
        recvPacket >> proto->NatureRes;
        recvPacket >> proto->FrostRes;
        recvPacket >> proto->ShadowRes;
        recvPacket >> proto->ArcaneRes;
        recvPacket >> proto->Delay;
        recvPacket >> proto->Ammo_type;

        recvPacket >> (float)proto->RangedModRange;
        for(int s = 0; s < 5; s++)
        {
            recvPacket >> proto->Spells[s].SpellId;
            recvPacket >> proto->Spells[s].SpellTrigger;
            recvPacket >> proto->Spells[s].SpellCharges;
            recvPacket >> proto->Spells[s].SpellCooldown;
            recvPacket >> proto->Spells[s].SpellCategory;
            recvPacket >> proto->Spells[s].SpellCategoryCooldown;
        }
        recvPacket >> proto->Bonding;
        recvPacket >> proto->Description;
        recvPacket >> proto->PageText;
        recvPacket >> proto->LanguageID;
        recvPacket >> proto->PageMaterial;
        recvPacket >> proto->StartQuest;
        recvPacket >> proto->LockID;
        recvPacket >> proto->Material;
        recvPacket >> proto->Sheath;
        recvPacket >> proto->Extra;
        recvPacket >> proto->Block;
        recvPacket >> proto->ItemSet;
        recvPacket >> proto->MaxDurability;
        recvPacket >> proto->Area;
        recvPacket >> proto->Unknown1;
        recvPacket >> proto->Unknown2; // Added in 1.12.x client branch

        logdetail("Got Item Info: Id=%u Name='%s' ReqLevel=%u Armor=%u Desc='%s'",
            proto->Id, proto->Name[0].c_str(), proto->RequiredLevel, proto->Armor, proto->Description.c_str());
        objmgr.Add(proto);
    }
    else
    {
        logdetail("Got info of nonexistent Item %u",proto->Id);
        objmgr.AddNonexistentItem(proto->Id);
        delete proto;
    }
}

Item::Item()
{
    _type |= TYPE_ITEM;
    _typeid = TYPEID_ITEM;

    _valuescount = ITEM_END;
    _slot = 0;
    //_bag = NULL; // not yet implemented
}

void Item::Create(uint64 guid)
{
    Object::Create(guid);
    // what else?
}