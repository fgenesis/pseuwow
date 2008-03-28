#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include "DefScript.h"

using namespace DefScriptTools;


DefReturnResult DefScriptPackage::func_shdn(CmdSet& Set){
    DefReturnResult r;
    //exit(0);
    // need to define own
    SCshdn(Set);
    return r;
}

DefReturnResult DefScriptPackage::func_out(CmdSet& Set){
    DefReturnResult r;
    printf("%s\n",Set.defaultarg.c_str());
    r.ret=Set.defaultarg;
    return r;
}

DefReturnResult DefScriptPackage::func_loaddef(CmdSet& Set){
    if( ScriptExists(Set.defaultarg) )
    {
        return "exists";
    }
    return func_reloaddef(Set);
}

// supply either the filename in default scripts directory, without extension,
// OR provide the full file path WITH extension
DefReturnResult DefScriptPackage::func_reloaddef(CmdSet& Set){
    DefReturnResult r;
    bool result=false;
    std::string fn;

    // if the filename does NOT contain any path, load from default script dir
    std::string::size_type slashpos = Set.defaultarg.find_last_of("\\/");
    std::string::size_type ppos = Set.defaultarg.find_last_of(".");
    if(slashpos == std::string::npos)
    {
        fn=scPath+Set.defaultarg;
    }
    else
    {
        fn=Set.defaultarg;
    }
    if(ppos==std::string::npos || ppos < slashpos) // even if there was neither / nor . they will be equal
        fn+=".def";
    
    result=LoadScriptFromFile(fn);

    r.ret=fn;
    if(!result)
    {
        //std::cout << "Could not load script '" << Set.defaultarg << "' [" << fn << "]\n";
        r.ret="";
    }
    return r;
}

DefReturnResult DefScriptPackage::func_unloaddef(CmdSet& Set)
{
    if(!ScriptExists(Set.defaultarg))
        return false;
    this->DeleteScript(Set.defaultarg);
    return true;
}

DefReturnResult DefScriptPackage::func_createdef(CmdSet& Set)
{
    std::string sn = stringToLower(Set.defaultarg);
    if(!ScriptExists(sn))
    {
        _UpdateOrCreateScriptByName(sn);
        return true;
    }
    return false;
}

DefReturnResult DefScriptPackage::func_unset(CmdSet& Set){
    DefReturnResult r;
    r.ret=Set.defaultarg;
    if(Set.defaultarg.empty()){
        //if(curIsDebug)
        //    printf("Can't unset, no variable name given.\n");
        return r;
    }
    if(Set.defaultarg.at(0)=='@'){
        //if(curIsDebug)
        //    printf("Can't unset macros!\n");
        return r;
    }
    std::string vn=_NormalizeVarName(Set.defaultarg, Set.caller);
    variables.Unset(vn);
    //std::cout<<"Unset var '"<<Set->defaultarg<<"'\n";
    return r;
}

DefReturnResult DefScriptPackage::func_set(CmdSet& Set)
{
    DefReturnResult r;
    if(Set.arg[0].empty())
    {
        //if(curIsDebug)
        //   printf("Can't assign value, no variable name given.\n");
        return r;
    }
    Set.arg[0]=UnescapeString(Set.arg[0]);
    if(Set.arg[0].at(0)=='@')
    {
        //if(curIsDebug)
        //    printf("Can't assign value to a macro!\n");
        return r;
    }
    std::string vname,vval=Set.defaultarg;
    vname=_NormalizeVarName(Set.arg[0], Set.myname);

   //if(!stricmp(Set.arg[1].c_str(),"onfail") && vval.find("${")!=std::string::npos)
   //     vval=Set.arg[2];

    variables.Set(vname,vval);
    r.ret=vval;
    
    DefScript *sc = GetScript(Set.myname);
    if(sc && sc->GetDebug())
        printf("VAR: %s = '%s'\n",vname.c_str(),vval.c_str());

    return r;
}

DefReturnResult DefScriptPackage::func_default(CmdSet& Set)
{
    DefReturnResult r;
    if(Set.arg[0].empty())
    {
        //if(curIsDebug)
        //   printf("Can't assign value, no variable name given.\n");
        return r;
    }
    Set.arg[0]=UnescapeString(Set.arg[0]);
    if(Set.arg[0].at(0)=='@')
    {
        //if(curIsDebug)
        //    printf("Can't assign value to a macro!\n");
        return r;
    }
    std::string vname,vval=Set.defaultarg;
    vname=_NormalizeVarName(Set.arg[0], Set.caller);

    if(variables.Get(vname).empty())
    {
        variables.Set(vname,vval); // set only if it has no value or the var doesnt exist
        r.ret=vval;
    }
    else
    {
        r.ret=variables.Get(vname);
    }

    return r;
}

DefReturnResult DefScriptPackage::func_setscriptpermission(CmdSet& Set)
{
    DefReturnResult r;
    if(Set.defaultarg.empty() || Set.arg[0].empty())
        return r;

    scriptPermissionMap[Set.arg[0]] = (unsigned char)toUint64(Set.defaultarg);
    return r;
}

DefReturnResult DefScriptPackage::func_toint(CmdSet& Set)
{
    DefReturnResult r;
    std::string num=toString(toUint64(Set.defaultarg));
    if(!Set.arg[0].empty())
    {
        std::string vname=_NormalizeVarName(Set.arg[0], Set.myname);
        variables.Set(vname,num);
    }
    r.ret=num;
    return r;
}

DefReturnResult DefScriptPackage::func_add(CmdSet& Set)
{
    DefReturnResult r;
    if(Set.arg[0].empty())
    {
        return r;
    }

    std::string vname=_NormalizeVarName(Set.arg[0], Set.myname);
    ldbl a=toNumber(variables.Get(vname));
    ldbl b=toNumber(Set.defaultarg);
    a+=b;
    variables.Set(vname,toString(a));
    r.ret=toString(a);
    return r;
}

DefReturnResult DefScriptPackage::func_sub(CmdSet& Set)
{
    DefReturnResult r;
    if(Set.arg[0].empty())
    {
        return r;
    }

    std::string vname=_NormalizeVarName(Set.arg[0], Set.myname);
    ldbl a=toNumber(variables.Get(vname));
    ldbl b=toNumber(Set.defaultarg);
    a-=b;
    variables.Set(vname,toString(a));
    r.ret=toString(a);
    return r;
}

DefReturnResult DefScriptPackage::func_mul(CmdSet& Set)
{
    DefReturnResult r;
    if(Set.arg[0].empty())
    {
        return r;
    }

    std::string vname=_NormalizeVarName(Set.arg[0], Set.myname);
    ldbl a=toNumber(variables.Get(vname));
    ldbl b=toNumber(Set.defaultarg);
    a*=b;
    variables.Set(vname,toString(a));
    r.ret=toString(a);
    return r;
}

DefReturnResult DefScriptPackage::func_div(CmdSet& Set)
{
    DefReturnResult r;
    if(Set.arg[0].empty())
    {
        return r;
    }

    std::string vname=_NormalizeVarName(Set.arg[0], Set.myname);
    ldbl a=toNumber(variables.Get(vname));
    ldbl b=toNumber(Set.defaultarg);
    if(b==0)
        a=0;
    else
        a/=b;
    variables.Set(vname,toString(a));
    r.ret=toString(a);
    return r;
}

DefReturnResult DefScriptPackage::func_mod(CmdSet& Set)
{
    DefReturnResult r;
    if(Set.arg[0].empty())
    {
        return r;
    }

    std::string vname=_NormalizeVarName(Set.arg[0], Set.myname);
    uint64 a=toUint64(variables.Get(vname));
    uint64 b=toUint64(Set.defaultarg);
    if(b==0)
        a=0;
    else
        a%=b;
    variables.Set(vname,toString(a));
    r.ret=toString(a);
    return r;
}

DefReturnResult DefScriptPackage::func_pow(CmdSet& Set)
{
    DefReturnResult r;
    if(Set.arg[0].empty())
    {
        return r;
    }

    std::string vname=_NormalizeVarName(Set.arg[0], Set.myname);
    ldbl a=toNumber(variables.Get(vname));
    ldbl b=toNumber(Set.defaultarg);
    a=pow(a,b);
    variables.Set(vname,toString(a));
    r.ret=toString(a);
    return r;
}

DefReturnResult DefScriptPackage::func_bitor(CmdSet& Set)
{
    DefReturnResult r;
    if(Set.arg[0].empty())
    {
        return r;
    }

    std::string vname=_NormalizeVarName(Set.arg[0], Set.myname);
    uint64 a=toUint64(variables.Get(vname));
    uint64 b=toUint64(Set.defaultarg);
    a|=b;
    variables.Set(vname,toString(a));
    r.ret=toString(a);
    return r;
}

DefReturnResult DefScriptPackage::func_bitand(CmdSet& Set)
{
    DefReturnResult r;
    if(Set.arg[0].empty())
    {
        return r;
    }

    std::string vname=_NormalizeVarName(Set.arg[0], Set.myname);
    uint64 a=toUint64(variables.Get(vname));
    uint64 b=toUint64(Set.defaultarg);
    a&=b;
    variables.Set(vname,toString(a));
    r.ret=toString(a);
    return r;
}

DefReturnResult DefScriptPackage::func_bitxor(CmdSet& Set)
{
    DefReturnResult r;
    if(Set.arg[0].empty())
    {
        return r;
    }

    std::string vname=_NormalizeVarName(Set.arg[0], Set.myname);
    uint64 a=toUint64(variables.Get(vname));
    uint64 b=toUint64(Set.defaultarg);
    a^=b;
    variables.Set(vname,toString(a));
    r.ret=toString(a);
    return r;
}

DefReturnResult DefScriptPackage::func_addevent(CmdSet& Set)
{
    GetEventMgr()->Add(Set.arg[0],Set.defaultarg,(clock_t)toNumber(Set.arg[1]),Set.myname.c_str(),isTrue(Set.arg[2]));
    return true;
}

DefReturnResult DefScriptPackage::func_removeevent(CmdSet& Set)
{
    GetEventMgr()->Remove(Set.defaultarg);
    return true;
}

DefReturnResult DefScriptPackage::func_strlen(CmdSet& Set)
{
    DefReturnResult r;
    r.ret=toString((uint64)Set.defaultarg.length());
    return r;
}

DefReturnResult DefScriptPackage::func_equal(CmdSet& Set)
{
    bool result;
    if(stringToLower(Set.arg[1])=="ignore")
    {
        result=stringToLower(Set.defaultarg)==stringToLower(Set.arg[0]);
    }
    else
    {
        result=Set.defaultarg==Set.arg[0];
    }

    // for debugging
    _DEFSC_DEBUG
    (
        char tmp[500];
        sprintf(tmp,"DEFSCRIPT: func_equal: ['%s'=='%s'] = %s\n",Set.arg[0].c_str(),Set.defaultarg.c_str(),result?"true":"false");
        hLogfile << tmp;
    );
    return result;
}


DefReturnResult DefScriptPackage::func_smaller(CmdSet& Set)
{
    return toNumber(Set.arg[0]) < toNumber(Set.defaultarg);
}

DefReturnResult DefScriptPackage::func_bigger(CmdSet& Set)
{
    return toNumber(Set.arg[0]) > toNumber(Set.defaultarg);
}

DefReturnResult DefScriptPackage::func_smaller_eq(CmdSet& Set)
{
    return toNumber(Set.arg[0]) <= toNumber(Set.defaultarg);
}

DefReturnResult DefScriptPackage::func_bigger_eq(CmdSet& Set)
{
    return toNumber(Set.arg[0]) >= toNumber(Set.defaultarg);
}

DefReturnResult DefScriptPackage::func_not(CmdSet& Set)
{
    return !isTrue(Set.defaultarg);
}

DefReturnResult DefScriptPackage::func_isset(CmdSet& Set)
{
    std::string vname=_NormalizeVarName(Set.defaultarg,Set.myname);
    return variables.Exists(vname);
}

DefReturnResult DefScriptPackage::func_tohex(CmdSet& Set)
{
    DefReturnResult r;
    char buf[50];
    bool full=stringToLower(Set.arg[0])=="full";
    uint64 u=toUint64(Set.defaultarg);
#if COMPILER == COMPILER_MICROSOFT
    sprintf(buf,"%016I64X",u);
#else
    sprintf(buf,"%016llX",u);
#endif
    std::string str=buf;
    if(!full)
        while(str.length() && str.at(0)=='0')
            str.erase(0,1);
    r.ret = std::string("0x").append(str);
    return r;
}

DefReturnResult DefScriptPackage::func_abs(CmdSet& Set)
{
    DefReturnResult r;
    r.ret=toString(fabs(toNumber(Set.defaultarg)));
    return r;
}

DefReturnResult DefScriptPackage::func_and(CmdSet& Set)
{
    return isTrue(Set.defaultarg) && isTrue(Set.arg[0]);
}

DefReturnResult DefScriptPackage::func_or(CmdSet& Set)
{
    return isTrue(Set.defaultarg) || isTrue(Set.arg[0]);
}

DefReturnResult DefScriptPackage::func_xor(CmdSet& Set)
{
    return (isTrue(Set.defaultarg) && isTrue(Set.arg[0])) || ( (!isTrue(Set.defaultarg)) && (!isTrue(Set.arg[0]) ) );
}

DefReturnResult DefScriptPackage::func_substr(CmdSet& Set)
{
    DefReturnResult r;
    if(Set.defaultarg.empty())
    {
        r.ret="";
    }
    else
    {
        unsigned int start,len;
        len=(unsigned int)toUint64(Set.arg[0]);
        start=(unsigned int)toUint64(Set.arg[1]);
        if(start+len>Set.defaultarg.length())
            len=Set.defaultarg.length()-start;
        r.ret=Set.defaultarg.substr(start,len);
    }
    return r;
}

DefReturnResult DefScriptPackage::func_uppercase(CmdSet& Set)
{
    DefReturnResult r;
    r.ret=stringToUpper(Set.defaultarg);
    return r;
}

DefReturnResult DefScriptPackage::func_lowercase(CmdSet& Set)
{
    DefReturnResult r;
    r.ret=stringToLower(Set.defaultarg);
    return r;
}

DefReturnResult DefScriptPackage::func_random(CmdSet& Set)
{
    DefReturnResult r;
    int min,max;
    min=(int)toUint64(Set.arg[0]);
    max=(int)toUint64(Set.defaultarg);
    r.ret=toString(min + ( rand() % (max - min + 1)) );
    return r;
}

DefReturnResult DefScriptPackage::func_fileexists(CmdSet& Set)
{
    std::fstream f;
    f.open(Set.defaultarg.c_str(),std::ios_base::in);
    if (f.is_open())
    {
        f.close();
        return true;
    }
    return false;
}

DefReturnResult DefScriptPackage::func_strfind(CmdSet& Set)
{
    unsigned int pos = Set.defaultarg.find(Set.arg[0],(unsigned int)toNumber(Set.arg[1]));
    if(pos == std::string::npos)
        return "";
    return toString((uint64)pos);    
}

DefReturnResult DefScriptPackage::func_scriptexists(CmdSet& Set)
{
    return ScriptExists(stringToLower(Set.defaultarg));
}

DefReturnResult DefScriptPackage::func_funcexists(CmdSet& Set)
{
    for(DefScriptFunctionTable::iterator i = _functable.begin(); i != _functable.end(); i++)
    {
        if(i->name == stringToLower(Set.defaultarg))
            return true;
    }
    return false;
}
