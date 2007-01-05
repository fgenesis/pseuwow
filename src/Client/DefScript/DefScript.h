
#ifndef __DEFSCRIPT_H
#define __DEFSCRIPT_H

#define MAXARGS 99

#include <map>
#include "VarSet.h"

class DefScriptPackage;

class CmdSet {
	public:
	CmdSet();
	~CmdSet();
	void Clear();
	std::string cmd;
	std::string arg[MAXARGS];
	std::string defaultarg;
    DefScriptPackage *pack;
    std::string caller;
    void *ptr;
};

struct DefScriptFunctionTable {
    char *name;
    bool (DefScriptPackage::*func)(CmdSet Set);
};

class DefScript {
public:
	DefScript();
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
	std::string *Line;
	unsigned int lines;
	std::string scriptname;
	unsigned char permission;
    bool debugmode;
    
    //DefScriptPackage *_parent;
    //CmdSet _mySet;
   	
};


class DefScriptPackage {
public:
	DefScriptPackage();
	~DefScriptPackage();
    void SetParentMethod(void*);
	void Clear(void);
	DefScript GetScript(unsigned int);
	unsigned int GetScripts(void);
	bool LoadScriptFromFile(std::string,std::string);
	bool RunScriptByName(std::string,CmdSet*,unsigned char);
	bool RunScriptByID(unsigned int,CmdSet*,unsigned char);
	std::string GetScriptName(unsigned int);
	unsigned int GetScriptID(std::string);
	bool RunSingleLine(std::string, unsigned char);
	bool ScriptExists(std::string);
	VarSet variables;
    void SetPath(std::string);
    bool LoadByName(std::string);
    void SetFunctionTable(DefScriptFunctionTable*);
    std::string _NormalizeVarName(std::string, std::string);
    bool DefScriptPackage::CallFunction(bool *func, CmdSet *pSet);

    std::string scPath;

private:
    std::string ReplaceVars(std::string, CmdSet*, bool, unsigned int, std::string); // changes the string directly
	CmdSet SplitLine(std::string);
    bool Interpret(CmdSet, std::string, unsigned char);
    CmdSet RemoveBrackets(CmdSet);
    std::string RemoveBracketsFromString(std::string);
    DefScriptFunctionTable *_GetFunctionTable(void) const;

    bool curIsDebug;
	unsigned int scripts;
	DefScript *Script;
    unsigned int recursionLevel;
    DefScriptFunctionTable *functionTable;
    unsigned int functions;
    void *parentMethod;

    std::map<std::string, unsigned int> permissionMap;

    // Usable internal basic functions:
    bool func_default(CmdSet);
    bool func_set(CmdSet);
    bool func_unset(CmdSet);
    bool func_loaddef(CmdSet);
    bool func_reloaddef(CmdSet);
    bool func_out(CmdSet);
    bool func_eof(CmdSet);
    bool func_shdn(CmdSet);

    // Useable own internal functions:
    bool SCpause(CmdSet);
    bool SCSendChatMessage(CmdSet);
    bool SCsavecache(CmdSet);
    bool SCemote(CmdSet);
    bool SCfollow(CmdSet);

};



#endif