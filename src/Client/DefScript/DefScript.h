
#ifndef __DEFSCRIPT_H
#define __DEFSCRIPT_H

#include <map>
#include <deque>
#include "VarSet.h"
#include "DynamicEvent.h"

#include "DefScriptDefines.h"

class DefScriptPackage;
class DefScript;


enum DefScriptBlockType
{
    BLOCK_IF,
    BLOCK_LOOP
};
// general struct for if..else..endif / loop..endloop blocks
struct Def_Block
{
    unsigned char type; // stores DefScriptBlockType
    bool istrue; // only relevant for if-statements
    unsigned int startline; // line where loop-statement started
};

struct DefReturnResult
{
    DefReturnResult() { ok=true; mustreturn=false; ret="true"; }
    DefReturnResult(bool b) { ok=true; mustreturn=false; ret=b?"true":"false"; }
    bool ok; // true if the execution of the current statement was successful
    bool mustreturn;
    std::string ret; // return value used by ?{..}
    //bool abrt; // true if ALL current script execution must be aborted.
    //std::string err; // error string, including tracestack, etc.
};

#define DEF_RETURN_ERROR { DefReturnResult __defreturnresult(false); __defreturnresult.ok=false; return __defreturnresult; }

struct DefXChgResult
{
    DefXChgResult() { changed=false; }
    bool changed;
    std::string str;
    DefReturnResult result;
};    

class CmdSet {
	public:
	CmdSet();
	~CmdSet();
	void Clear();
	std::string cmd;
	std::string arg[MAXARGS];
	std::string defaultarg;
    std::string myname;
    std::string caller;
};

struct DefScriptFunctionEntry {
    DefScriptFunctionEntry(std::string n,DefReturnResult (DefScriptPackage::*f)(CmdSet& Set))
    {
        name=n;
        func=f;
    }
    std::string name;
    DefReturnResult (DefScriptPackage::*func)(CmdSet& Set);
};

typedef std::deque<DefScriptFunctionEntry> DefScriptFunctionTable;

class DefScript {
public:
	DefScript(DefScriptPackage *p);
	~DefScript();
	std::string GetLine(unsigned int);
	unsigned int GetLines(void);
	bool AddLine(std::string );
	std::string GetName(void);
	void SetName(std::string);
	void SetPermission(unsigned char);
	unsigned char GetPermission(void);
	void Clear(void);
    void SetDebug(bool);
    bool GetDebug(void);
    //DefScriptPackage *GetParent(void);


private:
    std::deque<std::string> Line;
	unsigned int lines;
	std::string scriptname;
	unsigned char permission;
    bool debugmode;
    
    DefScriptPackage *_parent;   	
};


class DefScriptPackage {
public:
	DefScriptPackage();
	~DefScriptPackage();
    void SetParentMethod(void*); // used to extend the scripts with own c++ interface functions
	void Clear(void);
    DefScript *GetScript(std::string);
	unsigned int GetScripts(void);
	bool LoadScriptFromFile(std::string,std::string);
	DefReturnResult RunScript(std::string,CmdSet*);
    bool BoolRunScript(std::string,CmdSet*);
	unsigned int GetScriptID(std::string);
	DefReturnResult RunSingleLine(std::string);
	bool ScriptExists(std::string);
	VarSet variables;
    void SetPath(std::string);
    bool LoadByName(std::string);
    std::string _NormalizeVarName(std::string, std::string);
    DefReturnResult RunSingleLineFromScript(std::string line, DefScript *pScript);
    DefScript_DynamicEventMgr *GetEventMgr(void);
    void AddFunc(DefScriptFunctionEntry);
    void AddFunc(std::string n,DefReturnResult (DefScriptPackage::*)(CmdSet& Set));
    bool HasFunc(std::string);
    void DelFunc(std::string);

    
    std::string scPath;

    // Own executor functions
    void My_LoadUserPermissions(VarSet&);
    void My_Run(std::string line,std::string username);

private:
    void _InitFunctions(void);
    DefXChgResult ReplaceVars(std::string str, CmdSet* pSet, unsigned char VarType, bool run_embedded);
	void SplitLine(CmdSet&,std::string);
    DefReturnResult Interpret(CmdSet&);
    void RemoveBrackets(CmdSet&);
    std::string RemoveBracketsFromString(std::string);
    unsigned int functions;
    void *parentMethod;
    DefScript_DynamicEventMgr *_eventmgr;
    std::map<std::string,DefScript*> Script;
    std::map<std::string,unsigned char> scriptPermissionMap;
    DefScriptFunctionTable _functable;

    // Usable internal basic functions:
    DefReturnResult func_default(CmdSet&);
    DefReturnResult func_set(CmdSet&);
    DefReturnResult func_unset(CmdSet&);
    DefReturnResult func_loaddef(CmdSet&);
    DefReturnResult func_reloaddef(CmdSet&);
    DefReturnResult func_out(CmdSet&);
    DefReturnResult func_eof(CmdSet&);
    DefReturnResult func_shdn(CmdSet&);
    DefReturnResult func_setscriptpermission(CmdSet&);
    DefReturnResult func_toint(CmdSet&);
    DefReturnResult func_add(CmdSet&);
    DefReturnResult func_sub(CmdSet&);
    DefReturnResult func_mul(CmdSet&);
    DefReturnResult func_div(CmdSet&);
    DefReturnResult func_mod(CmdSet&);
    DefReturnResult func_pow(CmdSet&);
    DefReturnResult func_bitor(CmdSet&);
    DefReturnResult func_bitand(CmdSet&);
    DefReturnResult func_bitxor(CmdSet&);
    DefReturnResult func_addevent(CmdSet&);
    DefReturnResult func_removeevent(CmdSet&);
    DefReturnResult func_abs(CmdSet&);
    DefReturnResult func_bigger(CmdSet&);
    DefReturnResult func_equal(CmdSet&);
    DefReturnResult func_isset(CmdSet&);
    DefReturnResult func_not(CmdSet&);
    DefReturnResult func_smaller(CmdSet&);
    DefReturnResult func_strlen(CmdSet&);
    DefReturnResult func_tohex(CmdSet&);

    // setup own function declarations here
#   include "DefScriptInterfaceInclude.h"

    // Own variable declarations
    std::map<std::string, unsigned char> my_usrPermissionMap;

};



#endif