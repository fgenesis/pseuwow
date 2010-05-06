#include "WorldSession.h"
#include "UpdateFields.h"

#include "Item.h"
#include "Bag.h"

void WorldSession::_HandleItemQuerySingleResponseOpcode(WorldPacket& recvPacket)
{
    uint32 ItemID;
    uint32 unk;
    uint8 unk8;
    std::string s;

    recvPacket >> ItemID;
    if(!(ItemID & 0x80000000)) // invalid item flag?
    {
        ItemProto *proto = new ItemProto();
        proto->Id = ItemID;
        recvPacket >> proto->Class;
        recvPacket >> proto->SubClass;
		recvPacket >> unk; // dont need that value?
        recvPacket >> proto->Name;
        recvPacket >> unk8;
        recvPacket >> unk8;
        recvPacket >> unk8; // strip name2-4
        recvPacket >> proto->DisplayInfoID;
        recvPacket >> proto->Quality;
        recvPacket >> proto->Flags;
        recvPacket >> proto->Faction;                                  // 3.2 faction?
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
        recvPacket >> proto->StatsCount;
        for(uint32 i = 0; i < proto->StatsCount; i++)
        {
            recvPacket >> proto->ItemStat[i].ItemStatType;
            recvPacket >> proto->ItemStat[i].ItemStatValue;
        }
        recvPacket >> proto->ScalingStatDistribution;
        recvPacket >> proto->ScalingStatValue;
        for(int i = 0; i < MAX_ITEM_PROTO_DAMAGES; i++)
        {
            recvPacket >> proto->Damage[i].DamageMin;
            recvPacket >> proto->Damage[i].DamageMax;
            recvPacket >> proto->Damage[i].DamageType;
        }

        // resistances (7)
        recvPacket >> proto->Armor;
        recvPacket >> proto->HolyRes;
        recvPacket >> proto->FireRes;
        recvPacket >> proto->NatureRes;
        recvPacket >> proto->FrostRes;
        recvPacket >> proto->ShadowRes;
        recvPacket >> proto->ArcaneRes;

        recvPacket >> proto->Delay;
        recvPacket >> proto->Ammo_type;
        recvPacket >> proto->RangedModRange;

        for(int s = 0; s < MAX_ITEM_PROTO_SPELLS; s++)
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
        recvPacket >> proto->RandomProperty;
        recvPacket >> proto->RandomSuffix;
        recvPacket >> proto->Block;
        recvPacket >> proto->ItemSet;
        recvPacket >> proto->MaxDurability;
        recvPacket >> proto->Area;
		recvPacket >> proto->Map;
		recvPacket >> proto->BagFamily;
		recvPacket >> proto->TotemCategory; // Added in 1.12.x client branch
		for(uint32 s = 0; s < MAX_ITEM_PROTO_SOCKETS; s++)
		{
			recvPacket >> proto->Socket[s].Color;
			recvPacket >> proto->Socket[s].Content;
		}
		recvPacket >> proto->socketBonus;
		recvPacket >> proto->GemProperties;
		recvPacket >> proto->RequiredDisenchantSkill;
		recvPacket >> proto->ArmorDamageModifier; 
        recvPacket >> proto->Duration;
        recvPacket >> proto->ItemLimitCategory;
        recvPacket >> proto->HolidayId;

        logdetail("Got Item Info: Id=%u Name='%s' ReqLevel=%u Armor=%u Desc='%s'",
            proto->Id, proto->Name.c_str(), proto->RequiredLevel, proto->Armor, proto->Description.c_str());
        objmgr.Add(proto);
        objmgr.AssignNameToObj(proto->Id, TYPEID_ITEM, proto->Name);
        objmgr.AssignNameToObj(proto->Id, TYPEID_CONTAINER, proto->Name);
    }
    else
    {
        ItemID &= 0x7FFFFFFF; // remove nonexisting item flag
        logdetail("Item %u doesn't exist!",ItemID);
        objmgr.AddNonexistentItem(ItemID);
    }
}

Item::Item()
{
    _depleted = false;
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
