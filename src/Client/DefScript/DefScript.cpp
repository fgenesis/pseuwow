#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "VarSet.h"
#include "DefScript.h"
#include "DefScriptTools.h"

using namespace DefScriptTools;

#define SN_ONLOAD "?onload?"

enum DefScriptBlockType
{
    BLOCK_IF,
    BLOCK_LOOP
};

// --- SECTION FOR SCRIPT PACKAGES ---
DefScriptPackage::DefScriptPackage()
{
    _eventmgr=new DefScript_DynamicEventMgr(this);
    _InitFunctions();
#   ifdef USING_DEFSCRIPT_EXTENSIONS
    _InitDefScriptInterface();
#   endif
}

DefScriptPackage::~DefScriptPackage()
{
    if(_eventmgr)
        delete _eventmgr;
	Clear();	
}

void DefScriptPackage::SetParentMethod(void *p)
{
    parentMethod = p;
}

void DefScriptPackage::Clear(void)
{
    for(std::map<std::string,DefScript*>::iterator i = Script.begin(); i != Script.end(); i++)
    {
        delete i->second; // delete each script
    }

	Script.empty();
}

void DefScriptPackage::_InitFunctions(void)
{
    AddFunc("out",&DefScriptPackage::func_out);
    AddFunc("set",&DefScriptPackage::func_set);
    AddFunc("default",&DefScriptPackage::func_default);
    AddFunc("unset",&DefScriptPackage::func_unset);
    AddFunc("shdn",&DefScriptPackage::func_shdn);
    AddFunc("loaddef",&DefScriptPackage::func_loaddef);
    AddFunc("reloaddef",&DefScriptPackage::func_reloaddef);
    AddFunc("setscriptpermission",&DefScriptPackage::func_setscriptpermission);
    AddFunc("toint",&DefScriptPackage::func_toint);
    AddFunc("add",&DefScriptPackage::func_add);
    AddFunc("sub",&DefScriptPackage::func_sub);
    AddFunc("mul",&DefScriptPackage::func_mul);
    AddFunc("div",&DefScriptPackage::func_div);
    AddFunc("mod",&DefScriptPackage::func_mod);
    AddFunc("pow",&DefScriptPackage::func_pow);
    AddFunc("bitor",&DefScriptPackage::func_bitor);
    AddFunc("bitand",&DefScriptPackage::func_bitand);
    AddFunc("bitxor",&DefScriptPackage::func_bitxor);
    AddFunc("addevent",&DefScriptPackage::func_addevent);
    AddFunc("removeevent",&DefScriptPackage::func_removeevent);
    AddFunc("abs",&DefScriptPackage::func_abs);
    AddFunc("greater",&DefScriptPackage::func_bigger);
    AddFunc("bigger",&DefScriptPackage::func_bigger);
    AddFunc("equal",&DefScriptPackage::func_equal);
    AddFunc("isset",&DefScriptPackage::func_isset);
    AddFunc("not",&DefScriptPackage::func_not);
    AddFunc("smaller",&DefScriptPackage::func_smaller);
    AddFunc("strlen",&DefScriptPackage::func_strlen);
    AddFunc("tohex",&DefScriptPackage::func_tohex);
    AddFunc("and",&DefScriptPackage::func_and);
    AddFunc("or",&DefScriptPackage::func_or);
    AddFunc("xor",&DefScriptPackage::func_xor);
    AddFunc("substr",&DefScriptPackage::func_substr);
    AddFunc("uppercase",&DefScriptPackage::func_uppercase);
    AddFunc("lowercase",&DefScriptPackage::func_lowercase);
    AddFunc("random",&DefScriptPackage::func_random);
    AddFunc("fileexists",&DefScriptPackage::func_fileexists);

    // list functions
    AddFunc("lpushback",&DefScriptPackage::func_lpushback);
    AddFunc("lappend",&DefScriptPackage::func_lpushback); // alias for lpushback
    AddFunc("lpushfront",&DefScriptPackage::func_lpushfront);
    AddFunc("lpopback",&DefScriptPackage::func_lpopback);
    AddFunc("lpopfront",&DefScriptPackage::func_lpopfront);
    AddFunc("ldelete",&DefScriptPackage::func_ldelete);
    AddFunc("lexists",&DefScriptPackage::func_lexists);
    AddFunc("llen",&DefScriptPackage::func_llen);
    AddFunc("linsert",&DefScriptPackage::func_linsert);
    AddFunc("lsplit",&DefScriptPackage::func_lsplit);
    AddFunc("lcsplit",&DefScriptPackage::func_lcsplit);
    AddFunc("ljoin",&DefScriptPackage::func_ljoin);
    AddFunc("lindex",&DefScriptPackage::func_lindex);
    AddFunc("lclean",&DefScriptPackage::func_lclean);
    AddFunc("lmclean",&DefScriptPackage::func_lmclean);
    AddFunc("lerase",&DefScriptPackage::func_lerase);
    AddFunc("lsort",&DefScriptPackage::func_lsort);
}

void DefScriptPackage::AddFunc(std::string n,DefReturnResult (DefScriptPackage::*f)(CmdSet& Set))
{
    DefScriptFunctionEntry e(n,f);
    AddFunc(e);
}

void DefScriptPackage::AddFunc(DefScriptFunctionEntry e)
{
    if( (!e.name.empty()) && (!HasFunc(e.name)) )
        _functable.push_back(e);
}

bool DefScriptPackage::HasFunc(std::string n)
{
    for(DefScriptFunctionTable::iterator i=_functable.begin();i!=_functable.end();i++)
        if(i->name==n)
            return true;
    return false;
}

void DefScriptPackage::DelFunc(std::string n)
{
    for(DefScriptFunctionTable::iterator i=_functable.begin();i!=_functable.end();i++)
        if(i->name==n)
        {
            _functable.erase(i);
            break;
        }
}

void DefScriptPackage::SetPath(std::string p){
    scPath=p;
}

DefScript *DefScriptPackage::GetScript(std::string scname){
    return ScriptExists(scname) ? Script[scname] : NULL;
}

unsigned int DefScriptPackage::GetScripts(void){
    return Script.size();
}

DefScript_DynamicEventMgr *DefScriptPackage::GetEventMgr(void)
{
    return _eventmgr;
}

bool DefScriptPackage::ScriptExists(std::string name)
{
    for (std::map<std::string,DefScript*>::iterator i = Script.begin();i != Script.end();i++)
        if(i->first == name && i->second != NULL)
            return true;
    return false;
}

void DefScriptPackage::DeleteScript(std::string sn)
{
    if(ScriptExists(sn))
    {
        delete GetScript(sn);
        Script.erase(sn);
    }
}

bool DefScriptPackage::LoadByName(std::string name){
    return LoadScriptFromFile((scPath+name).append(".def"));
}

bool DefScriptPackage::LoadScriptFromFile(std::string fn){
	if(fn.empty())
        return false;

	std::string label, value, line, sn;
    std::fstream f;
    bool load_debug=false,load_notify=false,cantload=false;
    char z;
    unsigned int absline=0;
    DefScript *curScript = NULL;

    f.open(fn.c_str(),std::ios_base::in);
    if(!f.is_open())
        return false;

    std::deque<unsigned int> Blocks;

    // auto-assign the scriptname as the plain filename without extension. can be changed while loading
    unsigned int slashpos = fn.find_last_of("\\/");
    if(slashpos == std::string::npos)
        slashpos = 0;
    unsigned int ppos = fn.find_last_of(".");
    if(ppos == std::string::npos)
    ppos = fn.length();
    sn = stringToLower(fn.substr(slashpos+1,(ppos-slashpos-1)));
    _UpdateOrCreateScriptByName(sn);
    curScript=Script[sn];

    DeleteScript(SN_ONLOAD);

	while(!f.eof())
    {
		line.clear();
        while (true)
        {
            f.get(z);
            if(z=='\n' || f.eof())
                break;
            line+=z;
        }
        absline++;
		if(line.empty())
			continue; // line is empty, proceed with next line

		while( !line.empty() && (line.at(0)==' ' || line.at(0)=='\t') )
			line.erase(0,1);
		if(line.empty())
			continue;
		if(line.at(0)=='/' && line.at(1)=='/') 
			continue; // line is comment, proceed with next line
		if(line.at(0)=='#')
        {
			line.erase(0,1); // remove #
			label=line.substr(0,line.find('=',0));
			value=line.substr(line.find('=',0)+1,line.length());
			if(label=="permission")
            {
                scriptPermissionMap[sn] = atoi(value.c_str());
            }
            else if(label=="script")
            {
                if(!curScript->GetLines()) // delete script if unused
                    DeleteScript(curScript->GetName());
                sn = stringToLower(value);
                _UpdateOrCreateScriptByName(sn);
                _DEFSC_DEBUG(printf("DefScript: now loading '%s'\n",sn.c_str()));
                curScript=Script[sn];
            }
            else if(line=="debug")
            {
                if(curScript)
                    curScript->SetDebug(true);
            }
            else if(line=="onload")
                Script[SN_ONLOAD] = curScript = new DefScript(this);
            else if(line=="endonload" || line=="/onload")
            {
                RunScript(SN_ONLOAD,NULL,sn);
                DeleteScript(SN_ONLOAD);
                curScript=Script[sn];
            }

            //...
            continue; // line was an option, not script content
		}

        // help with loading lines where a space or tab have accidently been put after the cmd
        std::string tline=stringToLower(line);
        if(memcmp(tline.c_str(),"else ",5)==0 || memcmp(tline.c_str(),"else\t",5)==0) tline="else";
        else if(memcmp(tline.c_str(),"endif ",6)==0 || memcmp(tline.c_str(),"endif\t",5)==0) tline="endif";
        else if(memcmp(tline.c_str(),"loop ",5)==0 || memcmp(tline.c_str(),"loop\t",5)==0) tline="loop";
        else if(memcmp(tline.c_str(),"endloop ",8)==0 || memcmp(tline.c_str(),"endloop\t",8)==0) tline="endloop";
        if(tline=="else" || tline=="endif" || tline=="loop" || tline=="endloop")
            line=tline;

        // check for correct block match
        if(memcmp(tline.c_str(),"if ",3)==0) // need to check lowercased line! (IF may be uppercased)
            Blocks.push_back(BLOCK_IF);
        else if(line=="else")
        {
            if(Blocks.empty() || Blocks.back()!=BLOCK_IF)
            {
                cantload=true;
                break;
            }
        }
        else if(line=="endif")
        {
            if(Blocks.empty() || Blocks.back()!=BLOCK_IF)
            {
                cantload=true;
                break;
            }
            Blocks.pop_back();
        }
        else if(line=="loop")
            Blocks.push_back(BLOCK_LOOP);
        else if(line=="endloop")
        {
            if(Blocks.empty() || Blocks.back()!=BLOCK_LOOP)
            {
                cantload=true;
                break;
            }
            Blocks.pop_back();
        }

        // check for correct bracket match
        unsigned int bopen=0;
        bool mismatch=false;
        for(unsigned int bpos=0;bpos<line.length();bpos++)
        {
            if(line[bpos]=='{')
                bopen++;
            if(line[bpos]=='}')
            {
                if(bpos)
                    bopen--;
                else
                {
                    mismatch=true;
                    break;
                }
            }
        }
        if(mismatch || bopen) // no bracket must be left open now
        {
            printf("DefScript [%s:%u]: Bracket mismatch at line '%s'\n",sn.c_str(),absline,line.c_str());
            continue; // continue with next line without adding the current line to script
        }

        curScript->AddLine(line);
	}
	f.close();
    if(cantload || Blocks.size())
    {
        printf("DefScript: Error loading file '%s'. block mismatch?\n",fn.c_str());
        DeleteScript(sn);
        return false;
    }
	
	// ...
    return true;
}
	

// --- SECTION FOR THE INDIVIDUAL SCRIPTS IN A PACKAGE ---

DefScript::DefScript(DefScriptPackage *p)
{
    _parent=p;
	scriptname="{NONAME}";
    debugmode=false;
}

DefScript::~DefScript()
{
    Clear();
}

void DefScript::Clear(void)
{
    Line.clear();
}

void DefScript::SetDebug(bool d)
{
    debugmode=d;
}

bool DefScript::GetDebug(void)
{
    return debugmode;
}

void DefScript::SetName(std::string name)
{
	scriptname=name;
}

std::string DefScript::GetName(void)
{
	return scriptname;
}

unsigned int DefScript::GetLines(void)
{
    return Line.size();
}

std::string DefScript::GetLine(unsigned int id)
{
	return Line[id];
}

bool DefScript::AddLine(std::string l){
	if(l.empty())
		return false;
    Line.push_back(l);
	return true;
}


// --- SECTION FOR COMMAND SETS ---

CmdSet::CmdSet()
{
}

CmdSet::~CmdSet()
{
}

void CmdSet::Clear()
{
    arg.clear();
    cmd="";
    defaultarg="";
    caller="";
    myname="";
}


// --- FUNCTIONS TO PARSE AND EXECUTE A SCRIPT --- PARENT: DefScriptPackage!


// the referred pSet is the parent from which RunScript() has been called
bool DefScriptPackage::BoolRunScript(std::string name, CmdSet *pSet)
{
    DefReturnResult r = RunScript(name,pSet);
    return r.ok;
}

// the referred pSet is the parent from which RunScript() has been called
DefReturnResult DefScriptPackage::RunScript(std::string name, CmdSet *pSet,std::string override_name)
{
    DefReturnResult r;
    if( (!ScriptExists(name)) && (!HasFunc(name)) )
    {
        r.ok=false; // doesnt exist
        r.ret="";
        return r;
    }

    DefScript *sc = GetScript(name);
    if(!sc)
    {
        r.ok=false;
        r.ret="";
        return r;
    }

    if(!override_name.empty())
        name=override_name;

    CmdSet temp;
    if(!pSet)
    {
        pSet = &temp;
    }
    pSet->caller=pSet->myname;
    pSet->myname=name;

    std::deque<Def_Block> Blocks;

    for(unsigned int i=0;i<sc->GetLines();i++)
    {   
        CmdSet mySet;
        std::string line=sc->GetLine(i);
        if(line=="else")
        {
            if(!Blocks.size())
            {
                printf("DEBUG: else-block without any block?! [%s:%u]\n",name.c_str(),i);
                r.ok=false;
                break;
            }
            Def_Block b=Blocks.back();
            if(b.type==BLOCK_IF && b.istrue)
            {
                unsigned int other_ifs=0;
                for(i=b.startline+1;;i++)
                {
                    if(memcmp(sc->GetLine(i).c_str(),"if ",3)==0)
                        other_ifs++;
                    else if(sc->GetLine(i)=="endif")
                    {
                        if(!other_ifs)
                            break;
                        other_ifs--;
                    }
                }
                i--; // next line read will be "endif", decide then what to do
            }
            continue;
        }
        else if(line=="endif")
        {
            if(!Blocks.size())
            {
                printf("DEBUG: endif without any block [%s:%u]\n",name.c_str(),i);
                r.ok=false;
                break;
            }
            if(Blocks.back().type!=BLOCK_IF)
            {
                printf("DEBUG: endif: closed block is not an if block! [%s:%u]\n",name.c_str(),i);
                r.ok=false;
                break;
            }
            Blocks.pop_back();
            continue;
        }
        else if(line=="loop")
        {
            Def_Block b;
            b.startline=i;
            b.type=BLOCK_LOOP;
            b.istrue=true;
            Blocks.push_back(b);
            continue;
        }
        else if(line=="endloop")
        {
            if(!Blocks.size())
            {
                printf("DEBUG: endloop without any block [%s:%u]\n",name.c_str(),i);
                r.ok=false;
                break;
            }
            if(Blocks.back().type!=BLOCK_LOOP)
            {
                printf("DEBUG: endloop: closed block is not a loop block! [%s:%u]\n",name.c_str(),i);
                r.ok=false;
                break;
            }
            i=Blocks.back().startline; // next line executed will be the line after "loop"
            continue;
        }
        //_DEFSC_DEBUG(printf("DefScript before: \"%s\"\n",line.c_str()));
        DefXChgResult final=ReplaceVars(line,pSet,0,true);
        //_DEFSC_DEBUG(printf("DefScript parsed: \"%s\"\n",final.str.c_str()));
	    SplitLine(mySet,final.str);
        if(mySet.cmd=="if")
        {
            Def_Block b;
            b.startline=i;
            b.type=BLOCK_IF;
            b.istrue=isTrue(mySet.defaultarg);
            Blocks.push_back(b);
            if(!b.istrue)
            {
                unsigned int other_ifs=0;
                for(i=b.startline+1;;i++)
                {
                    if(!memcmp(sc->GetLine(i).c_str(),"if ",3))
                        other_ifs++;
                    if(sc->GetLine(i)=="else" || sc->GetLine(i)=="endif")
                    {
                        if(!other_ifs)
                            break;
                        if(sc->GetLine(i)=="endif")
                            other_ifs--;
                    }
                }
                i--; // next line read will be either "else" or "endif", decide then what to do
            }
            continue; // and read line after "else"
        }
        else if(mySet.cmd=="exitloop")
        {
            // skip some ifs if they are present
            while(Blocks.back().type!=BLOCK_LOOP)
                Blocks.pop_back();
            Blocks.pop_back();
            unsigned int other_loops=0;

            while(true)
            {
                if(sc->GetLine(i)=="loop")
                    other_loops++;
                if(sc->GetLine(i)=="endloop")
                {
                    if(!other_loops)
                        break;
                    other_loops--;
                }
                i++; // go until next endloop
            }
            // next line read will be the line after "endloop"
            continue;
        }

        mySet.myname=name;
        mySet.caller=pSet?pSet->myname:"";
        r=Interpret(mySet);
        if(r.mustreturn)
        {
            r.mustreturn=false;
            break;
        }
    }
    return r;
}

DefReturnResult DefScriptPackage::RunSingleLine(std::string line){
    DefXChgResult final=ReplaceVars(line,NULL,0,true);
	CmdSet Set;
    SplitLine(Set,final.str);
    return Interpret(Set);
}

DefReturnResult DefScriptPackage::RunSingleLineFromScript(std::string line, DefScript *pScript){
    CmdSet Set;
    Set.myname=pScript->GetName();
    DefXChgResult final=ReplaceVars(line,&Set,0,true);
    SplitLine(Set,final.str);
    return Interpret(Set);
}

void DefScriptPackage::SplitLine(CmdSet& Set,std::string line){	
	
	unsigned int i=0;
	unsigned int bracketsOpen=0,curParam=0;
    bool cmdDefined=false;
    std::string tempLine;

//	extract cmd+params and txt
    for(i=0;i<line.length();i++){

		if(line.at(i)=='{')
			bracketsOpen++;
        if(line.at(i)=='}')
			bracketsOpen--;
        
        if( line.at(i)==',' && !bracketsOpen)
        {
            if(!cmdDefined){
                Set.cmd=tempLine;
                cmdDefined=true;
            } else {
                Set.arg[curParam]=tempLine;
                curParam++;
            }
            tempLine.clear();
            
        } 
        else if( line.at(i)==' ' && !bracketsOpen)
        {
            if(!cmdDefined){
                Set.cmd=tempLine;
                cmdDefined=true;
            } else {
                Set.arg[curParam]=tempLine;
            }

            Set.defaultarg=line.substr(i,line.length()-i);
            Set.defaultarg.erase(0,1);
            //tempLine.clear();
            break;            
            
        }
        else
        {
            tempLine+=line.at(i);
        }
	}

    if(!cmdDefined)
        Set.cmd=tempLine;
    if(cmdDefined && !Set.cmd.empty() && Set.defaultarg.empty())
        Set.arg[curParam]=tempLine;

    Set.cmd = DefScriptTools::stringToLower(Set.cmd); // lowercase cmd
    RemoveBrackets(Set); // TODO: call this somewhere else as soon as IF and LOOP statements are implemented!
}

std::string DefScriptPackage::RemoveBracketsFromString(std::string t){
    if(t.empty())
        return t;

    if(t.at(0)=='{' && t.at(t.length()-1)=='}')
    {
        t.erase(t.length()-1,1);
        t.erase(0,1);
    }
    unsigned int ob=0,bo=0;
    bool isVar=false;
    for(unsigned int i=0;i<t.length();i++){
            

            if(t[i]=='{')
            {
                if(i>0 && (t[i-1]=='$' || t[i-1]=='?') )
                    isVar=true;
                if(!bo)
                    ob=i;
                bo++;
            }

            if(t[i]=='}')
            {
                bo--;
                if(!bo)
                {
                    if(!isVar)
                    {
                        unsigned int blen=i-ob+1;
                        std::string subStr=t.substr(ob,blen);
                        std::string retStr=RemoveBracketsFromString(subStr);
                        t.erase(ob,blen);
                        t.insert(ob,retStr);
                        i=ob-1;
                        
                    }
                isVar=false;
                }
            }
        }

    return t;
}

void DefScriptPackage::RemoveBrackets(CmdSet& Set)
{
    Set.defaultarg=RemoveBracketsFromString(Set.defaultarg);
    Set.cmd=RemoveBracketsFromString(Set.cmd);
    for(_CmdSetArgMap::iterator i=Set.arg.begin(); i!=Set.arg.end(); i++)
    {
        i->second=RemoveBracketsFromString(i->second);
    }
}


DefXChgResult DefScriptPackage::ReplaceVars(std::string str, CmdSet *pSet, unsigned char VarType, bool run_embedded){

    unsigned int
        openingBracket=0, // defines the position from where the recursive call is started
        closingBracket=0, // the closing bracket
        bracketsOpen=0, // amount of brackets opened
        bLen=0; // the lenth of the string in brackets, e.g. ${abc} == 3

    unsigned char 
        nextVar=DEFSCRIPT_NONE; // '$' or '?'
    bool
        hasChanged=false, // additional helper. once true, xchg.result will be true later also
        hasVar=false; // true if openingBracket (= the first bracket) was preceded by '$' or '?'

    std::string subStr;
    DefXChgResult xchg;

    for(unsigned int i=0;i<str.length();i++)
    {
        if(str[i]=='{')		
        {
            if(!bracketsOpen)
                openingBracket=i; // var starts with $, normal bracket with {
            if(i>0 && str[i-1]=='$')
            {
                hasVar=true;
                if(!bracketsOpen)
                    nextVar=DEFSCRIPT_VAR;
            }
            if(i>0 && str[i-1]=='?')
            {
                hasVar=true;
                if(!bracketsOpen)
                    nextVar=DEFSCRIPT_FUNC;
            }
            bracketsOpen++;
        }
 
        if(str[i]=='}')
        {
            if(bracketsOpen)
		        bracketsOpen--;
            if(!bracketsOpen)
            {
                closingBracket=i;
                if(nextVar==DEFSCRIPT_NONE && VarType!=DEFSCRIPT_NONE && !hasVar) // remove brackets in var names, like ${var{ia}ble}
                {
                    str.erase(closingBracket,1);
                    str.erase(openingBracket,1);
                    i=openingBracket; // jump to the pos where the opening bracket was
                    continue;
                }
                else
                {
                    bLen=closingBracket-openingBracket-1;
                    subStr=str.substr(openingBracket+1,bLen);
                    //printf("SUBSTR: \"%s\"\n",subStr.c_str());
                    xchg=ReplaceVars(subStr,pSet,nextVar,run_embedded);
                    if( nextVar==DEFSCRIPT_NONE && hasVar && xchg.changed )
                    {
                        str.erase(openingBracket+1,subStr.length());
                        str.insert(openingBracket+1,xchg.str);
                        hasVar=false;
                        i-=(subStr.length()+1);
                        hasChanged=true;
                    }
                    else if( nextVar!=DEFSCRIPT_NONE && hasVar && xchg.changed )
                    {
                        str.erase(openingBracket-1,bLen+3); // remove ${...} (+3 because of '${}')
                        i-=(bLen+2); // adjust position
                        str.insert(i,xchg.str);
                        hasVar=false;
                        nextVar=DEFSCRIPT_NONE;
                    }
                }
            }
       } // end if '}'
    } // end for
    if(!bracketsOpen && VarType!=DEFSCRIPT_NONE)
    {
        // fix for empty var: ${}
        if(str.empty())
        {
            xchg.str="";
            xchg.changed=true;
            return xchg;
        }
        if(VarType==DEFSCRIPT_VAR)
        {
            std::string vname=_NormalizeVarName(str, (pSet==NULL) ? "" : pSet->myname);
            if(vname[0]=='@')
            {
                std::stringstream vns;
                std::string subs=vname.substr(1,str.length()-1);
                unsigned int vn=atoi( subs.c_str() );
                vns << vn;
                if(pSet && vns.str()==subs) // resolve arg macros @0 - @99
                    str=pSet->arg[vn];
                else if(pSet && subs=="def")
                    str=pSet->defaultarg;
                else if(pSet && subs=="myname")
                    str=pSet->myname;
                else if(pSet && subs=="cmd")
                    str=pSet->cmd;
                else if(pSet && subs=="caller")
                    str=pSet->caller;
                else if(subs=="n")
                    str="\n";
                else if(variables.Exists(vname))
                    str=variables.Get(vname);
                else
                {
                    // TODO: call custom macro table
                    //...
                    str.clear();
                }
                xchg.changed=true;
            }
            else
            {
                if(variables.Exists(vname))
                {
                    str=variables.Get(vname);
                    xchg.changed=true;
                }
            }
        }
        else if(VarType==DEFSCRIPT_FUNC)
        {
            if(run_embedded)
            {
                DefReturnResult res;
                if(pSet)
                    res=RunSingleLineFromScript(str,GetScript(pSet->myname));
                else
                    res=RunSingleLine(str);
                str=res.ret; // returns empty string on invalid function!!
                xchg.result.ok=res.ok;
                xchg.changed=true;
                //xchg.result.err += res.err;
            }
            else // if not allowed to run scripts via ?{...}
            {
                str=""; // just replace with 0
                xchg.changed=true; // yes we have changed something
                xchg.result.ok=true; // change ok, insert our (empty) return value
            }
        }
    }

    xchg.str = str;
    if(hasChanged)
        xchg.changed=true;
    return xchg;
}

std::string DefScriptPackage::_NormalizeVarName(std::string vn_in, std::string sn){
    if(vn_in.empty())
        return vn_in;
    std::string vn=vn_in;
    bool global=false;
    if(sn.empty())
        global = true;
    while(true)
    {
        if(vn.at(0)=='#')
            global = true;
        if(vn.at(0)=='#' || vn.at(0)==':')
			vn.erase(0,1);
        else
            break;
    }

    if( (!global) && (vn.at(0)!='@') )  
        vn=sn+"::"+vn;

    return vn;
}

DefReturnResult DefScriptPackage::Interpret(CmdSet& Set)
{
    DefReturnResult result;

    // first search if the script is defined in the internal functions
    for(unsigned int i=0;i<_functable.size();i++)
    {
        if(Set.cmd==_functable[i].name)
        {
            result=(this->*(_functable[i].func))(Set);
            return result;
        }
    }

    if(Set.cmd=="return")
    {
        result.mustreturn=true;
        result.ret=Set.defaultarg;
        return result;
    }

    // if nothing has been found its maybe an external script file to run
	result=RunScript(Set.cmd, &Set);
    if((!result.ok) /*&& Script[Set.cmd]->GetDebug()*/)
        std::cout << "Could not execute script command '" << Set.cmd << "'\n";
    

    return result;
}

void DefScriptPackage::_UpdateOrCreateScriptByName(std::string sn)
{
    if(GetScript(sn))
        DeleteScript(sn);
    DefScript *newscript = new DefScript(this);
    newscript->SetName(sn); // necessary that the script knows its own name
    Script[sn] = newscript;
}
