
#ifndef __DEFSCRIPT_H
#define __DEFSCRIPT_H

#define MAXARGS 99
#ifdef _DEBUG
#    define _DEFSC_DEBUG(code) code;
#else
#    define _DEFSC_DEBUG(code) /* code */
#endif

#if COMPILER == COMPILER_MICROSOFT
typedef __int64            def_int64;
#else
typedef __int64_t   def_int64;
#endif



#include <map>
#include <deque>
#include "VarSet.h"

class DefScriptPackage;
class DefScript;

struct DefXChgResult {
    DefXChgResult() { changed=false; }
    bool changed;
    std::string str;
};

class CmdSet {
	public:
	CmdSet(DefScript *p);
	~CmdSet();
	void Clear();
	std::string cmd;
	std::string arg[MAXARGS];
	std::string defaultarg;
    std::string myname;
    std::string caller;
    DefScript *owner;
    void *ptr;
};

struct DefScriptFunctionTable {
    char *name;
    bool (DefScriptPackage::*func)(CmdSet Set);
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
    //CmdSet _mySet;
   	
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
	bool RunScript(std::string,CmdSet*);
	unsigned int GetScriptID(std::string);
	bool RunSingleLine(std::string);
	bool ScriptExists(std::string);
	VarSet variables;
    void SetPath(std::string);
    bool LoadByName(std::string);
    void SetFunctionTable(DefScriptFunctionTable*);
    std::string _NormalizeVarName(std::string, std::string);

    std::string scPath;

    // Own executor functions
    void My_LoadUserPermissions(VarSet&);
    bool My_Run(std::string line,std::string username);

private:
    DefXChgResult ReplaceVars(std::string, CmdSet*, bool);
	CmdSet SplitLine(std::string);
    bool Interpret(CmdSet);
    CmdSet RemoveBrackets(CmdSet);
    std::string RemoveBracketsFromString(std::string);
    bool RunSingleLineFromScript(std::string line, DefScript *pScript);
    DefScriptFunctionTable *_GetFunctionTable(void) const;
    DefScriptFunctionTable *functionTable;
    unsigned int functions;
    void *parentMethod;
    std::map<std::string,DefScript*> Script;
    std::map<std::string,unsigned char> scriptPermissionMap;

    // Usable internal basic functions:
    bool func_default(CmdSet);
    bool func_set(CmdSet);
    bool func_unset(CmdSet);
    bool func_loaddef(CmdSet);
    bool func_reloaddef(CmdSet);
    bool func_out(CmdSet);
    bool func_eof(CmdSet);
    bool func_shdn(CmdSet);
    bool func_setscriptpermission(CmdSet);
    bool func_toint(CmdSet);
    bool func_add(CmdSet);
    bool func_sub(CmdSet);
    bool func_mul(CmdSet);
    bool func_div(CmdSet);
    bool func_mod(CmdSet);
    bool func_pow(CmdSet);
    bool func_bitor(CmdSet);
    bool func_bitand(CmdSet);
    bool func_bitxor(CmdSet);

    // Useable own internal functions:
    bool SCpause(CmdSet);
    bool SCSendChatMessage(CmdSet);
    bool SCsavecache(CmdSet);
    bool SCemote(CmdSet);
    bool SCfollow(CmdSet);
    bool SCshdn(CmdSet);
    bool SCjoinchannel(CmdSet);
    bool SCleavechannel(CmdSet);
    bool SCloadconf(CmdSet);
    bool SCapplypermissions(CmdSet);
    bool SCapplyconf(CmdSet);
    bool SClog(CmdSet);
    bool SClogdetail(CmdSet);
    bool SClogdebug(CmdSet);

    // Own variable declarations
    std::map<std::string, unsigned char> my_usrPermissionMap;

};



#endif