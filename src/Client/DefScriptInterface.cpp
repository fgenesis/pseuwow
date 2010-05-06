#include <stdarg.h>
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
#include "MemoryDataHolder.h"


void DefScriptPackage::_InitDefScriptInterface(void)
{
    AddFunc("pause",&DefScriptPackage::SCpause);
    AddFunc("emote",&DefScriptPackage::SCemote);
    AddFunc("follow",&DefScriptPackage::SCfollow);
    AddFunc("savecache",&DefScriptPackage::SCsavecache);
    AddFunc("sendchatmessage",&DefScriptPackage::SCSendChatMessage);
    AddFunc("joinchannel",&DefScriptPackage::SCjoinchannel);
    AddFunc("leavechannel",&DefScriptPackage::SCleavechannel);
    AddFunc("listchannel",&DefScriptPackage::SClistchannel);
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
    AddFunc("getscpvalue",&DefScriptPackage::SCGetScpValue);
    AddFunc("getplayerguid",&DefScriptPackage::SCGetPlayerGuid);
    AddFunc("getname",&DefScriptPackage::SCGetName);
    AddFunc("getentry",&DefScriptPackage::SCGetEntry);
    AddFunc("getitemprotovalue",&DefScriptPackage::SCGetItemProtoValue);
    AddFunc("getobjecttype",&DefScriptPackage::SCGetObjectType);
    AddFunc("objectknown",&DefScriptPackage::SCObjectKnown);
    AddFunc("getplayerperm",&DefScriptPackage::SCGetPlayerPerm);
    AddFunc("getscriptperm",&DefScriptPackage::SCGetScriptPerm);
    AddFunc("lgetfiles",&DefScriptPackage::SCGetFileList);
    AddFunc("printscript",&DefScriptPackage::SCPrintScript);
    AddFunc("getobjectvalue",&DefScriptPackage::SCGetObjectValue);
    AddFunc("getrace",&DefScriptPackage::SCGetRace);
    AddFunc("getclass",&DefScriptPackage::SCGetClass);
    AddFunc("sendworldpacket",&DefScriptPackage::SCSendWorldPacket);
    AddFunc("getopcodename",&DefScriptPackage::SCGetOpcodeName);
    AddFunc("getopcodeid",&DefScriptPackage::SCGetOpcodeID);
    AddFunc("bbgetpackedguid",&DefScriptPackage::SCBBGetPackedGuid);
    AddFunc("bbputpackedguid",&DefScriptPackage::SCBBPutPackedGuid);
    AddFunc("gui",&DefScriptPackage::SCGui);
    AddFunc("sendwho",&DefScriptPackage::SCSendWho);
    AddFunc("getobjectdist",&DefScriptPackage::SCGetObjectDistance);
    AddFunc("getobjectpos",&DefScriptPackage::SCGetPos);
    AddFunc("switchopcodehandler",&DefScriptPackage::SCSwitchOpcodeHandler);
    AddFunc("opcodedisabled",&DefScriptPackage::SCOpcodeDisabled);    
    AddFunc("spoofworldpacket",&DefScriptPackage::SCSpoofWorldPacket);
    AddFunc("loaddb",&DefScriptPackage::SCLoadDB);
    AddFunc("adddbpath",&DefScriptPackage::SCAddDBPath);
    AddFunc("preloadfile",&DefScriptPackage::SCPreloadFile);
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
    SCPDatabaseMgr& dbmgr = ((PseuInstance*)parentMethod)->dbmgr;
    uint32 type=atoi(Set.arg[0].c_str());
    uint32 lang=atoi(Set.arg[1].c_str());

    ss << lang;
    if(ss.str()!=Set.arg[1]) // given lang is NOT a number
    {
        SCPDatabase *langdb = dbmgr.GetDB("language");
        uint32 dblang = langdb->GetFieldByStringValue("name",Set.arg[1].c_str());
        logdev("looking up language id for lang '%s', found %i",Set.arg[1].c_str(),dblang);
        // TODO: comment this out to enable using addon language??!
        if(dblang != SCP_INVALID_INT)
            lang = dblang;
    }

    std::string msg=Set.arg[2];
    std::string to=Set.arg[3];
    ((PseuInstance*)parentMethod)->GetWSession()->SendChatMessage(type,lang,msg,to);
    return true;
}

DefReturnResult DefScriptPackage::SCsavecache(CmdSet& Set){
   ((PseuInstance*)parentMethod)->SaveAllCache();
   if(DefScriptTools::isTrue(Set.defaultarg))
   {
        std::stringstream str;
        if(((PseuInstance*)parentMethod)->GetWSession())
        {
            str << "Cache saved. [ ";
            str << ((PseuInstance*)parentMethod)->GetWSession()->plrNameCache.GetSize();
            str << " Playernames, ";
            str << ((PseuInstance*)parentMethod)->GetWSession()->objmgr.GetItemProtoCount();
            str << " Item Prototypes, ";
            str << ((PseuInstance*)parentMethod)->GetWSession()->objmgr.GetCreatureTemplateCount();
            str << " Creature Templates, ";
            str << ((PseuInstance*)parentMethod)->GetWSession()->objmgr.GetGOTemplateCount();
            str << " GameObject Tempates";
            str << " ]";

            ((PseuInstance*)parentMethod)->GetWSession()->SendChatMessage(CHAT_MSG_SAY,0,str.str(),"");
        }
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
    SCPDatabase *emotedb = dbmgr.GetDB("emote");
    if(emotedb)
    {

        id = emotedb->GetFieldByStringValue("name",(char*)DefScriptTools::stringToUpper(Set.defaultarg).c_str()); // emote names are always uppercased
    }
    if(id == SCP_INVALID_INT)
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

DefReturnResult DefScriptPackage::SClistchannel(CmdSet& Set){
    if(Set.defaultarg.empty())
        return false;
    if(!(((PseuInstance*)parentMethod)->GetWSession()))
    {
        logerror("Invalid Script call: SClistchannel: WorldSession not valid");
        DEF_RETURN_ERROR;
    }
    ((PseuInstance*)parentMethod)->GetWSession()->GetChannels()->RequestList(Set.defaultarg);
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

DefReturnResult DefScriptPackage::SCqueryitem(CmdSet& Set)
{
    if(!(((PseuInstance*)parentMethod)->GetWSession()))
    {
        logerror("Invalid Script call: SCqueryitem: WorldSession not valid");
        DEF_RETURN_ERROR;
    }

    uint32 id = atoi(Set.defaultarg.c_str());
    if(!id)
        return false;

    ((PseuInstance*)parentMethod)->GetWSession()->SendQueryItem(id,0);
    return true;
}

DefReturnResult DefScriptPackage::SCtarget(CmdSet& Set)
{
    // TODO: special targets: _self _pet _nearest ...
    WorldSession *ws = ((PseuInstance*)parentMethod)->GetWSession();
    if(!ws)
    {
        logerror("Invalid Script call: SCtarget: WorldSession not valid");
        DEF_RETURN_ERROR;
    }

    if(Set.defaultarg.empty())
    {
        ((PseuInstance*)parentMethod)->GetWSession()->SendSetSelection(0); // no target
        return true;
    }

    std::string what = stringToLower(Set.arg[0]);
    uint64 guid = 0;
    if(what == "guid")
    {
        guid = DefScriptTools::toUint64(Set.defaultarg);
    }
    else if(what.empty() || what == "player")
    {
        // TODO: search through all objects. for now only allow to target player
        guid = ws->plrNameCache.GetGuid(Set.defaultarg);
    }

    Object *obj = ws->objmgr.GetObj(guid);
    if(obj && (obj->IsUnit() || obj->IsCorpse())) // only units and corpses are targetable
    {
        ws->SendSetSelection(guid); // will also set the target for myCharacter
        return DefScriptTools::toString(guid);
    }
    else
    {
        logdetail("Target '%s' not found!",Set.defaultarg.c_str());
        return false;
    }
    return "";
}

/*
DefReturnResult DefScriptPackage::SCloadscp(CmdSet& Set)
{
    SCPDatabaseMgr& dbmgr = ((PseuInstance*)parentMethod)->dbmgr;
    if(Set.defaultarg.empty()) // exit if no filename was given
        return false;
    std::string dbname = stringToLower(Set.arg[0]);
    uint32 sections = 0;
    if(dbname.empty()) // if no db name was supplied, try to find it automatically in the file ("#dbname=...")
    {
        sections = dbmgr.AutoLoadFile((char*)Set.defaultarg.c_str());
        if(!sections)
        {
            logerror("Tried to auto-load SCP file: \"%s\", but no name tag found. skipped.");
        }
    }
    else
    {
        sections = dbmgr.GetDB(dbname).LoadFromFile((char*)Set.defaultarg.c_str());
        if(sections)
        {
            logdetail("Loaded SCP: \"%s\" [%s] (%u sections)",dbname.c_str(),Set.defaultarg.c_str(),sections);
        }
        else
        {
            logerror("Failed to load SCP: \"%s\" [%s]",dbname.c_str(),Set.defaultarg.c_str());
        }
    }
    return DefScriptTools::toString((uint64)sections);
}
*/

// GetScpValue,db,key entry
// db & key will be stored, that multiple calls like GetScpValue entryxyz are possible
DefReturnResult DefScriptPackage::SCGetScpValue(CmdSet& Set)
{
    static std::string dbname;
    static uint32 keyid;
    std::string entry;
    SCPDatabaseMgr& dbmgr = ((PseuInstance*)parentMethod)->dbmgr;

    if(!Set.arg[0].empty())
        dbname=Set.arg[0];
    if(!Set.arg[1].empty())
        keyid=(uint32)DefScriptTools::toUint64(Set.arg[1]);
    if(!Set.defaultarg.empty())
        entry=Set.defaultarg;
    if( (!entry.empty()) && (!dbname.empty()) )
    {
        SCPDatabase *db = dbmgr.GetDB(dbname);
        if(db)
        {
            uint32 ftype = db->GetFieldType((char*)entry.c_str());
            switch(ftype)
            {
                case SCP_TYPE_INT:
                {
                    return DefScriptTools::toString(db->GetInt(keyid,(char*)entry.c_str()));
                }
                case SCP_TYPE_FLOAT: 
                {
                    return DefScriptTools::toString(db->GetFloat(keyid,(char*)entry.c_str()));
                }
                case SCP_TYPE_STRING:
                {
                    return std::string(db->GetString(keyid,(char*)entry.c_str()));
                }
                default: logerror("GetSCPValue: field '%s' does not exist in DB '%s'!",entry.c_str(),dbname.c_str());
            }
        }
        else
        {
            logerror("GetSCPValue: No such DB: '%s'",dbname.c_str());
        }
    }

    return "";
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
        r.ret=DefScriptTools::toString(guid);
    }
    return r;
}

DefReturnResult DefScriptPackage::SCGetName(CmdSet& Set)
{
    WorldSession *ws = ((PseuInstance*)parentMethod)->GetWSession();
    if(!ws)
    {
        logerror("Invalid Script call: SCGetName: WorldSession not valid");
        DEF_RETURN_ERROR;
    }
    uint64 id = DefScriptTools::toUint64(Set.defaultarg);
    std::string source = DefScriptTools::stringToLower(Set.arg[0]);

    // "guid" type can be everything - item, player, unit, gameobject... whatever has this guid.
    if(source.empty() || source == "guid")
    {
        Object *o = ws->objmgr.GetObj(id);
        std::string n;
        if(o)
        {
            n = o->GetName();
            if(!n.empty())
                return n;
        }

        if(!o || n.empty())
        {
            // fallback if that guid is player guid, its maybe stored, so check that
            if(IS_PLAYER_GUID(id))
            {
                if(ws->plrNameCache.IsKnown(id))
                    return ws->plrNameCache.GetName(id);
            }
        }
    }
    else if(source == "player")
    {
        if(ws->plrNameCache.IsKnown(id))
            return ws->plrNameCache.GetName(id);
        else
            return "";

    }
    else if(source == "item")
    {
        ItemProto *iproto = ws->objmgr.GetItemProto((uint32)id);
        return iproto ? iproto->Name : "";
    }
    else if(source == "unit")
    {
        CreatureTemplate *ct = ws->objmgr.GetCreatureTemplate((uint32)id);
        return ct ? ct->name : "";
    }
    else if(source == "gameobject")
    {
        GameobjectTemplate *gt = ws->objmgr.GetGOTemplate((uint32)id);
        return gt ? gt->name : "";
    }
    // TODO: add gameobject, dynamicobject

    return "";
}

DefReturnResult DefScriptPackage::SCGetEntry(CmdSet& Set)
{
    if(!(((PseuInstance*)parentMethod)->GetWSession()))
    {
        logerror("Invalid Script call: SCGetEntry: WorldSession not valid");
        DEF_RETURN_ERROR;
    }
    DefReturnResult r;
    uint64 guid=DefScriptTools::toUint64(Set.defaultarg);
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
    uint64 guid=DefScriptTools::toUint64(Set.defaultarg);
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
    uint64 guid=DefScriptTools::toUint64(Set.defaultarg);
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
    uint32 entry=DefScriptTools::toUint64(Set.arg[0]);
    ItemProto *proto=((PseuInstance*)parentMethod)->GetWSession()->objmgr.GetItemProto(entry);
    if(proto)
    {
        std::string t=stringToLower(Set.defaultarg);
        uint32 tmp=0;
        if(t=="class")     r.ret=toString(proto->Class);
        else if(t=="subclass")  r.ret=toString(proto->SubClass);
        else if(t=="name") r.ret=proto->Name;
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
        else if(t=="randomprop")  r.ret=toString(proto->RandomProperty);
        else if(t=="randomsuffix")  r.ret=toString(proto->RandomSuffix);
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
            if( tmp != ext )
            {
                i = l->erase(i);
                continue;
            }
            i++;
        }
    }
    return toString((uint64)l->size());
}

DefReturnResult DefScriptPackage::SCPrintScript(CmdSet &Set)
{
    DefScript *sc = GetScript(DefScriptTools::stringToLower(Set.defaultarg));
    if(!sc)
        return false;

    logcustom(0,GREEN,"== DefScript \"%s\", %u lines: ==",sc->GetName().c_str(),sc->GetLines());
    for(uint32 i = 0; i < sc->GetLines(); i++)
    {
        logcustom(0,GREEN,sc->GetLine(i).c_str());
    }

    return true;
}

DefReturnResult DefScriptPackage::SCGetObjectValue(CmdSet &Set)
{
    WorldSession *ws = ((PseuInstance*)parentMethod)->GetWSession();
    if(!ws)
    {
        logerror("Invalid Script call: SCGetObjectValue: WorldSession not valid");
        DEF_RETURN_ERROR;
    }

    uint64 guid = DefScriptTools::toUint64(Set.defaultarg);
    Object *o = ws->objmgr.GetObj(guid);
    if(o)
    {
        uint32 v = (uint32)DefScriptTools::toUint64(Set.arg[0]);
        if(v > o->GetValuesCount())
        {
            logerror("SCGetObjectValue ["I64FMTD", type %u]: invalid value index: %u",guid,o->GetTypeId(),v);
            return "";
        }
        else
        {
            if(DefScriptTools::stringToLower(Set.arg[1]) == "f")
                return toString((ldbl)o->GetFloatValue(v));
            else if(DefScriptTools::stringToLower(Set.arg[1]) == "i64")
                return toString(o->GetUInt64Value(v));
            else
                return toString((uint64)o->GetUInt32Value(v));
        }
    }
    return "";
}

DefReturnResult DefScriptPackage::SCGetRace(CmdSet &Set)
{
    WorldSession *ws = ((PseuInstance*)parentMethod)->GetWSession();
    if(!ws)
    {
        logerror("Invalid Script call: SCGetObjectValue: WorldSession not valid");
        DEF_RETURN_ERROR;
    }

    uint64 guid = DefScriptTools::toUint64(Set.defaultarg);
    Object *o = ws->objmgr.GetObj(guid);
    if(o && (o->GetTypeId() == TYPEID_UNIT || o->GetTypeId() == TYPEID_PLAYER))
    {
        return DefScriptTools::toString((uint64)((Unit*)o)->GetRace());
    }
    return "";
}

DefReturnResult DefScriptPackage::SCGetClass(CmdSet &Set)
{
    WorldSession *ws = ((PseuInstance*)parentMethod)->GetWSession();
    if(!ws)
    {
        logerror("Invalid Script call: SCGetObjectValue: WorldSession not valid");
        DEF_RETURN_ERROR;
    }

    uint64 guid = DefScriptTools::toUint64(Set.defaultarg);
    Object *o = ws->objmgr.GetObj(guid);
    if(o && (o->GetTypeId() == TYPEID_UNIT || o->GetTypeId() == TYPEID_PLAYER))
    {
        return DefScriptTools::toString((uint64)((Unit*)o)->GetClass());
    }
    return "";
}

DefReturnResult DefScriptPackage::SCSendWorldPacket(CmdSet &Set)
{
    WorldSession *ws = ((PseuInstance*)parentMethod)->GetWSession();
    if(!ws)
    {
        logerror("Invalid Script call: SCSendWorldPacket: WorldSession not valid");
        DEF_RETURN_ERROR;
    }
    ByteBuffer *bb = bytebuffers.GetNoCreate(_NormalizeVarName(Set.defaultarg,Set.myname));
    if(bb)
    {
        uint16 opc = (uint16)DefScriptTools::toUint64(Set.arg[0]);
        if(!opc)
        {
            opc = (uint16)GetOpcodeID(Set.arg[0].c_str());
            if(opc == uint16(-1))
                return false;
        }
        if(opc) // prevent sending CMSG_NULL_ACTION
        {
            WorldPacket wp(opc, bb->size());
            if(bb->size())
                wp.append(bb->contents(), bb->size());
            ws->SendWorldPacket(wp);
            return true;
        }
    }
    return false;
}

DefReturnResult DefScriptPackage::SCGetOpcodeID(CmdSet &Set)
{
    uint32 id = GetOpcodeID(Set.defaultarg.c_str());
    return id != (uint32)(-1) ? toString(id) : "";
}

DefReturnResult DefScriptPackage::SCGetOpcodeName(CmdSet &Set)
{
    return GetOpcodeName((uint32)DefScriptTools::toUint64(Set.defaultarg));
}

DefReturnResult DefScriptPackage::SCBBGetPackedGuid(CmdSet &Set)
{
    ByteBuffer *bb = bytebuffers.GetNoCreate(_NormalizeVarName(Set.defaultarg,Set.myname));
    if (!bb)
        return false;

    if (bb->size() - bb->rpos() < sizeof(uint8))
        return false;

    uint8 mask;
    *bb >> mask;
    uint64 guid=0;
    for(uint8 i=0;i<8;i++)
    {
        if(mask & (1<<i) )
        {
            if (bb->size() - bb->rpos() < sizeof(uint8))
                return false;

            *bb >> ((uint8*)&guid)[i];
        }
    }

    return toString(guid);
}

DefReturnResult DefScriptPackage::SCBBPutPackedGuid(CmdSet &Set)
{
    ByteBuffer *bb = bytebuffers.Get(_NormalizeVarName(Set.arg[0],Set.myname));

    uint64 guid = DefScriptTools::toUint64(Set.defaultarg);
    if (!guid) // fast check if guid is empty (in this case mask must be 0 with no extra data)
    {
        *bb << uint8(0);
        return false;
    }

    ByteBuffer bbmask;
    uint8 mask = 0;

    for(uint8 i = 0; i < 8; i++)
    {
        if(guid & 0xFF)
        {
            mask |= (1<<i);
            bbmask << ((uint8)(guid & 0xFF));
        }
        guid >>= 8;
    }

    *bb << mask;
    bb->append(bbmask);

    return true;
}

DefReturnResult DefScriptPackage::SCGui(CmdSet &Set)
{
    PseuInstance *ins = (PseuInstance*)parentMethod;
    if(ins->InitGUI())
        logdebug("SCGui: gui created");
    else
    {
        logerror("SCGui: failed");
        return false;
    }
    while(!ins->GetGUI() || !ins->GetGUI()->IsInitialized())
        ins->GetRunnable()->sleep(1);

    ZThread::FastMutex mut;
    mut.acquire();
    if(!ins->GetWSession() && !ins->GetRSession())
    {
        ins->GetGUI()->SetSceneState(SCENESTATE_LOGINSCREEN);
    }
    else if(ins->GetRSession() && !ins->GetWSession())
    {
        ins->GetGUI()->SetSceneState(SCENESTATE_REALMSELECT);
    }
    else if(ins->GetRSession() && ins->GetWSession())
    {
        ins->GetGUI()->SetSceneState(SCENESTATE_CHARSELECT);
    }
    /*else if(ins->GetWSession() && !ins->GetWSession()->InWorld())
    {
        ins->GetGUI()->SetSceneState(SCENESTATE_LOGINSCREEN);
    }*/ // TODO: uncomment after implemented
    else if(ins->GetWSession() && ins->GetWSession()->InWorld())
    {
       ins->GetGUI()->SetSceneState(SCENESTATE_WORLD);
       ins->GetWSession()->objmgr.ReNotifyGUI();
    }
    else
        ins->GetGUI()->SetSceneState(SCENESTATE_GUISTART);
    mut.release();

    return true;
}

DefReturnResult DefScriptPackage::SCSendWho(CmdSet &Set)
{
    WorldSession *ws = ((PseuInstance*)parentMethod)->GetWSession();
    if(!ws)
    {
        logerror("Invalid Script call: SCSendWho: WorldSession not valid");
        DEF_RETURN_ERROR;
    }
    uint32 minlvl = (uint32)DefScriptTools::toUint64(Set.arg[0]);
    uint32 maxlvl = (uint32)DefScriptTools::toUint64(Set.arg[1]);
    uint32 racemask = (uint32)DefScriptTools::toUint64(Set.arg[2]);
    uint32 classmask = (uint32)DefScriptTools::toUint64(Set.arg[3]);
    std::string name = Set.arg[4];
    std::string guildname = Set.arg[5];
    // TODO: implement zonelist

    // convert empty string (default: 0) to default expected by server (-1 = 0xFFFFFFFF = all)
    // and do some values cleanup
    if(!racemask)
        racemask = -1;
    if(!classmask)
        classmask = -1;
    if(!maxlvl)
        maxlvl = 100;
    else if(maxlvl > 255)
        maxlvl = 255;
    if(minlvl > 255)
        minlvl = 255;
    if(minlvl > maxlvl)
        minlvl = maxlvl;

    ws->SendWhoListRequest(minlvl,maxlvl,racemask,classmask,name,guildname);
    return "";
}

DefReturnResult DefScriptPackage::SCGetObjectDistance(CmdSet &Set)
{
    WorldSession *ws = ((PseuInstance*)parentMethod)->GetWSession();
    if(!ws)
    {
        logerror("Invalid Script call: SCGetObjectDistance: WorldSession not valid");
        DEF_RETURN_ERROR;
    }
    uint64 guid1 = DefScriptTools::toUint64(Set.defaultarg);
    uint64 guid2 = DefScriptTools::toUint64(Set.arg[0]);
    std::string s = stringToLower(Set.arg[1]);
    Object *o1, *o2;
    float dist = 0;
    o1 = ws->objmgr.GetObj(guid1);
    o2 = ws->objmgr.GetObj(guid2);
    if(o1 && o2 && o1->IsWorldObject() && o2->IsWorldObject())
    {
        if(s.empty() || s == "2d")
            dist = ((WorldObject*)o1)->GetDistance2d((WorldObject*)o2);
        else if(s == "3d")
            dist = ((WorldObject*)o1)->GetDistance((WorldObject*)o2);

        return toString(ldbl(dist));
    }

    return "";
}

DefReturnResult DefScriptPackage::SCSwitchOpcodeHandler(CmdSet &Set)
{
    WorldSession *ws = ((PseuInstance*)parentMethod)->GetWSession();
    if(!ws)
    {
        logerror("Invalid Script call: SCSwitchOpcodeHandler: WorldSession not valid");
        DEF_RETURN_ERROR;
    }
    uint16 opc = (uint16)DefScriptTools::toUint64(Set.arg[0]);
    bool switchon = DefScriptTools::isTrue(Set.defaultarg);
    if(!opc) // ok we cant turn off MSG_NULL_ACTION with this, but who needs it anyway...
    {
        opc = (uint16)GetOpcodeID(Set.arg[0].c_str());
        if(opc == uint16(-1))
            return false;
    }
    else if(opc >= MAX_OPCODE_ID)
    {
        logerror("Can't enable/disable opcode handling of %u", opc);
        return false;
    }
    if(switchon)
        ws->EnableOpcode(opc);
    else
        ws->DisableOpcode(opc);

    logdebug("Opcode handler for %s (%u) %s",GetOpcodeName(opc), opc, switchon ? "enabled" : "disabled");
    return true;
}

DefReturnResult DefScriptPackage::SCOpcodeDisabled(CmdSet &Set)
{
    WorldSession *ws = ((PseuInstance*)parentMethod)->GetWSession();
    if(!ws)
    {
        logerror("Invalid Script call: SCOpcodeDisabled: WorldSession not valid");
        DEF_RETURN_ERROR;
    }
    uint16 opc = (uint16)DefScriptTools::toUint64(Set.defaultarg);
    if(!opc)
    {
        opc = (uint16)GetOpcodeID(Set.arg[0].c_str());
        if(opc == uint16(-1))
            return false;
    }
    else if(opc >= MAX_OPCODE_ID)
    {
        logerror("SCOpcodeDisabled: Opcode %u out of range", opc);
        return false; // we can NEVER handle out of range opcode, but since its not disabled, return false
    }
    return ws->IsOpcodeDisabled(opc);
}

DefReturnResult DefScriptPackage::SCSpoofWorldPacket(CmdSet &Set)
{
    WorldSession *ws = ((PseuInstance*)parentMethod)->GetWSession();
    if(!ws)
    {
        logerror("Invalid Script call: SCSpoofWorldPacket: WorldSession not valid");
        DEF_RETURN_ERROR;
    }
    ByteBuffer *bb = bytebuffers.GetNoCreate(_NormalizeVarName(Set.defaultarg,Set.myname));
    if(bb)
    {
        uint32 opcode = (uint32)DefScriptTools::toUint64(Set.arg[0]);
        if(opcode) // ok, here again CMSG_NULL_ACTION doesnt work, but who cares
        {
            WorldPacket *wp = new WorldPacket(opcode, bb->size()); // will be deleted by the opcode handler later
            if(bb->size())
                wp->append(bb->contents(), bb->size());
            logdebug("Spoofing WorldPacket with opcode %s (%u), size %u",GetOpcodeName(opcode),opcode,wp->size());
            ws->AddToPktQueue(wp); // handle this packet as if it was sent by the server
            return true;
        }
    }
    return false;
}

DefReturnResult DefScriptPackage::SCLoadDB(CmdSet &Set)
{
    PseuInstance *ins = (PseuInstance*)parentMethod;
    if(ins->dbmgr.GetDB(Set.defaultarg.c_str()))
        return "exists";
    logdetail("Loading database '%s'",Set.defaultarg.c_str());
    uint32 result = ins->dbmgr.SearchAndLoad((char*)Set.defaultarg.c_str(), false);
    return toString(result);
}

DefReturnResult DefScriptPackage::SCAddDBPath(CmdSet &Set)
{
    PseuInstance *ins = (PseuInstance*)parentMethod;
    ins->dbmgr.AddSearchPath((char*)Set.defaultarg.c_str());
    return true;
}

DefReturnResult DefScriptPackage::SCGetPos(CmdSet &Set)
{
    WorldSession *ws = ((PseuInstance*)parentMethod)->GetWSession();
    if(!ws)
    {
        logerror("Invalid Script call: SCGetPos: WorldSession not valid");
        DEF_RETURN_ERROR;
    }

    uint64 guid = DefScriptTools::toUint64(Set.arg[0]);
    Object* obj = ws->objmgr.GetObj(guid ? guid : ws->GetGuid());
    if (!obj || !obj->IsWorldObject())
        return "";

    if (Set.defaultarg == "x")
        return DefScriptTools::toString( ((WorldObject*)obj)->GetX() );
    else if (Set.defaultarg == "y")
        return DefScriptTools::toString( ((WorldObject*)obj)->GetY() );
    else if (Set.defaultarg == "z")
        return DefScriptTools::toString( ((WorldObject*)obj)->GetZ() );
    else if (Set.defaultarg == "o")
        return DefScriptTools::toString( ((WorldObject*)obj)->GetO() );
    return "";
}

DefReturnResult DefScriptPackage::SCPreloadFile(CmdSet& Set)
{
    MemoryDataHolder::BackgroundLoadFile(Set.defaultarg);
    return true;
}

void DefScriptPackage::My_LoadUserPermissions(VarSet &vs)
{
    static const char *prefix = "USERS::";
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
    int16 scperm = -1; // builtin functions that have no explicit req. permission level assigned should be secure from beeing called from ingame
    uint8 usrperm=0; // users/players that have no explicit permission assigned should have minimal permission

    for (std::map<std::string,unsigned char>::iterator i = my_usrPermissionMap.begin(); i != my_usrPermissionMap.end(); i++)
    {
        if(i->first == username)
        {
            usrperm = i->second;
        }
    }

    DefXChgResult final;
    if(usrperm < 255 && line.find("?{")!=std::string::npos)
    {
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


    if(scperm >= 0) // skip "invisible" scripts (without any permission set) completely.
    {
        if(usrperm < scperm)
        {
            CmdSet Set;
            Set.arg[0] = username;
            Set.arg[1] = DefScriptTools::toString(usrperm);
            Set.arg[2] = DefScriptTools::toString(scperm);
            Set.arg[3] = curSet.cmd;
            RunScript("_nopermission",&Set);
            return;
        }
        else
        {
            Interpret(curSet);
        }
    }

}

void DefScriptPackage::my_print(const char *fmt, ...)
{
    if(!fmt)
        return;
    char buf[1000];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf,1000, fmt, ap);
    va_end(ap);
    log(buf);
}

void DefScriptPackage::my_print_error(const char *fmt, ...)
{
    if(!fmt)
        return;
    char buf[1000];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf,1000, fmt, ap);
    va_end(ap);
    logerror(buf);
}

void DefScriptPackage::my_print_debug(const char *fmt, ...)
{
    if(!fmt)
        return;
    char buf[1000];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf,1000, fmt, ap);
    va_end(ap);
    logdebug(buf);
}



