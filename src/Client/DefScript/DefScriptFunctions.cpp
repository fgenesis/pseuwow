#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include "DefScript.h"
#include "DefScriptTools.h"

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
    DefReturnResult r;
    if( ScriptExists(Set.defaultarg) )
    {
        r.ret="exists";
        return r;
    }
    return func_reloaddef(Set);
}

DefReturnResult DefScriptPackage::func_reloaddef(CmdSet& Set){
    DefReturnResult r;
    bool result=false;
    std::string fn;
    if(Set.arg[0].empty())
    {
        result=LoadByName(Set.defaultarg);
        fn=(scPath + Set.defaultarg).append(".def");
    }
    else
    {
        std::string::size_type pos = Set.arg[0].find('/');
        if(pos == std::string::npos)
            fn=scPath+Set.arg[0];
        else
            fn=Set.arg[0];
        result=LoadScriptFromFile(fn,Set.defaultarg);
    }
    r.ret=fn;
    if(!result)
    {
        std::cout << "Could not load script '" << Set.defaultarg << "' [" << fn << "]\n";
        r.ret="";
    }
    return r;
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

DefReturnResult DefScriptPackage::func_set(CmdSet& Set){
    DefReturnResult r;
    if(Set.arg[0].empty()){
        //if(curIsDebug)
        //   printf("Can't assign value, no variable name given.\n");
        return r;
    }
    if(Set.arg[0].at(0)=='@'){
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

DefReturnResult DefScriptPackage::func_default(CmdSet& Set){
    DefReturnResult r;
    if(Set.arg[0].empty()){
        //if(curIsDebug)
        //   printf("Can't assign value, no variable name given.\n");
        return r;
    }
    if(Set.arg[0].at(0)=='@'){
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

    scriptPermissionMap[Set.arg[0]] = atoi(Set.defaultarg.c_str());
    return r;
}

DefReturnResult DefScriptPackage::func_toint(CmdSet& Set)
{
    DefReturnResult r;
    std::string num=toString(floor(toNumber(Set.defaultarg)));
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
    uint64 a=(uint64)toNumber(variables.Get(vname));
    uint64 b=(uint64)toNumber(Set.defaultarg);
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
    uint64 a=(uint64)toNumber(variables.Get(vname));
    uint64 b=(uint64)toNumber(Set.defaultarg);
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
    uint64 a=(uint64)toNumber(variables.Get(vname));
    uint64 b=(uint64)toNumber(Set.defaultarg);
    a^=b;
    variables.Set(vname,toString(a));
    r.ret=toString(a);
    return r;
}

DefReturnResult DefScriptPackage::func_addevent(CmdSet& Set)
{
    GetEventMgr()->Add(Set.arg[0],Set.defaultarg,(clock_t)toNumber(Set.arg[1]),Set.myname.c_str());
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
    if(stringToLower(Set.arg[1])=="ignore")
    {
        return stringToLower(Set.defaultarg)==stringToLower(Set.arg[0]);
    }
    return Set.defaultarg==Set.arg[0];
}

DefReturnResult DefScriptPackage::func_smaller(CmdSet& Set)
{
    return toNumber(Set.arg[0]) < toNumber(Set.defaultarg);
}

DefReturnResult DefScriptPackage::func_bigger(CmdSet& Set)
{
    return toNumber(Set.arg[0]) > toNumber(Set.defaultarg);
}

DefReturnResult DefScriptPackage::func_not(CmdSet& Set)
{
    return !isTrue(Set.defaultarg);
}

DefReturnResult DefScriptPackage::func_isset(CmdSet& Set)
{
    return variables.Exists(Set.defaultarg);
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
    unsigned int start,len;
    len=(unsigned int)toNumber(Set.arg[0]);
    start=(unsigned int)toNumber(Set.arg[1]);
    if(start+len<Set.defaultarg.length())
        len=Set.defaultarg.length()-start;
    r.ret=Set.defaultarg.substr(start,len);
    return r;
}

