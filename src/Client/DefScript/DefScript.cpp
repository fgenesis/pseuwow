#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "VarSet.h"
#include "DefScript.h"

// --- SECTION FOR SCRIPT PACKAGES ---
DefScriptPackage::DefScriptPackage()
{
    functionTable=_GetFunctionTable();
    _eventmgr=new DefScript_DynamicEventMgr(this);
}

DefScriptPackage::~DefScriptPackage()
{
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

DefScriptFunctionTable *DefScriptPackage::_GetFunctionTable(void) const
{
    static DefScriptFunctionTable table[] = {
        // basic functions:
        {"out",&DefScriptPackage::func_out},
        {"set",&DefScriptPackage::func_set},
        {"default",&DefScriptPackage::func_default},
        {"unset",&DefScriptPackage::func_unset},
        {"shdn",&DefScriptPackage::func_shdn},
        {"loaddef",&DefScriptPackage::func_loaddef},
        {"reloaddef",&DefScriptPackage::func_reloaddef},
        {"setscriptpermission",&DefScriptPackage::func_setscriptpermission},

        // mathematical functions:
        {"toint",&DefScriptPackage::func_toint},
        {"add",&DefScriptPackage::func_add},
        {"sub",&DefScriptPackage::func_sub},
        {"mul",&DefScriptPackage::func_mul},
        {"div",&DefScriptPackage::func_div},
        {"mod",&DefScriptPackage::func_mod},
        {"pow",&DefScriptPackage::func_pow},
        {"bitor",&DefScriptPackage::func_bitor},
        {"bitand",&DefScriptPackage::func_bitand},
        {"bitxor",&DefScriptPackage::func_bitxor},
        {"addevent",&DefScriptPackage::func_addevent},
        {"removeevent",&DefScriptPackage::func_removeevent},

        // user functions:
        {"pause",&DefScriptPackage::SCpause},
        {"emote",&DefScriptPackage::SCemote},
        //{"follow",&DefScriptPackage::SCfollow},
        {"savecache",&DefScriptPackage::SCsavecache},
        {"sendchatmessage",&DefScriptPackage::SCSendChatMessage},
        {"joinchannel",&DefScriptPackage::SCjoinchannel},
        {"leavechannel",&DefScriptPackage::SCleavechannel},
        {"loadconf",&DefScriptPackage::SCloadconf},
        {"applyconf",&DefScriptPackage::SCapplyconf},
        {"applypermissions",&DefScriptPackage::SCapplypermissions},
        {"log",&DefScriptPackage::SClog},
        {"logdetail",&DefScriptPackage::SClogdetail},
        {"logerror",&DefScriptPackage::SClogerror},
        {"logdebug",&DefScriptPackage::SClogdebug},
		{"castspell", &DefScriptPackage::SCcastspell},
        {"queryitem", &DefScriptPackage::SCqueryitem},

        // table termination
        {NULL,NULL}

    };
    return table;
}

void DefScriptPackage::SetFunctionTable(DefScriptFunctionTable *tab)
{
    functionTable=tab;
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
    for(unsigned int i=0;functionTable[i].func!=NULL;i++)
        if(name == functionTable[i].name)
            return true;
    return false;
}

bool DefScriptPackage::LoadByName(std::string name){
    return LoadScriptFromFile((scPath+name).append(".def"),name);
}

bool DefScriptPackage::LoadScriptFromFile(std::string fn, std::string sn){
	if(fn.empty() || sn.empty()) return false;

	std::string label, value, line;
    std::fstream f;
    bool load_debug=false,load_notify=false, exec=false;
    char z;

    f.open(fn.c_str(),std::ios_base::in);
    if(!f.is_open())
        return false;

    if(GetScript(sn))
        delete GetScript(sn);
    DefScript *newscript = new DefScript(this);
    Script[sn] = newscript;
    Script[sn]->SetName(sn); // necessary that the script knows its own name
	while(!f.eof()){
		line.clear();
        while (true) {
            f.get(z);
            if(z=='\n' || f.eof())
                break;
            line+=z;
        }
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
            if(line=="load_debug")
                load_debug=true;
            if(line=="load_notify")
                load_notify=true;
            if(line=="debug")
                Script[sn]->SetDebug(true);
            if(line=="onload")
                exec=true;
            if(line=="endonload" || line=="/onload")
                exec=false;
            //...
            continue; // line was an option, not script content
		}
        if(load_debug)
            std::cout<<"~LOAD: "<<line<<"\n";
        if(!exec)
		    Script[sn]->AddLine(line);
        else
        {
            this->RunSingleLineFromScript(line,Script[sn]);
        }
		
		
	}
	f.close();
    if(load_notify)
        std::cout << "+> Script '" << sn << "' [" << fn << "] successfully loaded.\n";
	
	// ...
    return true;
}
	

// --- SECTION FOR THE INDIVIDUAL SCRIPTS IN A PACKAGE ---

DefScript::DefScript(DefScriptPackage *p)
{
    _parent=p;
	scriptname="{NONAME}";
    debugmode=false;
//    printf("--> DefScript inited!\n");
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
    Line.insert(Line.end(),l);
	return true;
}


// --- SECTION FOR COMMAND SETS ---

CmdSet::CmdSet(DefScript *p){
	Clear();
    owner = p;
}

CmdSet::~CmdSet(){
	Clear();
}

void CmdSet::Clear()
{
    for(unsigned int i=0;i<MAXARGS;i++){
		arg[i]="";
	}
	cmd="";
    defaultarg="";
    caller="";
}


// --- FUNCTIONS TO PARSE AND EXECUTE A SCRIPT --- PARENT: DefScriptPackage!


// the referred pSet is the parent from which RunScript() has been called
bool DefScriptPackage::RunScript(std::string name, CmdSet *pSet)
{
    if(!ScriptExists(name))
        if(!LoadByName(name))
            return false; // doesnt exist & cant be loaded

    DefScript *sc = GetScript(name);
    if(!sc)
        return false;

    CmdSet temp(sc);
    if(!pSet)
    {
        pSet = &temp;
    }
    pSet->caller=pSet->myname;
    pSet->myname=name;

    for(unsigned int i=0;i<sc->GetLines();i++)
    {   
        CmdSet curSet(NULL);
        DefXChgResult final=ReplaceVars(sc->GetLine(i),pSet,false);
        //printf("SC: \"%s\"\n",final.str.c_str());
	    curSet=SplitLine(final.str);
        curSet.owner=sc; // must set the owner after SplitLine()
        curSet.myname=name;
        curSet.caller=pSet?pSet->myname:"";
        Interpret(curSet);
    }
    return true;
}

bool DefScriptPackage::RunSingleLine(std::string line){
    DefXChgResult final=ReplaceVars(line,NULL,false);
	CmdSet curSet=SplitLine(final.str);
    return Interpret(curSet);
}

bool DefScriptPackage::RunSingleLineFromScript(std::string line, DefScript *pScript){
    CmdSet Set(pScript);
    Set.myname=pScript->GetName(); // temp fix, this needs to be cleaned up later
    DefXChgResult final=ReplaceVars(line,&Set,false);
    CmdSet curSet=SplitLine(final.str);
    curSet.myname=pScript->GetName(); // temp fix, this needs to be cleaned up later
    return Interpret(curSet);
}

CmdSet DefScriptPackage::SplitLine(std::string line){	
	
	unsigned int i=0;
	unsigned int bracketsOpen=0,curParam=0;
    bool cmdDefined=false;
    std::string tempLine;
	CmdSet outSet(NULL);

//	extract cmd+params and txt
    for(i=0;i<line.length();i++){

		if(line.at(i)=='{')
			bracketsOpen++;
        if(line.at(i)=='}')
			bracketsOpen--;
        
        if( line.at(i)==',' && !bracketsOpen)
        {
            if(!cmdDefined){
                outSet.cmd=tempLine;
                cmdDefined=true;
            } else {
                outSet.arg[curParam]=tempLine;
                curParam++;
            }
            tempLine.clear();
            
        } 
        else if( line.at(i)==' ' && !bracketsOpen)
        {
            if(!cmdDefined){
                outSet.cmd=tempLine;
                cmdDefined=true;
            } else {
                outSet.arg[curParam]=tempLine;
            }

            outSet.defaultarg=line.substr(i,line.length()-i);
            outSet.defaultarg.erase(0,1);
            //tempLine.clear();
            break;            
            
        }
        else
        {
            tempLine+=line.at(i);
        }
	}

    if(!cmdDefined)
        outSet.cmd=tempLine;
    if(cmdDefined && !outSet.cmd.empty() && outSet.defaultarg.empty())
        outSet.arg[curParam]=tempLine;

    outSet.cmd.assign(strlwr((char*)outSet.cmd.c_str())); // TODO: use toLower() instead!
    outSet.owner=GetScript(outSet.cmd);
    return RemoveBrackets(outSet);	
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
                if(i>0 && t[i-1]=='$')
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

CmdSet DefScriptPackage::RemoveBrackets(CmdSet oldSet){
    CmdSet Set=oldSet;
    std::string t;
    for(unsigned int a=0;a<MAXARGS+2;a++){
        if(a==0)
            t=Set.defaultarg;
        else if(a==1)
            t=Set.cmd;
        else
            t=Set.arg[a-2];

        if(t.empty()) // skip empty args
            continue;        
        
        t=RemoveBracketsFromString(t);
        
        if(a==0)
            Set.defaultarg=t;
        else if(a==1)
            Set.cmd=t;
        else
            Set.arg[a-2]=t;
    }

    return Set;
}


DefXChgResult DefScriptPackage::ReplaceVars(std::string str, CmdSet *pSet, bool isVar){

    unsigned int
        openingBracket=0, // defines the position from where the recursive call is started
        closingBracket=0, // the closing bracket
        bracketsOpen=0, // amount of brackets opened
        bLen=0; // the lenth of the string in brackets, e.g. ${abc} == 3

    bool 
        nextIsVar=false, // true if the last bracket was preceded by '$'
        hasVar=false, // true if openingBracket (= the first bracket) was preceded by '$'
        hasChanged=false; // additional helper. once true, xchg.result will be true later also

    //  isVar (arg)  : defines if the current string is a variable (can only be true in recursive calls!)


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
                    nextIsVar=true;
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
                if(!nextIsVar && isVar && !hasVar) // remove brackets in var names, like ${var{ia}ble}
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
                    xchg=ReplaceVars(subStr,pSet,nextIsVar);
                    if( (!nextIsVar) && hasVar && xchg.changed )
                    {
                        str.erase(openingBracket+1,subStr.length());
                        str.insert(openingBracket+1,xchg.str);
                        hasVar=false;
                        nextIsVar=false;
                        i-=(subStr.length()+1);
                        hasChanged=true;
                    }
                    else if( nextIsVar && hasVar && xchg.changed )
                    {
                        str.erase(openingBracket-1,bLen+3); // remove ${...} (+3 because of '${}')
                        i-=(bLen+2); // adjust position
                        str.insert(i,xchg.str);
                        //i--;
                        hasVar=false;
                        nextIsVar=false;
                    }
                }
            }
       } // end if '}'
    } // end for
    if(!bracketsOpen && isVar)
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
            if(variables.Exists(vname))
            {
                str=variables.Get(vname);
                xchg.changed=true;
            }
    }

    xchg.str = str;
    if(hasChanged)
        xchg.changed=true;
    return xchg;
}

std::string DefScriptPackage::_NormalizeVarName(std::string vn_in, std::string sn){
    std::string vn=vn_in;
    bool global=false;
    while(true)
    {
        if(sn.empty())
            return vn;
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

bool DefScriptPackage::Interpret(CmdSet Set)
{
    bool result=false;

    // first search if the script is defined in the internal functions
    for(unsigned int i=0;result==false;i++)
    {
        if(functionTable[i].func==NULL || functionTable[i].name==NULL) // reached the end of the table?
        {
            break;
        }
        if(Set.cmd==functionTable[i].name)
        {
            result=(this->*functionTable[i].func)(Set);
            break;
        }
    }

    // if nothing has been found its maybe an external script file to run
    if(!result)
    {
	    result=RunScript(Set.cmd, &Set);
        if((!result) /*&& Script[Set.cmd]->GetDebug()*/)
            std::cout << "Could not execute script command '" << Set.cmd << "'\n";
    }

    return result;
}


