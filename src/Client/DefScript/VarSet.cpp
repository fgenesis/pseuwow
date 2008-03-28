#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <algorithm>
#include <cctype>
#include "VarSet.h"

VarSet::VarSet()
{
}

VarSet::~VarSet()
{
	Clear();
}

std::string VarSet::Get(std::string varname)
{
    for(std::deque<Var>::iterator i=buffer.begin();i!=buffer.end();i++)
		if( i->name==varname )
			return i->value;
    return ""; // if var has not been set return empty string
}

void VarSet::Set(std::string varname, std::string varvalue)
{
	if(varname.empty())
        return;
	for(std::deque<Var>::iterator i = buffer.begin();i!=buffer.end();i++)
    {
		if( i->name==varname )
        {
			i->value=varvalue;
			return;
		}
	}
    Var v;
    v.name=varname;
    v.value=varvalue;
    buffer.push_back(v);
	return;
}

unsigned int VarSet::Size(void)
{
    return buffer.size();
}

bool VarSet::Exists(std::string varname)
{
	for(std::deque<Var>::iterator i = buffer.begin();i!=buffer.end();i++)
        if(i->name==varname)
            return true;
	return false;
}

void VarSet::Unset(std::string varname)
{
    if ( varname.empty() )
        return;
	for(std::deque<Var>::iterator i = buffer.begin();i!=buffer.end();i++)
    {
		if(i->name==varname)
        {
			buffer.erase(i);
			return;
		}
	}
}

void VarSet::Clear(void)
{
    buffer.clear();
}

Var VarSet::operator[](unsigned int id)
 {
     return buffer.at(id);
 }
	
bool VarSet::ReadVarsFromFile(std::string fn)
{
    std::fstream fh;
    std::string line,prefix,vn,vv;
    char c;
    bool upper=false,lower=false;

    fh.open(fn.c_str(),std::ios_base::in);
    if( !fh.is_open() )
        return false;
    while(!fh.eof())
    {
        c=fh.get();
        if(c=='\n' || c==13 || c==10 || fh.eof())
        {
            if(line.empty())
                continue;

            vn.clear();
            vv.clear();
            while(line.at(0)==' ' || line.at(0)=='\t')
                line.erase(0,1);
            //while(line.at(line.length()-1)==' ' || line.at(line.length()-1)=='\t')
            //    line.erase(line.length(),1);
            if(line.empty() || (line.at(0)=='/' && line.at(0)=='/') )
            {
                line.clear();
                continue;
            }
            if(line.at(0)=='[' && line.at(line.length()-1)==']')
            {
                std::string loadinfo;
                loadinfo=line.substr(1,line.length()-2);
                if(!loadinfo.empty())
                {
                    if(loadinfo.at(0)=='#')
                        loadinfo=toLower(loadinfo);
                    if(loadinfo=="#uppercase")
                    {
                        upper=true;
                        lower=false;
                    }
                    else if(loadinfo=="#normal")
                    {
                        upper=false;
                        lower=false;
                    }
                    else if(loadinfo=="#lowercase")
                    {
                        lower=true;
                        upper=false;
                    }
                    else if(loadinfo=="#noprefix")
                    {
                        prefix.clear();
                    }
                    else
                    {
                            prefix=loadinfo+"::";
                    }
                }
            }
            else
            {
                unsigned int pos=line.find("=");
                if(pos!=std::string::npos)
                {
                    std::string v=line.substr(0,pos);
                    
                    if(upper)
                        v=toUpper(v);
                    else if(lower)
                        v=toLower(v);

                    vn=prefix+v;
                    vv=line.substr(pos+1,line.length()-1);
                    Set(vn,vv);
#ifdef _DEBUG
                        printf("DEBUG: Var import [%s] = %s\n",vn.c_str(),vv.c_str());
#endif
                }
                // else invalid line, must have '='
            }
            line.clear();
        }
        else
            line+=c; // fill up line until a newline is reached (see above)
    }
    fh.close();
    return true;
}

std::string VarSet::toLower(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(), tolower);
	return s;
}

std::string VarSet::toUpper(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(), toupper);
	return s;
}
