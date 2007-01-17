#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include "DefScript.h"



bool DefScriptPackage::func_shdn(CmdSet Set){
    //exit(0);
    // need to define own
    SCshdn(Set);
    return true;
}

bool DefScriptPackage::func_eof(CmdSet Set){
    // do nothing still
    return true;
}

bool DefScriptPackage::func_out(CmdSet Set){
    printf("%s\n",Set.defaultarg.c_str());
    return true;
}

bool DefScriptPackage::func_loaddef(CmdSet Set){
    if( ScriptExists(Set.defaultarg) )
        return true;
    bool result=false;
    std::string fn;
    if(Set.arg[0].empty())
        result=LoadByName(Set.defaultarg);
    else
    {
        std::string::size_type pos = Set.arg[0].find('/');
        if(pos == std::string::npos)
            fn=scPath+Set.arg[0];
        else
            fn=Set.arg[0];
        result=LoadScriptFromFile(fn,Set.defaultarg);
    }
    //if(!result && curIsDebug)
    //    std::cout << "Could not load script '" << Set->defaultarg << "' [" << fn << "]\n";
    return result;
}

bool DefScriptPackage::func_reloaddef(CmdSet Set){
    bool result=false;
    std::string fn;
    if(Set.arg[0].empty())
        result=LoadByName(Set.defaultarg);
    else
    {
        std::string::size_type pos = Set.arg[0].find('/');
        if(pos == std::string::npos)
            fn=scPath+Set.arg[0];
        else
            fn=Set.arg[0];
        result=LoadScriptFromFile(fn,Set.defaultarg);
    }
    //if(!result && curIsDebug)
    //    std::cout << "Could not load script '" << Set->defaultarg << "' [" << fn << "]\n";
    return result;
}

bool DefScriptPackage::func_unset(CmdSet Set){
    if(Set.defaultarg.empty()){
        //if(curIsDebug)
        //    printf("Can't unset, no variable name given.\n");
        return false;
    }
    if(Set.defaultarg.at(0)=='@'){
        //if(curIsDebug)
        //    printf("Can't unset macros!\n");
        return false;
    }
    std::string vn=_NormalizeVarName(Set.defaultarg, Set.caller);
    variables.Unset(vn);
    //std::cout<<"Unset var '"<<Set->defaultarg<<"'\n";
    return true;
}

bool DefScriptPackage::func_set(CmdSet Set){
    if(Set.arg[0].empty()){
        //if(curIsDebug)
        //   printf("Can't assign value, no variable name given.\n");
        return false;
    }
    if(Set.arg[0].at(0)=='@'){
        //if(curIsDebug)
        //    printf("Can't assign value to a macro!\n");
        return false;
    }
    std::string vname,vval=Set.defaultarg;
    vname=_NormalizeVarName(Set.arg[0], Set.myname);

   if(!stricmp(Set.arg[1].c_str(),"onfail") && vval.find("${")!=std::string::npos)
        vval=Set.arg[2];

    variables.Set(vname,vval);

    if(Set.owner && Set.owner->GetDebug())
        printf("VAR: %s = '%s'\n",vname.c_str(),vval.c_str());

    return true;
}

bool DefScriptPackage::func_default(CmdSet Set){
    if(Set.arg[0].empty()){
        //if(curIsDebug)
        //   printf("Can't assign value, no variable name given.\n");
        return false;
    }
    if(Set.arg[0].at(0)=='@'){
        //if(curIsDebug)
        //    printf("Can't assign value to a macro!\n");
        return false;
    }
    std::string vname,vval=Set.defaultarg;
    vname=_NormalizeVarName(Set.arg[0], Set.caller);

    if(variables.Get(vname).empty())
        variables.Set(vname,vval); // set only if it has no value

    return true;
}

bool DefScriptPackage::func_setscriptpermission(CmdSet Set)
{
    if(Set.defaultarg.empty() || Set.arg[0].empty())
        return false;

    scriptPermissionMap[Set.arg[0]] = atoi(Set.defaultarg.c_str());
    return true;
}

bool DefScriptPackage::func_toint(CmdSet Set)
{
    if(Set.arg[0].empty())
        return false;

    std::string vname=_NormalizeVarName(Set.arg[0], Set.myname);
    def_int64 myint = strtol(Set.defaultarg.c_str(), NULL, 10);
    std::stringstream ss;
    ss << myint;
    variables.Set(vname,ss.str());
    return true;
}

bool DefScriptPackage::func_add(CmdSet Set)
{
    if(Set.arg[0].empty())
        return false;

    std::string vname=_NormalizeVarName(Set.arg[0], Set.myname);
    long long myvar = strtol(variables.Get(vname).c_str(), NULL, 10);
    def_int64 myadd = strtol(Set.defaultarg.c_str(), NULL, 10);
    myvar += myadd;
    std::stringstream ss;
    ss << myvar;
    variables.Set(vname,ss.str());
    return true;
}

bool DefScriptPackage::func_sub(CmdSet Set)
{
    if(Set.arg[0].empty())
        return false;

    std::string vname=_NormalizeVarName(Set.arg[0], Set.myname);
    def_int64 myvar = strtol(variables.Get(vname).c_str(), NULL, 10);
    def_int64 myadd = strtol(Set.defaultarg.c_str(), NULL, 10);
    myvar -= myadd;
    std::stringstream ss;
    ss << myvar;
    variables.Set(vname,ss.str());
    return true;
}

bool DefScriptPackage::func_mul(CmdSet Set)
{
    if(Set.arg[0].empty())
        return false;

    std::string vname=_NormalizeVarName(Set.arg[0], Set.myname);
    def_int64 myvar = strtol(variables.Get(vname).c_str(), NULL, 10);
    def_int64 myadd = strtol(Set.defaultarg.c_str(), NULL, 10);
    myvar *= myadd;
    std::stringstream ss;
    ss << myvar;
    variables.Set(vname,ss.str());
    return true;
}

bool DefScriptPackage::func_div(CmdSet Set)
{
    if(Set.arg[0].empty())
        return false;

    std::string vname=_NormalizeVarName(Set.arg[0], Set.myname);
    def_int64 myvar = strtol(variables.Get(vname).c_str(), NULL, 10);
    def_int64 myadd = strtol(Set.defaultarg.c_str(), NULL, 10);
    if(myadd==0)
    {
        // TODO: some error handling
        variables.Set(vname,"0");
    }
    myvar /= myadd;
    std::stringstream ss;
    ss << myvar;
    variables.Set(vname,ss.str());
    return true;
}

bool DefScriptPackage::func_mod(CmdSet Set)
{
    if(Set.arg[0].empty())
        return false;

    std::string vname=_NormalizeVarName(Set.arg[0], Set.myname);
    def_int64 myvar = strtol(variables.Get(vname).c_str(), NULL, 10);
    def_int64 myadd = strtol(Set.defaultarg.c_str(), NULL, 10);
    myvar %= myadd;
    std::stringstream ss;
    ss << myvar;
    variables.Set(vname,ss.str());
    return true;
}

bool DefScriptPackage::func_pow(CmdSet Set)
{
    if(Set.arg[0].empty())
        return false;

    std::string vname=_NormalizeVarName(Set.arg[0], Set.myname);
    def_int64 myvar = strtol(variables.Get(vname).c_str(), NULL, 10);
    def_int64 myadd = strtol(Set.defaultarg.c_str(), NULL, 10);
    long double myd=(long double)myvar,mya=(long double)myadd;
    myd = (long double)pow(myd,mya);
    myvar = (def_int64)myd;
    std::stringstream ss;
    ss << myvar;
    variables.Set(vname,ss.str());
    return true;
}

bool DefScriptPackage::func_bitor(CmdSet Set)
{
    if(Set.arg[0].empty())
        return false;

    std::string vname=_NormalizeVarName(Set.arg[0], Set.myname);
    def_int64 myvar = strtol(variables.Get(vname).c_str(), NULL, 10);
    def_int64 myadd = strtol(Set.defaultarg.c_str(), NULL, 10);
    myvar |= myadd;
    std::stringstream ss;
    ss << myvar;
    variables.Set(vname,ss.str());
    return true;
}

bool DefScriptPackage::func_bitand(CmdSet Set)
{
    if(Set.arg[0].empty())
        return false;

    std::string vname=_NormalizeVarName(Set.arg[0], Set.myname);
    def_int64 myvar = strtol(variables.Get(vname).c_str(), NULL, 10);
    def_int64 myadd = strtol(Set.defaultarg.c_str(), NULL, 10);
    myvar &= myadd;
    std::stringstream ss;
    ss << myvar;
    variables.Set(vname,ss.str());
    return true;
}

bool DefScriptPackage::func_bitxor(CmdSet Set)
{
    if(Set.arg[0].empty())
        return false;

    std::string vname=_NormalizeVarName(Set.arg[0], Set.myname);
    def_int64 myvar = strtol(variables.Get(vname).c_str(), NULL, 10);
    def_int64 myadd = strtol(Set.defaultarg.c_str(), NULL, 10);
    myvar ^= myadd;
    std::stringstream ss;
    ss << myvar;
    variables.Set(vname,ss.str());
    return true;
}
