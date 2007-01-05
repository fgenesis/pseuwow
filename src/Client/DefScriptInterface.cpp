#ifndef __DEFSCRIPTINTERFACE_H
#define __DEFSCRIPTINTERFACE_H

#include "common.h"
#include "PseuWoW.h"
#include "NameTables.h"
#include "DefScript/DefScript.h"
#include "Player.h"
#include "Opcodes.h"
#include "SharedDefines.h"
#include "WorldSession.h"


bool DefScriptPackage::SCpause(CmdSet Set){
    SDL_Delay(atoi(Set.defaultarg.c_str()));
    return true;
}

bool DefScriptPackage::SCSendChatMessage(CmdSet Set){
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
    tmp << ((PseuInstance*)parentMethod)->GetWSession()->plrNameCache.GetSize();
    str+="Cache saved. [ "+tmp.str()+ " Playernames ]";
    ((PseuInstance*)parentMethod)->GetWSession()->SendChatMessage(CHAT_MSG_SAY,0,str,"");
    return true;


}

bool DefScriptPackage::SCemote(CmdSet Set){
    if(Set.defaultarg.empty())
        return true;
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



#endif
