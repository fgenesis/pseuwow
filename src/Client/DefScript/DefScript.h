
#ifndef __DEFSCRIPT_H
#define __DEFSCRIPT_H

#include <map>
#include <deque>
#include "VarSet.h"
#include "DynamicEvent.h"

#include "DefScriptDefines.h"

class DefScriptPackage;
class DefScript;


struct DefReturnResult
{
    DefReturnResult() { ok=true; mustreturn=false; ret="true"; }
    DefReturnResult(bool b) { ok=b; mustreturn=false; ret=b?"true":"false"; }
    bool ok; // true if the execution of the current statement was successful
    bool mustreturn;
    std::string ret; // return value used by ?{..}
    //bool abrt; // true if ALL current script execution must be aborted.
    //std::string err; // error string, including tracestack, etc.
};

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

struct DefScriptFunctionTable {
    char *name;
    DefReturnResult (DefScriptPackage::*func)(CmdSet& Set);
};

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
    void SetFunctionTable(DefScriptFunctionTable*);
    std::string _NormalizeVarName(std::string, std::string);
    DefReturnResult RunSingleLineFromScript(std::string line, DefScript *pScript);
    DefScript_DynamicEventMgr *GetEventMgr(void);
    
    std::string scPath;

    // Own executor functions
    void My_LoadUserPermissions(VarSet&);
    void My_Run(std::string line,std::string username);

private:
    DefXChgResult ReplaceVars(std::string str, CmdSet* pSet, unsigned char VarType);
	void SplitLine(CmdSet&,std::string);
    DefReturnResult Interpret(CmdSet&);
    void RemoveBrackets(CmdSet&);
    std::string RemoveBracketsFromString(std::string);
    DefScriptFunctionTable *_GetFunctionTable(void) const;
    DefScriptFunctionTable *functionTable;
    unsigned int functions;
    void *parentMethod;
    DefScript_DynamicEventMgr *_eventmgr;
    std::map<std::string,DefScript*> Script;
    std::map<std::string,unsigned char> scriptPermissionMap;

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

    // Useable own internal functions:
    DefReturnResult SCpause(CmdSet&);
    DefReturnResult SCSendChatMessage(CmdSet&);
    DefReturnResult SCsavecache(CmdSet&);
    DefReturnResult SCemote(CmdSet&);
    DefReturnResult SCfollow(CmdSet&);
    DefReturnResult SCshdn(CmdSet&);
    DefReturnResult SCjoinchannel(CmdSet&);
    DefReturnResult SCleavechannel(CmdSet&);
    DefReturnResult SCloadconf(CmdSet&);
    DefReturnResult SCapplypermissions(CmdSet&);
    DefReturnResult SCapplyconf(CmdSet&);
    DefReturnResult SClog(CmdSet&);
    DefReturnResult SClogdetail(CmdSet&);
    DefReturnResult SClogdebug(CmdSet&);
    DefReturnResult SClogerror(CmdSet&);
	DefReturnResult SCcastspell(CmdSet&);
    DefReturnResult SCqueryitem(CmdSet&);
    DefReturnResult SCtarget(CmdSet&);
    DefReturnResult SCloadscp(CmdSet&);

    // Own variable declarations
    std::map<std::string, unsigned char> my_usrPermissionMap;

};



#endif