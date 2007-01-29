
#include "common.h"
#include "PseuWoW.h"
#include "NameTables.h"
#include "DefScript/DefScript.h"
#include "Player.h"
#include "Opcodes.h"
#include "SharedDefines.h"
#include "WorldSession.h"
#include "Channel.h"

bool DefScriptPackage::SCshdn(CmdSet Set)
{
    ((PseuInstance*)parentMethod)->Stop();
    return true;
}

bool DefScriptPackage::SCpause(CmdSet Set){
    ((PseuInstance*)parentMethod)->Sleep(atoi(Set.defaultarg.c_str()));
    return true;
}

bool DefScriptPackage::SCSendChatMessage(CmdSet Set){
    if(!(((PseuInstance*)parentMethod)->GetWSession() && ((PseuInstance*)parentMethod)->GetWSession()->IsValid()))
    {
        log("Invalid Script call: SCSendChatMessage: WorldSession not valid");
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

bool DefScriptPackage::SCsavecache(CmdSet Set){
   ((PseuInstance*)parentMethod)->SaveAllCache();
    std::stringstream tmp;
    std::string str;
    if(((PseuInstance*)parentMethod)->GetWSession() && ((PseuInstance*)parentMethod)->GetWSession()->IsValid())
    {
        tmp << ((PseuInstance*)parentMethod)->GetWSession()->plrNameCache.GetSize();
        str+="Cache saved. [ "+tmp.str()+ " Playernames ]";
        ((PseuInstance*)parentMethod)->GetWSession()->SendChatMessage(CHAT_MSG_SAY,0,str,"");
    }
    return true;
}

bool DefScriptPackage::SCemote(CmdSet Set){
    if(Set.defaultarg.empty())
        return true;
    if(!(((PseuInstance*)parentMethod)->GetWSession() && ((PseuInstance*)parentMethod)->GetWSession()->IsValid()))
    {
        log("Invalid Script call: SCEmote: WorldSession not valid");
        return false;
    }
    uint32 id=atoi(Set.defaultarg.c_str());
    ((PseuInstance*)parentMethod)->GetWSession()->SendEmote(id);
    return true;
}

bool DefScriptPackage::SCfollow(CmdSet Set){
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

bool DefScriptPackage::SCjoinchannel(CmdSet Set){
    if(Set.defaultarg.empty())
        return true;
    if(!(((PseuInstance*)parentMethod)->GetWSession() && ((PseuInstance*)parentMethod)->GetWSession()->IsValid()))
    {
        log("Invalid Script call: SCjoinchannel: WorldSession not valid");
        return false;
    }
    ((PseuInstance*)parentMethod)->GetWSession()->GetChannels()->Join(Set.defaultarg,Set.arg[0]);
    return true;
}

bool DefScriptPackage::SCleavechannel(CmdSet Set){
    if(Set.defaultarg.empty())
        return true;
    if(!(((PseuInstance*)parentMethod)->GetWSession() && ((PseuInstance*)parentMethod)->GetWSession()->IsValid()))
    {
        log("Invalid Script call: SCleavechannel: WorldSession not valid");
        return false;
    }
    ((PseuInstance*)parentMethod)->GetWSession()->GetChannels()->Leave(Set.defaultarg);
    return true;
}

bool DefScriptPackage::SCloadconf(CmdSet Set){
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

bool DefScriptPackage::SCapplypermissions(CmdSet Set){
    this->My_LoadUserPermissions(variables);
    return true;
}

bool DefScriptPackage::SCapplyconf(CmdSet Set){
    ((PseuInstance*)parentMethod)->GetConf()->ApplyFromVarSet(variables);
    return true;
}

bool DefScriptPackage::SClog(CmdSet Set){
    log(Set.defaultarg.c_str());
    return true;
}

bool DefScriptPackage::SClogdetail(CmdSet Set){
    logdetail(Set.defaultarg.c_str());
    return true;
}

bool DefScriptPackage::SClogdebug(CmdSet Set){
    logdebug(Set.defaultarg.c_str());
    return true;
}

bool DefScriptPackage::SCcastspell(CmdSet Set)
{
	if(Set.defaultarg.empty())
		return true;
	if(!(((PseuInstance*)parentMethod)->GetWSession() && ((PseuInstance*)parentMethod)->GetWSession()->IsValid()))
	{
		log("Invalid Script call: SCcastspell: WorldSession not valid");
		return false;
	}

	uint32 spellId = 0;// = atoi(Set.defaultarg.c_str());
	uint64 spellTarget = 0;// atoi(Set.arg[0]);

	spellId = atoi(Set.defaultarg.c_str());

	if (spellId <= 0)
	{
		log("Invalid Script call: SCcastspell: SpellId not valid");
		return false;
	}

	if (spellTarget <= 0)
	{
		spellTarget = ((PseuInstance*)parentMethod)->GetWSession()->GetGuid();
	}

	((PseuInstance*)parentMethod)->GetWSession()->GetPlayerSettings()->CastSpell(spellId, spellTarget);
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
            DEBUG( log("Player '%s' permission = %u",usr.c_str(),atoi(variables[i].value.c_str())); )
        }
    }
}

bool DefScriptPackage::My_Run(std::string line, std::string username)
{
    DefXChgResult final=ReplaceVars(line,NULL,false);
	CmdSet curSet=SplitLine(final.str);

    uint8 scperm=0,usrperm=0;
    
    for (std::map<std::string,unsigned char>::iterator i = my_usrPermissionMap.begin(); i != my_usrPermissionMap.end(); i++)
    {
        if(i->first == username)
        {
            usrperm = i->second;
        }
    }

    for (std::map<std::string,unsigned char>::iterator i = scriptPermissionMap.begin(); i != scriptPermissionMap.end(); i++)
    {
        if(i->first == curSet.cmd)
        {
            scperm = i->second;
        }
    }

    if(usrperm < scperm)
    {
        CmdSet Set(NULL);
        Set.arg[0] = username;
        Set.arg[1] = toString(usrperm);
        Set.arg[2] = toString(scperm);
        Set.arg[3] = curSet.cmd;
        RunScript("_nopermission",&Set);
        return false;
    }

    Interpret(curSet);
    return true;
}