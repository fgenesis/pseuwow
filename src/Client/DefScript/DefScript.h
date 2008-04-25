
#ifndef __DEFSCRIPT_H
#define __DEFSCRIPT_H

#include "DefScriptDefines.h"
#include <map>
#include <deque>
#include <fstream>
#include "VarSet.h"
#include "ByteBuffer.h"
#include "DynamicEvent.h"
#include "TypeStorage.h"
#include "DefScriptTools.h"

class DefScriptPackage;
class DefScript;

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
	DefReturnResult(std::string s) { ok=true; mustreturn=false; ret=s; }
    DefReturnResult(const char *s) { ok=true; mustreturn=false; ret=s; }
    DefReturnResult(char *s) { ok=true; mustreturn=false; ret=s; }
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

typedef std::map<unsigned int,std::string> _CmdSetArgMap;

class CmdSet {
	public:
	CmdSet();
	~CmdSet();
	void Clear();
	std::string cmd;
	_CmdSetArgMap arg;
	std::string defaultarg;
    std::string myname;
    std::string caller;
};

struct DefScriptFunctionEntry {
    DefScriptFunctionEntry(std::string n,DefReturnResult (DefScriptPackage::*f)(CmdSet& Set), bool esc)
    {
        name=n;
        func=f;
        escape=esc;
    }
    std::string name;
    DefReturnResult (DefScriptPackage::*func)(CmdSet& Set);
    bool escape;
};

typedef std::deque<DefScriptFunctionEntry> DefScriptFunctionTable;

typedef std::deque<std::string> DefList;
typedef std::map<std::string,DefList*> DefListMap;

class DefScript {
    friend class DefScriptPackage;
public:
	DefScript(DefScriptPackage *p);
	~DefScript();

    inline std::string GetLine(unsigned int id) { return Line[id]; }
    inline unsigned int GetLines(void) { return Line.size(); }
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
    DefList Line;
	unsigned int lines;
	std::string scriptname;
	unsigned char permission;
    bool debugmode;
    
    DefScriptPackage *_parent;   	
};


class DefScriptPackage {
    friend class DefScript;
public:
	DefScriptPackage();
	~DefScriptPackage();
    void SetParentMethod(void*); // used to extend the scripts with own c++ interface functions
	void Clear(void);
    DefScript *GetScript(std::string);
	unsigned int GetScripts(void);
	bool LoadScriptFromFile(std::string);
    DefReturnResult RunScript(std::string name,CmdSet* pSet,std::string override_name="");
    bool BoolRunScript(std::string,CmdSet*);
    bool RunScriptIfExists(std::string name, CmdSet *pSet = NULL);
	unsigned int GetScriptID(std::string);
	DefReturnResult RunSingleLine(std::string);
	bool ScriptExists(std::string);
    void DeleteScript(std::string);
	VarSet variables;
    void SetPath(std::string);
    bool LoadByName(std::string);
    std::string _NormalizeVarName(std::string, std::string);
    DefReturnResult RunSingleLineFromScript(std::string line, DefScript *pScript);
    DefScript_DynamicEventMgr *GetEventMgr(void);
    void AddFunc(DefScriptFunctionEntry);
    void AddFunc(std::string n,DefReturnResult (DefScriptPackage::*)(CmdSet& Set), bool esc=true);
    bool HasFunc(std::string);
    void DelFunc(std::string);
	TypeStorage<DefList> lists;
    TypeStorage<ByteBuffer> bytebuffers;
    TypeStorage<std::fstream> files;
    std::string SecureString(std::string);
    std::string EscapeString(std::string);
    std::string UnescapeString(std::string);
    std::string GetUnescapedVar(std::string);
    
    std::string scPath;

    // Own executor functions
    void My_LoadUserPermissions(VarSet&);
    void My_Run(std::string line,std::string username);

private:
    void _UpdateOrCreateScriptByName(std::string);
    void _InitFunctions(void);
    DefXChgResult ReplaceVars(std::string str, CmdSet* pSet, unsigned char VarType, bool run_embedded);
	void SplitLine(CmdSet&,std::string);
    DefReturnResult Interpret(CmdSet&);
    void RemoveBrackets(CmdSet&);
    void UnescapeSet(CmdSet&);
    std::string RemoveBracketsFromString(std::string);
    void def_print(const char *fmt, ...);
    void *parentMethod;
    DefScript_DynamicEventMgr *_eventmgr;
    std::map<std::string,DefScript*> Script;
    std::map<std::string,unsigned char> scriptPermissionMap;
    DefScriptFunctionTable _functable;
    _DEFSC_DEBUG(std::fstream hLogfile);

    // Usable internal basic functions:
    DefReturnResult func_default(CmdSet&);
    DefReturnResult func_set(CmdSet&);
    DefReturnResult func_unset(CmdSet&);
    DefReturnResult func_loaddef(CmdSet&);
    DefReturnResult func_reloaddef(CmdSet&);
    DefReturnResult func_createdef(CmdSet&);
    DefReturnResult func_unloaddef(CmdSet&);
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
    DefReturnResult func_bigger_eq(CmdSet&);
    DefReturnResult func_equal(CmdSet&);
    DefReturnResult func_isset(CmdSet&);
    DefReturnResult func_not(CmdSet&);
    DefReturnResult func_smaller(CmdSet&);
    DefReturnResult func_smaller_eq(CmdSet&);
    DefReturnResult func_strlen(CmdSet&);
    DefReturnResult func_tohex(CmdSet&);
    DefReturnResult func_and(CmdSet&);
    DefReturnResult func_or(CmdSet&);
    DefReturnResult func_xor(CmdSet&);
    DefReturnResult func_substr(CmdSet&);
    DefReturnResult func_uppercase(CmdSet&);
    DefReturnResult func_lowercase(CmdSet&);
    DefReturnResult func_random(CmdSet&);
    DefReturnResult func_fileexists(CmdSet&);
    DefReturnResult func_strfind(CmdSet&);
    DefReturnResult func_scriptexists(CmdSet&);
    DefReturnResult func_funcexists(CmdSet&);


    // list functions
    DefReturnResult func_lpushback(CmdSet&);
    DefReturnResult func_lpushfront(CmdSet&);
    DefReturnResult func_lpopback(CmdSet&);
    DefReturnResult func_lpopfront(CmdSet&);
    DefReturnResult func_ldelete(CmdSet&);
    DefReturnResult func_lexists(CmdSet&);
    DefReturnResult func_llen(CmdSet&);
    DefReturnResult func_linsert(CmdSet&);
    DefReturnResult func_lsplit(CmdSet&);
    DefReturnResult func_lcsplit(CmdSet&);
    DefReturnResult func_ljoin(CmdSet&);
    DefReturnResult func_lindex(CmdSet&);
    DefReturnResult func_lclean(CmdSet&);
    DefReturnResult func_lmclean(CmdSet&);
    DefReturnResult func_lerase(CmdSet&);
    DefReturnResult func_lsort(CmdSet&);

    // ByteBuffer functions
    DefReturnResult func_bbinit(CmdSet&);
    DefReturnResult func_bbdelete(CmdSet&);
    DefReturnResult func_bbappend(CmdSet&);
    DefReturnResult func_bbread(CmdSet&);
    DefReturnResult func_bbsetrpos(CmdSet&);
    DefReturnResult func_bbsetwpos(CmdSet&);
    DefReturnResult func_bbhexlike(CmdSet&);
    DefReturnResult func_bbtextlike(CmdSet&);
    DefReturnResult func_bbsize(CmdSet&);

    // file functions
    DefReturnResult func_fopen(CmdSet&);
    DefReturnResult func_fclose(CmdSet&);
    DefReturnResult func_fisopen(CmdSet&);
    DefReturnResult func_feof(CmdSet&);
    DefReturnResult func_frpos(CmdSet&);
    DefReturnResult func_fwpos(CmdSet&);
    DefReturnResult func_fdel(CmdSet&);
    DefReturnResult func_fflush(CmdSet&);
    DefReturnResult func_fwrite(CmdSet&);
    DefReturnResult func_fread(CmdSet&);
    DefReturnResult func_freadb(CmdSet&);
    DefReturnResult func_fwriteb(CmdSet&);
    DefReturnResult func_freadbb(CmdSet&);
    DefReturnResult func_fwritebb(CmdSet&);
    DefReturnResult func_fsize(CmdSet&);
    DefReturnResult func_freadline(CmdSet&);
    DefReturnResult func_fseekw(CmdSet&);
    DefReturnResult func_fseekr(CmdSet&);

    // setup own function declarations here
#   include "DefScriptInterfaceInclude.h"

    // Own variable declarations
    std::map<std::string, unsigned char> my_usrPermissionMap;

};



#endif
