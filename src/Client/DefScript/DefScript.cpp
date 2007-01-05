#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "VarSet.h"
#include "DefScript.h"

struct DefScriptXChgResult {
    std::string str;
    bool change;
};

// --- SECTION FOR SCRIPT PACKAGES ---
DefScriptPackage::DefScriptPackage(){
	scripts=0;
	Script=NULL;
    curIsDebug=false;
    recursionLevel=0;
    functions=0;
    functionTable=_GetFunctionTable();

    

//    printf("---> DefScriptPackage inited!\n");
}

DefScriptPackage::~DefScriptPackage(){
	Clear();		
}

void DefScriptPackage::SetParentMethod(void *p)
{
    parentMethod = p;
}

void DefScriptPackage::Clear(void){
	//for(unsigned int i=0;i<scripts;i++){
	//	Script[i].Clear();
	//}
	delete [] Script;
	Script=NULL;
	scripts=0;
}

DefScriptFunctionTable *DefScriptPackage::_GetFunctionTable(void) const
{
    static DefScriptFunctionTable table[] = {
        // basic functions:
        {"set",&DefScriptPackage::func_set},
        {"defult",&DefScriptPackage::func_default},
        {"unset",&DefScriptPackage::func_unset},
        {"shdn",&DefScriptPackage::func_shdn},
        {"loaddef",&DefScriptPackage::func_loaddef},
        {"reloaddef",&DefScriptPackage::func_reloaddef},

        // user functions:
        {"pause",&DefScriptPackage::SCpause},
        {"emote",&DefScriptPackage::SCemote},
        //{"follow",&DefScriptPackage::SCfollow},
        {"savecache",&DefScriptPackage::SCsavecache},
        {"sendchatmessage",&DefScriptPackage::SCSendChatMessage},

        // table termination
        {NULL,NULL}

    };
    return table;
}

void DefScriptPackage::SetFunctionTable(DefScriptFunctionTable *tab){
    functionTable=tab;
}

void DefScriptPackage::SetPath(std::string p){
    scPath=p;
}

DefScript DefScriptPackage::GetScript(unsigned int id){
	return Script[id];
}

unsigned int DefScriptPackage::GetScripts(void){
	return scripts;
}

std::string DefScriptPackage::GetScriptName(unsigned int id){
	if(id>scripts) return NULL;
	return Script[id].GetName();
}

// returns array position!
unsigned int DefScriptPackage::GetScriptID(std::string name){
	for(unsigned int i=0;i<scripts;i++){
		if( !Script[i].GetName().empty() && Script[i].GetName()==name ){
			return  i;
		}
	}
	return 0; // Be careful you check if the script exists before you trust it. this could
			  // also mean Script[0]!!
}

bool DefScriptPackage::ScriptExists(std::string name){
	if(Script==NULL) return false;
	for(unsigned int i=0;i<scripts;i++){
		if( !Script[i].GetName().empty() && Script[i].GetName()==name ){
			return true;
		}
	}
	return false;
}

bool DefScriptPackage::LoadByName(std::string name){
    return LoadScriptFromFile((scPath+name).append(".def"),name);
}

bool DefScriptPackage::LoadScriptFromFile(std::string fn, std::string sn){
	bool increase_flag=false;
	std::string label, value;
	unsigned int id;
	if(fn.empty() || sn.empty()) return false;	
	if(ScriptExists(sn)){ // if the script already exists, clear it and reassign content
		id=GetScriptID(sn); // script exists, get ID...
		Script[id].Clear(); // ... and remove content to refill it later on
	} else { // if not, set scripts as ID for new script. scripts is equal to new array index then
		id=scripts;
		increase_flag=true;	

		// Script does not yet exist, since the scriptcount will be increased we have to alloc mem before
        DefScript *Script_old;
        if(scripts){		    
            Script_old=new DefScript[scripts];
		    memcpy(Script_old,Script,scripts*sizeof(DefScript));
		    delete [] Script;
        }
		Script = new DefScript[scripts+1];
        if(scripts){
		    memcpy(Script,Script_old,scripts*sizeof(DefScript));
            delete [] Script_old;
        }
  
	}

    std::fstream f;
    f.open(fn.c_str(),std::ios_base::in);
    if(!f.is_open())
        return false;
	std::string line;
    char z;
    bool load_debug=false,load_notify=false;
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
		if(line.at(0)=='#'){
			line.erase(0,1); // remove #
			label=line.substr(0,line.find('=',0));
			value=line.substr(line.find('=',0)+1,line.length());
			if(label=="permission"){
				Script[id].SetPermission((unsigned char)atoi(value.c_str())); // MAYBE: instead of this use Var @permission
			}  // ...	
            if(line=="load_debug")
                load_debug=true;
            if(line=="load_notify")
                load_notify=true;
            if(line=="debug")
                Script[id].SetDebug(true);
            //...
            continue; // line was an option, not script content
		}
        if(load_debug)
            std::cout<<"~LOAD: "<<line<<"\n";
		Script[id].AddLine(line);
		
		
	}
	f.close();
	//Script[id].AddLine("eof"); // to be sure the script is terminated correctly
    Script[id].SetName(sn);
    if(load_notify)
        std::cout << "+> Script '" << sn << "' [" << fn << "] successfully loaded.\n";
	
	// ...
	if(increase_flag) scripts++;
    return true;
}
	

// --- SECTION FOR THE INDIVIDUAL SCRIPTS IN A PACKAGE ---

DefScript::DefScript(){
	lines=0;
	Line=NULL;
    permission=0;
	scriptname="{NONAME}";
    debugmode=false;
//    printf("--> DefScript inited!\n");
}

DefScript::~DefScript(){
	Clear();
}

void DefScript::Clear(void){
	//if(lines)
    //    delete [] Line; //?! causes crash!
	Line=NULL;
	lines=0;
    permission=0;
}

void DefScript::SetDebug(bool d){
    debugmode=d;
}

bool DefScript::GetDebug(void){
    return debugmode;
}

void DefScript::SetName(std::string name){
	scriptname=name;
}

std::string DefScript::GetName(void){
	return scriptname;
}

void DefScript::SetPermission(unsigned char p){
	permission=p;
}

unsigned char DefScript::GetPermission(void){
	return permission;
}

unsigned int DefScript::GetLines(void){
	return lines;
}

std::string DefScript::GetLine(unsigned int id){
	if(id>lines)
		return "";
	return Line[id];
}

bool DefScript::AddLine(std::string l){
	if(l.empty())
		return false;
	std::string *Line_old=new std::string[lines];
	for(unsigned int i=0;i<lines;i++)
		Line_old[i]=Line[i];
	delete [] Line;
    Line=new std::string[lines+1];
	for(unsigned int i=0;i<lines;i++)
		Line[i]=Line_old[i];
    Line[lines]=l;
	lines++;
    delete [] Line_old;
	return true;
}


// --- SECTION FOR COMMAND SETS ---

CmdSet::CmdSet(){
	for(unsigned int i=0;i<MAXARGS;i++){
		arg[i]="";
	}
	cmd="";
    defaultarg="";
}

CmdSet::~CmdSet(){
	Clear();
}

void CmdSet::Clear(){
	
}


// --- FUNCTIONS TO PARSE AND EXECUTE A SCRIPT --- PARENT: DefScriptPackage!

bool DefScriptPackage::RunScriptByID(unsigned int id, CmdSet *pSet, unsigned char p){
    if(id>GetScripts())
        return false;

    curIsDebug=GetScript(id).GetDebug();

	for(unsigned int cur_line=0;cur_line<GetScript(id).GetLines();cur_line++){
           
		std::string line=GetScript(id).GetLine(cur_line);
        std::string final=ReplaceVars(line,pSet,false,0,GetScript(id).GetName()); // must only parse globals if pSet == NULL
		CmdSet curSet=SplitLine(final);
		Interpret(curSet,GetScript(id).GetName(), p);
	}

	curIsDebug=false;
    return true;
}

bool DefScriptPackage::RunScriptByName(std::string scname, CmdSet *pSet, unsigned char p){
	if(scname.empty())
        return false;
	if(ScriptExists(scname)){
		RunScriptByID(GetScriptID(scname),pSet,p);
        return true;
    } else {
        bool result=LoadByName(scname);
        if(result){
            RunScriptByID(GetScriptID(scname),pSet,p);
            return true;
        }
    }
    return false;
		
}

bool DefScriptPackage::RunSingleLine(std::string line, unsigned char p){
    unsigned int temp=0;
    std::string final=ReplaceVars(line,NULL,false,temp,"");
	CmdSet curSet=SplitLine(final);
    return Interpret(curSet,"",p);
}

CmdSet DefScriptPackage::SplitLine(std::string line){	
	
	unsigned int i=0;
	unsigned int bracketsOpen=0,curParam=0;
    bool cmdDefined=false;
    std::string tempLine;
	CmdSet outSet;

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

    outSet.cmd.assign(strlwr((char*)outSet.cmd.c_str()));
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


std::string DefScriptPackage::ReplaceVars(std::string str, CmdSet *pSet, bool isVar, unsigned int vardepth, std::string sc_name){
   //std::cout<<">>ReplaceVars: '"<<str<<"'  var="<<isVar<<"\n";
    unsigned int  
        
        openingBracket=0, // defines the position from where the recursive call should be started
        closingBracket=0, // the closing bracket
        bracketsOpen=0,
        bLen=0; // the lenth of the string in brackets, e.g. ${abc} == 3

    bool nextIsVar=false, hasVar=false;


    std::string subStr,retStr;
    char temp;
	
	while(str.at(0)==' ' || str.at(0)=='\t')
		str.erase(0,1); // trim spaces if there are any

    for(unsigned int i=0;i<str.length();i++){
        temp=str.at(i);


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
                    i-=2;
                    continue;
                }
                else
                {
                    bLen=closingBracket-openingBracket-1;
                    subStr=str.substr(openingBracket+1,bLen);
                    retStr=ReplaceVars(subStr,pSet,nextIsVar,vardepth+(nextIsVar?1:0),sc_name);
                    if( hasVar && !nextIsVar && subStr!=retStr )
                    {
                        str.erase(openingBracket+1,subStr.length());
                        str.insert(openingBracket+1,retStr);
                        hasVar=false;
                        nextIsVar=false;
                        i-=(subStr.length()+1);
                    }
                    else if( hasVar && nextIsVar && subStr!=retStr )
                    {
                        str.erase(openingBracket-1,bLen+3); // remove {...
                        i-=(bLen+2); // adjust position
                        str.insert(i,retStr);
                        //i--;
                        hasVar=false;
                        nextIsVar=false;
                    }
                    else
                        continue;
                }
            }
       } // end if {
    } // end for
    if(!bracketsOpen && isVar && vardepth)
    {
        std::string vname=_NormalizeVarName(str, sc_name);
        if(vname[0]=='@')
        {
            std::stringstream vns;
            std::string subs=vname.substr(1,str.length()-1);
            unsigned int vn=atoi( subs.c_str() );
            vns << vn;
            if(pSet && vns.str()==subs)
                str=pSet->arg[vn];
            else if(pSet && subs=="def")
                str=pSet->defaultarg;
            else if(pSet && subs=="cmd")
                str=pSet->cmd;
            //else if(pSet && subs=="parent")
            //    str=pSet->parent;
            else if(variables.Exists(vname))
                str=variables.Get(vname);
            else
            {
                // TODO: call custom macro table
                //...
                str.clear();
            }


        }
        else
            if(variables.Exists(vname))
                str=variables.Get(vname);
            else if(!variables.Exists(vname) && !subStr.empty())
                str="${"+str+"}";
    }

   
    return str;
}

std::string DefScriptPackage::_NormalizeVarName(std::string vn_in, std::string sn){
    std::string vn=vn_in;
    if(sn.empty())
        return vn;
    if(vn.at(0)=='#')
		while(vn.at(0)=='#')
			vn.erase(0,1);
    else if(vn.at(0)=='@')
        ;// do nothing for now
    else
        vn=sn+"::"+vn;

    return vn;
}

bool DefScriptPackage::Interpret(CmdSet Set, std::string sc_name, unsigned char p){
    bool result=false;

    for(unsigned int i=0;result==false;i++){
        if(functionTable[i].func==NULL || functionTable[i].name==NULL){ // reached the end of the table?
            break;
        }
        if(Set.cmd==functionTable[i].name){
            result=(this->*functionTable[i].func)(Set);
            break;
        }
    }

    // if still nothing has been found its maybe a script command
    if(!result){
        unsigned int perm=GetScript(GetScriptID(Set.cmd)).GetPermission();
        if(p<perm)
            return false; // permisson level too low
		
	    result=RunScriptByName(Set.cmd,&Set,255); // call the subscript, this time with full privileges
        if(!result && curIsDebug)
            std::cout << "Could not execute script command '" << Set.cmd << "'\n";
    }

    return result;
}

// TODO: how to get this work?!
/*
bool DefScriptPackage::CallFunction(bool *func, CmdSet *pSet){
    //bool result = *func(pSet, this);
    //return result;
}
*/


