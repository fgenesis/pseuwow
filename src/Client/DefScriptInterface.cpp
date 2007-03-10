
#include "common.h"
#include "PseuWoW.h"
#include "NameTables.h"
#include "DefScript/DefScript.h"
#include "Player.h"
#include "Opcodes.h"
#include "SharedDefines.h"
#include "WorldSession.h"
#include "Channel.h"
#include "CacheHandler.h"
#include "SCPDatabase.h"

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
    if(!(((PseuInstance*)parentMethod)->GetWSession() && ((PseuInstance*)parentMethod)->GetWSession()->IsValid()))
    {
        logerror("Invalid Script call: SCSendChatMessage: WorldSession not valid");
        return false;
    }
    std::stringstream ss;
    uint32 type=atoi(Set.arg[0].c_str());
    uint32 lang=atoi(Set.arg[1].c_str());
    ss << lang;
    if(ss.str()!=Set.arg[1]) // given lang is NOT a number
    {
        for(uint32 i=0;i<=33;i++)
        {
            if(!stricmp(Set.arg[1].c_str(),LookupName(i,langNames)))
            {
                lang=i;
                break;
            }
        }
    }
    std::string msg=Set.arg[2];
    std::string to=Set.arg[3];
    ((PseuInstance*)parentMethod)->GetWSession()->SendChatMessage(type,lang,msg,to);
    return true;
}

DefReturnResult DefScriptPackage::SCsavecache(CmdSet& Set){
   ((PseuInstance*)parentMethod)->SaveAllCache();
    std::stringstream str;
    if(((PseuInstance*)parentMethod)->GetWSession() && ((PseuInstance*)parentMethod)->GetWSession()->IsValid())
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
        return true;
    if(!(((PseuInstance*)parentMethod)->GetWSession() && ((PseuInstance*)parentMethod)->GetWSession()->IsValid()))
    {
        logerror("Invalid Script call: SCEmote: WorldSession not valid");
        return false;
    }
    uint32 id=atoi(Set.defaultarg.c_str());
    ((PseuInstance*)parentMethod)->GetWSession()->SendEmote(id);
    return true;
}

DefReturnResult DefScriptPackage::SCfollow(CmdSet& Set){
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
    return true;

}

DefReturnResult DefScriptPackage::SCjoinchannel(CmdSet& Set){
    if(Set.defaultarg.empty())
        return true;
    if(!(((PseuInstance*)parentMethod)->GetWSession() && ((PseuInstance*)parentMethod)->GetWSession()->IsValid()))
    {
        logerror("Invalid Script call: SCjoinchannel: WorldSession not valid");
        return false;
    }
    ((PseuInstance*)parentMethod)->GetWSession()->GetChannels()->Join(Set.defaultarg,Set.arg[0]);
    return true;
}

DefReturnResult DefScriptPackage::SCleavechannel(CmdSet& Set){
    if(Set.defaultarg.empty())
        return true;
    if(!(((PseuInstance*)parentMethod)->GetWSession() && ((PseuInstance*)parentMethod)->GetWSession()->IsValid()))
    {
        logerror("Invalid Script call: SCleavechannel: WorldSession not valid");
        return false;
    }
    ((PseuInstance*)parentMethod)->GetWSession()->GetChannels()->Leave(Set.defaultarg);
    return true;
}

DefReturnResult DefScriptPackage::SCloadconf(CmdSet& Set){
    if(Set.defaultarg.empty())
        return true;
    std::string fn;
    if(Set.defaultarg.find('/')==std::string::npos && Set.defaultarg.find('\\')==std::string::npos)
        fn += ((PseuInstance*)parentMethod)->GetConfDir();
    fn += Set.defaultarg;

    if(variables.ReadVarsFromFile(fn))
        log("Loaded conf file [%s]",fn.c_str());
    else
        log("Error loading conf file [%s]",fn.c_str());
    return true;
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
		return true;
	if(!(((PseuInstance*)parentMethod)->GetWSession() && ((PseuInstance*)parentMethod)->GetWSession()->IsValid()))
	{
		logerror("Invalid Script call: SCcastspell: WorldSession not valid");
		return false;
	}

	uint32 spellId = atoi(Set.defaultarg.c_str());

	if (spellId <= 0)
	{
		logerror("Invalid Script call: SCcastspell: SpellId not valid");
		return false;
	}

	((PseuInstance*)parentMethod)->GetWSession()->SendCastSpell(spellId);
	return true;
}

DefReturnResult DefScriptPackage::SCqueryitem(CmdSet& Set){
    uint32 id = atoi(Set.defaultarg.c_str());
    if(!id)
        return true;

    if(!(((PseuInstance*)parentMethod)->GetWSession() && ((PseuInstance*)parentMethod)->GetWSession()->IsValid()))
    {
        logerror("Invalid Script call: SCqueryitem: WorldSession not valid");
        return false;
    }
    ((PseuInstance*)parentMethod)->GetWSession()->SendQueryItem(id,0);
    return true;
}

DefReturnResult DefScriptPackage::SCtarget(CmdSet& Set)
{
    // TODO: special targets: _self _pet _nearest ...

    if(!(((PseuInstance*)parentMethod)->GetWSession() && ((PseuInstance*)parentMethod)->GetWSession()->IsValid()))
    {
        logerror("Invalid Script call: SCtarget: WorldSession not valid");
        return false;
    }

    if(Set.defaultarg.empty())
    {
        ((PseuInstance*)parentMethod)->GetWSession()->SendSetSelection(0); // no target
        return true;
    }

    // TODO: search through all objects. for now only allow to target player
    uint64 guid = (((PseuInstance*)parentMethod)->GetWSession()->plrNameCache.GetGuid(Set.defaultarg));

    if( guid && ((PseuInstance*)parentMethod)->GetWSession()->objmgr.GetObj(guid) ) // object must be near
        ((PseuInstance*)parentMethod)->GetWSession()->SendSetSelection(guid);
    else
        logdetail("Target '%s' not found!",Set.defaultarg.c_str());

    return true;
}

DefReturnResult DefScriptPackage::SCloadscp(CmdSet& Set)
{
    if(Set.arg[0].empty() || Set.defaultarg.empty())
        return true;
    std::string dbname = stringToLower(Set.arg[0]);
    uint32 sections=((PseuInstance*)parentMethod)->GetSCPDatabase(dbname).LoadFromFile((char*)Set.defaultarg.c_str());
    if(sections)
    {
        logdetail("Loaded SCP: \"%s\" [%s] (%u sections)",dbname.c_str(),Set.defaultarg.c_str(),sections);
    }
    else
    {
        logerror("Failed to load SCP: \"%s\" [%s]",dbname.c_str(),Set.defaultarg.c_str());
    }
    return true;
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
    uint8 scperm=0,usrperm=0;

    for (std::map<std::string,unsigned char>::iterator i = my_usrPermissionMap.begin(); i != my_usrPermissionMap.end(); i++)
    {
        if(i->first == username)
        {
            usrperm = i->second;
        }
    }

    // temp fix to prevent users from executing scripts via return values exploit. example:
    // -out ?{say .shutdown}
    // note that the following code can still be executed:
    // -out ${q}?{say .shutdown}
    // where var q = "?"
    if(usrperm < 255 && line.find("?{")!=std::string::npos)
    {
        logerror("WARNING: %s wanted to exec \"%s\"",username.c_str(),line.c_str());
        return;
    }

    DefXChgResult final=ReplaceVars(line,NULL,false);
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
