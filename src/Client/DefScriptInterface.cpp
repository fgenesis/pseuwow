
#include "common.h"
#include "PseuWoW.h"
#include "DefScript/DefScript.h"
#include "DefScript/DefScriptTools.h"
#include "Player.h"
#include "Opcodes.h"
#include "SharedDefines.h"
#include "WorldSession.h"
#include "Channel.h"
#include "CacheHandler.h"
#include "SCPDatabase.h"


void DefScriptPackage::_InitDefScriptInterface(void)
{
    AddFunc("pause",&DefScriptPackage::SCpause);
    AddFunc("emote",&DefScriptPackage::SCemote);
    AddFunc("follow",&DefScriptPackage::SCfollow);
    AddFunc("savecache",&DefScriptPackage::SCsavecache);
    AddFunc("sendchatmessage",&DefScriptPackage::SCSendChatMessage);
    AddFunc("joinchannel",&DefScriptPackage::SCjoinchannel);
    AddFunc("loadconf",&DefScriptPackage::SCloadconf);
    AddFunc("applyconf",&DefScriptPackage::SCapplyconf);
    AddFunc("applypermissions",&DefScriptPackage::SCapplypermissions);
    AddFunc("log",&DefScriptPackage::SClog);
    AddFunc("logdetail",&DefScriptPackage::SClogdetail);
    AddFunc("logerror",&DefScriptPackage::SClogerror);
    AddFunc("logdebug",&DefScriptPackage::SClogdebug);
    AddFunc("castspell",&DefScriptPackage::SCcastspell);
    AddFunc("queryitem",&DefScriptPackage::SCqueryitem);
    AddFunc("target",&DefScriptPackage::SCtarget);
    AddFunc("loadscp",&DefScriptPackage::SCloadscp);
    AddFunc("scpexists",&DefScriptPackage::SCScpExists);
    AddFunc("scpsectionexists",&DefScriptPackage::SCScpSectionExists);
    AddFunc("scpentryexists",&DefScriptPackage::SCScpEntryExists);
    AddFunc("getscpvalue",&DefScriptPackage::SCGetScpValue);
    AddFunc("getplayerguid",&DefScriptPackage::SCGetPlayerGuid);
    AddFunc("getname",&DefScriptPackage::SCGetName);
    AddFunc("getentry",&DefScriptPackage::SCGetName);
    AddFunc("getitemprotovalue",&DefScriptPackage::SCGetName);
    AddFunc("getobjecttype",&DefScriptPackage::SCGetObjectType);
    AddFunc("objectknown",&DefScriptPackage::SCObjectKnown);
    AddFunc("getplayerperm",&DefScriptPackage::SCGetPlayerPerm);
    AddFunc("getscriptperm",&DefScriptPackage::SCGetScriptPerm);
    AddFunc("lgetfiles",&DefScriptPackage::SCGetFileList);
}

DefReturnResult DefScriptPackage::SCshdn(CmdSet& Set)
{
    ((PseuInstance*)parentMethod)->Stop();
    return true;
}

DefReturnResult DefScriptPackage::SCpause(CmdSet& Set){
    ((PseuInstance*)parentMethod)->Sleep(atoi(Set.defaultarg.c_str()));
    return true;
}

DefReturnResult DefScriptPackage::SCSendChatMessage(CmdSet& Set){
    if(!(((PseuInstance*)parentMethod)->GetWSession()))
    {
        logerror("Invalid Script call: SCSendChatMessage: WorldSession not valid");
        DEF_RETURN_ERROR;
    }
    std::stringstream ss;
    uint32 type=atoi(Set.arg[0].c_str());
    uint32 lang=atoi(Set.arg[1].c_str());

    ss << lang;
    if(ss.str()!=Set.arg[1]) // given lang is NOT a number
    {
        uint32 dblang;
        dblang = ((PseuInstance*)parentMethod)->dbmgr.GetDB("language").GetFieldByValue("name",Set.arg[1]);
        logdev("looking up language id for lang '%s', found %i",Set.arg[1].c_str(),dblang);
        if(dblang != -1)
            lang = dblang;
    }

    std::string msg=Set.arg[2];
    std::string to=Set.arg[3];
    ((PseuInstance*)parentMethod)->GetWSession()->SendChatMessage(type,lang,msg,to);
    return true;
}

DefReturnResult DefScriptPackage::SCsavecache(CmdSet& Set){
   ((PseuInstance*)parentMethod)->SaveAllCache();
    std::stringstream str;
    if(((PseuInstance*)parentMethod)->GetWSession())
    {
        str << "Cache saved. [ ";
        str << ((PseuInstance*)parentMethod)->GetWSession()->plrNameCache.GetSize();
        str << " Playernames, ";
        str << ((PseuInstance*)parentMethod)->GetWSession()->objmgr.GetItemProtoCount();
        str << " Item Prototypes";
        str << " ]";
        
        ((PseuInstance*)parentMethod)->GetWSession()->SendChatMessage(CHAT_MSG_SAY,0,str.str(),"");
    }
    return true;
}

DefReturnResult DefScriptPackage::SCemote(CmdSet& Set){
    if(Set.defaultarg.empty())
        return false;
    if(!(((PseuInstance*)parentMethod)->GetWSession()))
    {
        logerror("Invalid Script call: SCEmote: WorldSession not valid");
        DEF_RETURN_ERROR;
    }

    // check if the given name exists in the database, if it does, use its record id; if not, convert emote name into a number.
    // this supports calls like "emote 126" and "emote ready"
    uint32 id = uint32(-1);
    SCPDatabaseMgr& dbmgr = ((PseuInstance*)parentMethod)->dbmgr;
    if(dbmgr.HasDB("emote"))
    {
        SCPDatabase& db = dbmgr.GetDB("emote");
        id = db.GetFieldByValue("name",DefScriptTools::stringToUpper(Set.defaultarg)); // emote names are always uppercased
    }
    if(id == uint32(-1))
    {
        id=atoi(Set.defaultarg.c_str());
        if(!id)
        {
            logerror("SCEmote: Invalid emote!");
            return true;
        }
    }
    ((PseuInstance*)parentMethod)->GetWSession()->SendEmote(id);
    return true;
}


DefReturnResult DefScriptPackage::SCfollow(CmdSet& Set)
{
    DEF_RETURN_ERROR; // prevent execution for now
	/*
    WorldSession *ws=((PseuInstance*)parentMethod)->GetWSession();
    if(Set.defaultarg.empty()){
        ws->SendChatMessage(CHAT_MSG_SAY,0,"Stopping! (Please give me a Playername to follow!)","");
        ws->SetFollowTarget(0);
        return true;
    }
    ws->SetFollowTarget(ws->plrNameCache.GetGuid(Set.defaultarg));
    std::stringstream ss;
    if(ws->GetFollowTarget())
        ss << "Following player '"<<Set.defaultarg<<"'";
    else
        ss << "Can't follow player '"<<Set.defaultarg<<"' (not known)";
    ws->SendChatMessage(CHAT_MSG_SAY,0,ss.str(),"");
	*/
    return true;

}

DefReturnResult DefScriptPackage::SCjoinchannel(CmdSet& Set){
    if(Set.defaultarg.empty())
        return false;
    if(!(((PseuInstance*)parentMethod)->GetWSession()))
    {
        logerror("Invalid Script call: SCjoinchannel: WorldSession not valid");
        DEF_RETURN_ERROR;
    }
    ((PseuInstance*)parentMethod)->GetWSession()->GetChannels()->Join(Set.defaultarg,Set.arg[0]);
    return true;
}

DefReturnResult DefScriptPackage::SCleavechannel(CmdSet& Set){
    if(Set.defaultarg.empty())
        return false;
    if(!(((PseuInstance*)parentMethod)->GetWSession()))
    {
        logerror("Invalid Script call: SCleavechannel: WorldSession not valid");
        DEF_RETURN_ERROR;
    }
    ((PseuInstance*)parentMethod)->GetWSession()->GetChannels()->Leave(Set.defaultarg);
    return true;
}

DefReturnResult DefScriptPackage::SCloadconf(CmdSet& Set){
    if(Set.defaultarg.empty())
        return false;
    std::string fn;
    if(Set.defaultarg.find('/')==std::string::npos && Set.defaultarg.find('\\')==std::string::npos)
        fn += ((PseuInstance*)parentMethod)->GetConfDir();
    fn += Set.defaultarg;

    if(variables.ReadVarsFromFile(fn))
    {
        logdev("Loaded conf file [%s]",fn.c_str());
        return true;
    }

    log("Error loading conf file [%s]",fn.c_str());
    return false;
}

DefReturnResult DefScriptPackage::SCapplypermissions(CmdSet& Set){
    this->My_LoadUserPermissions(variables);
    return true;
}

DefReturnResult DefScriptPackage::SCapplyconf(CmdSet& Set){
    ((PseuInstance*)parentMethod)->GetConf()->ApplyFromVarSet(variables);
    return true;
}

DefReturnResult DefScriptPackage::SClog(CmdSet& Set){
    log(Set.defaultarg.c_str());
    return true;
}

DefReturnResult DefScriptPackage::SClogdetail(CmdSet& Set){
    logdetail(Set.defaultarg.c_str());
    return true;
}

DefReturnResult DefScriptPackage::SClogdebug(CmdSet& Set){
    logdebug(Set.defaultarg.c_str());
    return true;
}

DefReturnResult DefScriptPackage::SClogerror(CmdSet& Set){
    logerror(Set.defaultarg.c_str());
    return true;
}

DefReturnResult DefScriptPackage::SCcastspell(CmdSet& Set)
{
	if(Set.defaultarg.empty())
		return false;
	if(!(((PseuInstance*)parentMethod)->GetWSession()))
	{
		logerror("Invalid Script call: SCcastspell: WorldSession not valid");
		DEF_RETURN_ERROR;
	}

	uint32 spellId = atoi(Set.defaultarg.c_str());

	if (spellId <= 0)
	{
		return false;
	}

	((PseuInstance*)parentMethod)->GetWSession()->SendCastSpell(spellId);
	return true;
}

DefReturnResult DefScriptPackage::SCqueryitem(CmdSet& Set){
    uint32 id = atoi(Set.defaultarg.c_str());
    if(!id)
        return false;

    if(!(((PseuInstance*)parentMethod)->GetWSession()))
    {
        logerror("Invalid Script call: SCqueryitem: WorldSession not valid");
        DEF_RETURN_ERROR;
    }
    ((PseuInstance*)parentMethod)->GetWSession()->SendQueryItem(id,0);
    return true;
}

DefReturnResult DefScriptPackage::SCtarget(CmdSet& Set)
{
    // TODO: special targets: _self _pet _nearest ...
    DefReturnResult r;

    if(!(((PseuInstance*)parentMethod)->GetWSession()))
    {
        logerror("Invalid Script call: SCtarget: WorldSession not valid");
        DEF_RETURN_ERROR;
    }

    if(Set.defaultarg.empty())
    {
        ((PseuInstance*)parentMethod)->GetWSession()->SendSetSelection(0); // no target
        return true;
    }

    // TODO: search through all objects. for now only allow to target player
    uint64 guid = (((PseuInstance*)parentMethod)->GetWSession()->plrNameCache.GetGuid(Set.defaultarg));

    if( guid && ((PseuInstance*)parentMethod)->GetWSession()->objmgr.GetObj(guid) ) // object must be near
    {
        ((PseuInstance*)parentMethod)->GetWSession()->SendSetSelection(guid); // will also set the target for myCharacter
        r.ret=toString(guid);
    }
    else
    {
        logdetail("Target '%s' not found!",Set.defaultarg.c_str());
        return false;
    }

    return r;
}

DefReturnResult DefScriptPackage::SCloadscp(CmdSet& Set)
{
    DefReturnResult r;
    if(Set.arg[0].empty() || Set.defaultarg.empty())
        return false;
    std::string dbname = stringToLower(Set.arg[0]);
    // TODO: remove db if loading was not successful
    uint32 sections=((PseuInstance*)parentMethod)->dbmgr.GetDB(dbname).LoadFromFile((char*)Set.defaultarg.c_str());
    if(sections)
    {
        logdetail("Loaded SCP: \"%s\" [%s] (%u sections)",dbname.c_str(),Set.defaultarg.c_str(),sections);
    }
    else
    {
        logerror("Failed to load SCP: \"%s\" [%s]",dbname.c_str(),Set.defaultarg.c_str());
    }
    r.ret=toString((uint64)sections);
    return r;
}

DefReturnResult DefScriptPackage::SCScpExists(CmdSet& Set)
{
    return (!Set.defaultarg.empty()) && ((PseuInstance*)parentMethod)->dbmgr.HasDB(Set.defaultarg);
}

DefReturnResult DefScriptPackage::SCScpSectionExists(CmdSet& Set)
{
    static std::string dbname;
    if(!Set.arg[0].empty())
        dbname=Set.arg[0];
    return (!Set.defaultarg.empty()) && (!dbname.empty())
        && ((PseuInstance*)parentMethod)->dbmgr.HasDB(dbname)
        && ((PseuInstance*)parentMethod)->dbmgr.GetDB(dbname).HasField((uint32)DefScriptTools::toNumber(Set.defaultarg));
}

DefReturnResult DefScriptPackage::SCScpEntryExists(CmdSet& Set)
{
    static std::string dbname;
    static uint32 keyid;
    if(!Set.arg[0].empty())
        dbname=Set.arg[0];
    if(!Set.arg[1].empty())
        keyid=(uint32)DefScriptTools::toNumber(Set.arg[1]);
    return (!Set.defaultarg.empty()) && (!dbname.empty())
        && ((PseuInstance*)parentMethod)->dbmgr.HasDB(dbname)
        && ((PseuInstance*)parentMethod)->dbmgr.GetDB(dbname).HasField(keyid)
        && ((PseuInstance*)parentMethod)->dbmgr.GetDB(dbname).GetField(keyid).HasEntry(Set.defaultarg);
}


// GetScpValue,db,key entry
// db & key will be stored, that multiple calls like GetScpValue entryxyz are possible
DefReturnResult DefScriptPackage::SCGetScpValue(CmdSet& Set)
{
    static std::string dbname;
    static uint32 keyid;
    std::string entry;
    DefReturnResult r;

    if(!Set.arg[0].empty())
        dbname=Set.arg[0];
    if(!Set.arg[1].empty())
        keyid=(uint32)DefScriptTools::toNumber(Set.arg[1]);
    if(!Set.defaultarg.empty())
        entry=Set.defaultarg;
    if( (!entry.empty()) && (!dbname.empty())
        && ((PseuInstance*)parentMethod)->dbmgr.HasDB(dbname)
        && ((PseuInstance*)parentMethod)->dbmgr.GetDB(dbname).HasField(keyid)
        && ((PseuInstance*)parentMethod)->dbmgr.GetDB(dbname).GetField(keyid).HasEntry(entry))
    {
        r.ret = ((PseuInstance*)parentMethod)->dbmgr.GetDB(dbname).GetField(keyid).GetString(entry);
    }
    else
    {
        r.ret = "";
    }
    return r;
}

DefReturnResult DefScriptPackage::SCGetPlayerGuid(CmdSet& Set)
{
    if(!(((PseuInstance*)parentMethod)->GetWSession()))
    {
        logerror("Invalid Script call: SCGetPlayerGuid: WorldSession not valid");
        DEF_RETURN_ERROR;
    }
    DefReturnResult r;
    if(Set.defaultarg.empty())
    {
        r.ret="0";
    }
    else
    {
        uint64 guid=((PseuInstance*)parentMethod)->GetWSession()->plrNameCache.GetGuid(Set.defaultarg);
        r.ret=toString(guid);
    }
    return r;
}

DefReturnResult DefScriptPackage::SCGetName(CmdSet& Set)
{
    if(!(((PseuInstance*)parentMethod)->GetWSession()))
    {
        logerror("Invalid Script call: SCGetName: WorldSession not valid");
        DEF_RETURN_ERROR;
    }
    DefReturnResult r;
    uint64 guid=DefScriptTools::toNumber(Set.defaultarg);
    r.ret="Unknown Entity";
    Object *o=((PseuInstance*)parentMethod)->GetWSession()->objmgr.GetObj(guid);
    if(o)
    {
        if(o->GetTypeId()==TYPEID_ITEM || o->GetTypeId()==TYPEID_CONTAINER)
        {
            ItemProto *proto=((PseuInstance*)parentMethod)->GetWSession()->objmgr.GetItemProto(o->GetEntry());
            r.ret=proto->Name[0]; // and whats with Name[1] - Name[3]?
        }
        else if(o->GetTypeId()==TYPEID_UNIT || o->GetTypeId()==TYPEID_PLAYER || o->GetTypeId()==TYPEID_GAMEOBJECT || o->GetTypeId()==TYPEID_CORPSE)
        {
            r.ret=((WorldObject*)o)->GetName();
        }
        // TODO: add support for gameobjects etc.
    }
    else
    {
        logerror("SCGetName: Object "I64FMT" not known",guid);
    }
    return r;
}

DefReturnResult DefScriptPackage::SCGetEntry(CmdSet& Set)
{
    if(!(((PseuInstance*)parentMethod)->GetWSession()))
    {
        logerror("Invalid Script call: SCGetEntry: WorldSession not valid");
        DEF_RETURN_ERROR;
    }
    DefReturnResult r;
    uint64 guid=DefScriptTools::toNumber(Set.defaultarg);
    r.ret="0";
    Object *o=((PseuInstance*)parentMethod)->GetWSession()->objmgr.GetObj(guid);
    if(o)
    {
        r.ret=DefScriptTools::toString((uint64)o->GetEntry());
    }
    else
    {
        logerror("SCGetEntry: Object "I64FMT" not known",guid);
    }
    return r;
}

DefReturnResult DefScriptPackage::SCGetObjectType(CmdSet& Set)
{
    if(!(((PseuInstance*)parentMethod)->GetWSession()))
    {
        logerror("Invalid Script call: SCGetObjectType: WorldSession not valid");
        DEF_RETURN_ERROR;
    }
    DefReturnResult r;
    uint64 guid=DefScriptTools::toNumber(Set.defaultarg);
    r.ret="0";
    Object *o=((PseuInstance*)parentMethod)->GetWSession()->objmgr.GetObj(guid);
    if(o)
    {
        r.ret=DefScriptTools::toString((uint64)o->GetTypeId());
    }
    else
    {
        logerror("SCGetObjectType: Object "I64FMT" not known",guid);
    }
    return r;
}

DefReturnResult DefScriptPackage::SCObjectKnown(CmdSet& Set)
{
    if(!(((PseuInstance*)parentMethod)->GetWSession()))
    {
        logerror("Invalid Script call: SCObjectIsKnown: WorldSession not valid");
        DEF_RETURN_ERROR;
    }
    uint64 guid=DefScriptTools::toNumber(Set.defaultarg);
    Object *o=((PseuInstance*)parentMethod)->GetWSession()->objmgr.GetObj(guid);
    return o!=NULL;
}

DefReturnResult DefScriptPackage::SCGetPlayerPerm(CmdSet& Set)
{
    DefReturnResult r;
    uint8 perm=0;
    for (std::map<std::string,unsigned char>::iterator i = my_usrPermissionMap.begin(); i != my_usrPermissionMap.end(); i++)
        if(i->first == Set.defaultarg)
            perm = i->second;
    r.ret = toString(perm);
    return r;
}

DefReturnResult DefScriptPackage::SCGetScriptPerm(CmdSet& Set)
{
    DefReturnResult r;
    uint8 perm=0;
    for (std::map<std::string,unsigned char>::iterator i = scriptPermissionMap.begin(); i != scriptPermissionMap.end(); i++)
        if(i->first == Set.defaultarg)
            perm = i->second;
    r.ret = toString(perm);
    return r;
}

DefReturnResult DefScriptPackage::SCGetItemProtoValue(CmdSet& Set)
{
    if(!(((PseuInstance*)parentMethod)->GetWSession()))
    {
        logerror("Invalid Script call: SCGetItemProtoValue: WorldSession not valid");
        DEF_RETURN_ERROR;
    }
    DefReturnResult r;
    uint32 entry=DefScriptTools::toNumber(Set.arg[0]);
    ItemProto *proto=((PseuInstance*)parentMethod)->GetWSession()->objmgr.GetItemProto(entry);
    if(proto)
    {
        std::string t=stringToLower(Set.defaultarg);
        uint32 tmp=0;
        if(t=="class")     r.ret=toString(proto->Class);
        else if(t=="subclass")  r.ret=toString(proto->SubClass);
        else if(t=="name1" || t=="name") proto->Name[0];
        else if(t=="name2") proto->Name[1];
        else if(t=="name3") proto->Name[2];
        else if(t=="name4") proto->Name[3];
        else if(t=="model" || t=="displayid")  r.ret=toString(proto->DisplayInfoID);
        else if(t=="quality")  r.ret=toString(proto->Quality);
        else if(t=="flags")  r.ret=toString(proto->Flags);
        else if(t=="buycount")  r.ret=toString(proto->BuyCount);
        else if(t=="buyprice")  r.ret=toString(proto->BuyPrice);
        else if(t=="sellprice")  r.ret=toString(proto->SellPrice);
        else if(t=="inventorytype")  r.ret=toString(proto->InventoryType);
        else if(t=="allowableclass")  r.ret=toString(proto->AllowableClass);
        else if(t=="allowablerace")  r.ret=toString(proto->AllowableRace);
        else if(t=="itemlevel")  r.ret=toString(proto->ItemLevel);
        else if(t=="reqlevel")  r.ret=toString(proto->RequiredLevel);
        else if(t=="reqskill")  r.ret=toString(proto->RequiredSkill);
        else if(t=="reqskillrank")  r.ret=toString(proto->RequiredSkillRank);
        else if(t=="reqspell")  r.ret=toString(proto->RequiredSpell);
        else if(t=="reqhonorrank")  r.ret=toString(proto->RequiredHonorRank);
        else if(t=="reqcityrank")  r.ret=toString(proto->RequiredCityRank);
        else if(t=="reqrepfaction")  r.ret=toString(proto->RequiredReputationFaction);
        else if(t=="reqreprank")  r.ret=toString(proto->RequiredReputationRank);
        else if(t=="maxcount")  r.ret=toString(proto->MaxCount);
        else if(t=="stackable")  r.ret=toString(proto->Stackable);
        else if(t=="slots")  r.ret=toString(proto->ContainerSlots);
        else if(t=="str")
        {
            for(uint32 i=0;i<10;i++)
                if(proto->ItemStat[i].ItemStatType==ITEM_STAT_STRENGTH)
                    tmp+=proto->ItemStat[i].ItemStatValue;
            r.ret = toString((uint64)tmp);
        }
        else if(t=="agi")
        {
            for(uint32 i=0;i<10;i++)
                if(proto->ItemStat[i].ItemStatType==ITEM_STAT_AGILITY)
                    tmp+=proto->ItemStat[i].ItemStatValue;
            r.ret = toString((uint64)tmp);
        }
        else if(t=="sta")
        {
            for(uint32 i=0;i<10;i++)
                if(proto->ItemStat[i].ItemStatType==ITEM_STAT_STAMINA)
                    tmp+=proto->ItemStat[i].ItemStatValue;
            r.ret = toString((uint64)tmp);
        }
        else if(t=="int")
        {
            for(uint32 i=0;i<10;i++)
                if(proto->ItemStat[i].ItemStatType==ITEM_STAT_INTELLECT)
                    tmp+=proto->ItemStat[i].ItemStatValue;
            r.ret = toString((uint64)tmp);
        }
        else if(t=="spi")
        {
            for(uint32 i=0;i<10;i++)
                if(proto->ItemStat[i].ItemStatType==ITEM_STAT_SPIRIT)
                    tmp+=proto->ItemStat[i].ItemStatValue;
            r.ret = toString((uint64)tmp);
        }
        else if(t=="hp" || t=="health")
        {
            for(uint32 i=0;i<10;i++)
                if(proto->ItemStat[i].ItemStatType==ITEM_STAT_HEALTH)
                    tmp+=proto->ItemStat[i].ItemStatValue;
            r.ret = toString((uint64)tmp);
        }
        else if(t=="mindmg")
        {
            for(uint32 i=0;i<5;i++)
                if(proto->Damage[i].DamageType==NORMAL_DAMAGE)
                    tmp+=proto->Damage[i].DamageMin;
            r.ret = toString((uint64)tmp);
        }
        else if(t=="maxdmg")
        {
            for(uint32 i=0;i<5;i++)
                if(proto->Damage[i].DamageType==NORMAL_DAMAGE)
                    tmp+=proto->Damage[i].DamageMax;
            r.ret = toString((uint64)tmp);
        }
        else if(t=="mindmg_holy")
        {
            for(uint32 i=0;i<5;i++)
                if(proto->Damage[i].DamageType==HOLY_DAMAGE)
                    tmp+=proto->Damage[i].DamageMin;
            r.ret = toString((uint64)tmp);
        }
        else if(t=="maxdmg_holy")
        {
            for(uint32 i=0;i<5;i++)
                if(proto->Damage[i].DamageType==HOLY_DAMAGE)
                    tmp+=proto->Damage[i].DamageMax;
            r.ret = toString((uint64)tmp);
        }
        else if(t=="mindmg_fire")
        {
            for(uint32 i=0;i<5;i++)
                if(proto->Damage[i].DamageType==FIRE_DAMAGE)
                    tmp+=proto->Damage[i].DamageMin;
            r.ret = toString((uint64)tmp);
        }
        else if(t=="maxdmg_fire")
        {
            for(uint32 i=0;i<5;i++)
                if(proto->Damage[i].DamageType==FIRE_DAMAGE)
                    tmp+=proto->Damage[i].DamageMax;
            r.ret = toString((uint64)tmp);
        }
        else if(t=="mindmg_nature")
        {
            for(uint32 i=0;i<5;i++)
                if(proto->Damage[i].DamageType==NATURE_DAMAGE)
                    tmp+=proto->Damage[i].DamageMin;
            r.ret = toString((uint64)tmp);
        }
        else if(t=="maxdmg_nature")
        {
            for(uint32 i=0;i<5;i++)
                if(proto->Damage[i].DamageType==NATURE_DAMAGE)
                    tmp+=proto->Damage[i].DamageMax;
            r.ret = toString((uint64)tmp);
        }
        else if(t=="mindmg_frost")
        {
            for(uint32 i=0;i<5;i++)
                if(proto->Damage[i].DamageType==FROST_DAMAGE)
                    tmp+=proto->Damage[i].DamageMin;
            r.ret = toString((uint64)tmp);
        }
        else if(t=="maxdmg_frost")
        {
            for(uint32 i=0;i<5;i++)
                if(proto->Damage[i].DamageType==FROST_DAMAGE)
                    tmp+=proto->Damage[i].DamageMax;
            r.ret = toString((uint64)tmp);
        }
        else if(t=="mindmg_shadow")
        {
            for(uint32 i=0;i<5;i++)
                if(proto->Damage[i].DamageType==SHADOW_DAMAGE)
                    tmp+=proto->Damage[i].DamageMin;
            r.ret = toString((uint64)tmp);
        }
        else if(t=="maxdmg_shadow")
        {
            for(uint32 i=0;i<5;i++)
                if(proto->Damage[i].DamageType==SHADOW_DAMAGE)
                    tmp+=proto->Damage[i].DamageMax;
            r.ret = toString((uint64)tmp);
        }
        else if(t=="mindmg_arcane")
        {
            for(uint32 i=0;i<5;i++)
                if(proto->Damage[i].DamageType==ARCANE_DAMAGE)
                    tmp+=proto->Damage[i].DamageMin;
            r.ret = toString((uint64)tmp);
        }
        else if(t=="maxdmg_arcane")
        {
            for(uint32 i=0;i<5;i++)
                if(proto->Damage[i].DamageType==ARCANE_DAMAGE)
                    tmp+=proto->Damage[i].DamageMax;
            r.ret = toString((uint64)tmp);
        }
        else if(t=="armor")  r.ret=toString(proto->Armor);
        else if(t=="holyres")  r.ret=toString(proto->HolyRes);
        else if(t=="fireres")  r.ret=toString(proto->FireRes);
        else if(t=="natureres")  r.ret=toString(proto->NatureRes);
        else if(t=="frostres")  r.ret=toString(proto->FrostRes);
        else if(t=="shadowres")  r.ret=toString(proto->ShadowRes);
        else if(t=="arcaneres")  r.ret=toString(proto->ArcaneRes);
        else if(t=="delay")  r.ret=toString(proto->Delay);
        else if(t=="ammotype")  r.ret=toString(proto->Ammo_type);
        else if(t=="range")  r.ret=toString(proto->RangedModRange);
        else if(t=="bonding")  r.ret=toString(proto->Bonding);
        else if(t=="desc")  r.ret=proto->Description;
        else if(t=="pagetext")  r.ret=toString(proto->PageText);
        else if(t=="languageid")  r.ret=toString(proto->LanguageID);
        else if(t=="pagematerial")  r.ret=toString(proto->PageMaterial);
        else if(t=="startquest")  r.ret=toString(proto->StartQuest);
        else if(t=="lockid")  r.ret=toString(proto->LockID);
        else if(t=="material")  r.ret=toString(proto->Material);
        else if(t=="sheath")  r.ret=toString(proto->Sheath);
        else if(t=="extra")  r.ret=toString(proto->Extra);
        else if(t=="block")  r.ret=toString(proto->Block);
        else if(t=="itemset")  r.ret=toString(proto->ItemSet);
        else if(t=="maxdur")  r.ret=toString(proto->MaxDurability);
        else if(t=="area")  r.ret=toString(proto->Area);
        else if(t=="bagfamily")  r.ret=toString(proto->BagFamily);
        // TODO: add item spells
        else
        {
            logerror("SCGetItemProtoValue: Unknown property \"%s\" for Item Prototype %u",t.c_str(),entry);
        }
    }
    else
    {
        logerror("SCGetItemProtoValue: Item Prototype %u not known",entry);
    }
    return r;
}

DefReturnResult DefScriptPackage::SCGetFileList(CmdSet& Set)
{
    DefList *l = lists.Get(_NormalizeVarName(Set.arg[0],Set.myname));
    l->clear();
    *l = (DefList)GetFileList(Set.defaultarg);
    if(Set.arg[1].length())
    {
        std::string ext = ".";
        ext += Set.arg[1];
        ext = stringToLower(ext);
        for(DefList::iterator i = l->begin(); i != l->end(); )
        {
            std::string tmp = stringToLower(i->c_str() + (i->length() - ext.length()));
            if( stringToLower(i->c_str() + (i->length() - ext.length())) != ext )
            {
                l->erase(i);
                continue;
            }
            i++;
        }
    }
    return toString((uint64)l->size());
}



void DefScriptPackage::My_LoadUserPermissions(VarSet &vs)
{
    static char *prefix = "USERS::";
    std::string sub,usr;
    for(uint32 i=0;i<variables.Size();i++)
    {
        sub = variables[i].name.substr(0,strlen(prefix));
        if(sub == prefix)
        {   
            usr = variables[i].name.substr(strlen(prefix), variables[i].name.length() - strlen(prefix));
            my_usrPermissionMap[usr] = atoi(variables[i].value.c_str());
            DEBUG( logdebug("Player '%s' permission = %u",usr.c_str(),atoi(variables[i].value.c_str())); )
        }
    }
}

void DefScriptPackage::My_Run(std::string line, std::string username)
{
    uint8 scperm=255; // builtin functions that have no explicit req. permission level assigned should be secure from beeing called
    uint8 usrperm=0; // users/players that have no explicit permission assigned should have minimal permission

    for (std::map<std::string,unsigned char>::iterator i = my_usrPermissionMap.begin(); i != my_usrPermissionMap.end(); i++)
    {
        if(i->first == username)
        {
            usrperm = i->second;
        }
    }

    DefXChgResult final;
    if(usrperm < 255)
    {
        if(line.find("?{")!=std::string::npos)
            logerror("WARNING: %s wanted to exec \"%s\"",username.c_str(),line.c_str());
        final=ReplaceVars(line,NULL,0,false); // prevent execution of embedded scripts (= using return values) that could trigger dangerous stuff.
    }
    else
        final=ReplaceVars(line,NULL,0,true); // exec as usual

    
    CmdSet curSet;
    SplitLine(curSet,final.str);

    for (std::map<std::string,unsigned char>::iterator i = scriptPermissionMap.begin(); i != scriptPermissionMap.end(); i++)
    {
        if(i->first == curSet.cmd)
        {
            scperm = i->second;
        }
    }

    if(usrperm < scperm)
    {
        CmdSet Set;
        Set.arg[0] = username;
        Set.arg[1] = toString(usrperm);
        Set.arg[2] = toString(scperm);
        Set.arg[3] = curSet.cmd;
        RunScript("_nopermission",&Set);
        return;
    }

    Interpret(curSet);
}
