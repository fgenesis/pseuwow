#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <algorithm>
#include "DefScript.h"

using namespace DefScriptTools;

DefReturnResult DefScriptPackage::func_lpushback(CmdSet& Set)
{
	DefList *l = lists.Get(_NormalizeVarName(Set.arg[0],Set.myname));
	l->push_back(Set.defaultarg);
	return true;
}

DefReturnResult DefScriptPackage::func_lpushfront(CmdSet& Set)
{
	DefList *l = lists.Get(_NormalizeVarName(Set.arg[0],Set.myname));
	l->push_front(Set.defaultarg);
	return true;
}

DefReturnResult DefScriptPackage::func_lpopback(CmdSet& Set)
{
    std::string r;
	DefList *l = lists.GetNoCreate(_NormalizeVarName(Set.defaultarg,Set.myname));
    if( (!l) || (!l->size()) ) // cant pop any element if the list doesnt exist or is empty
        return "";
	r= l->back();
	l->pop_back();
	return r;
}

DefReturnResult DefScriptPackage::func_lpopfront(CmdSet& Set)
{
    std::string r;
	DefList *l = lists.GetNoCreate(_NormalizeVarName(Set.defaultarg,Set.myname));
    if( (!l) || (!l->size()) ) // cant pop any element if the list doesnt exist or is empty
        return "";
	r = l->front();
	l->pop_front();
	return r;
}

// delete a list and all its elements
DefReturnResult DefScriptPackage::func_ldelete(CmdSet& Set)
{
    std::string lname = _NormalizeVarName(Set.defaultarg,Set.myname);
    if(strncmp(lname.c_str(), SCRIPT_NAMESPACE,strlen(SCRIPT_NAMESPACE))==0)
    {
        printf("DefScript: WARNING: ldelete used on a script list, clearing instead! (called by '%s', list '%s')\n",Set.myname.c_str(), lname.c_str());
        DefList *l = lists.GetNoCreate(lname);
        if(l)
            l->clear();
        return true;
    }
	lists.Delete(lname);
	return true;
}

// returns true if a list with the given name exists, else false
DefReturnResult DefScriptPackage::func_lexists(CmdSet& Set)
{
	return lists.Exists(_NormalizeVarName(Set.defaultarg,Set.myname));
}

// return list element count
DefReturnResult DefScriptPackage::func_llen(CmdSet& Set)
{
	DefList *l = lists.GetNoCreate(_NormalizeVarName(Set.defaultarg,Set.myname));
    if(!l)
        return ""; // return nothing if list doesnt exist
    return toString((uint64)l->size()); // return any number
}

// insert item at some position in the list
DefReturnResult DefScriptPackage::func_linsert(CmdSet& Set)
{
	bool result;
	DefList *l = lists.Get(_NormalizeVarName(Set.arg[0],Set.myname));
	unsigned int pos = (unsigned int)toNumber(Set.arg[1]);
	if(pos > l->size()) // if the list is too short to insert at that pos...
	{
		l->push_back(Set.defaultarg); // ... just append at the end
		result = false;
	}
	else
	{
        DefList::iterator it = l->begin();
        advance(it,pos);
		l->insert(it,Set.defaultarg); // ... else insert at correct position
		result = true;
	}
	return result;
}

// returns the amount of string fragments added to the list
// arg0: list name; arg1: delimiter string; defaultarg: string to split
DefReturnResult DefScriptPackage::func_lsplit(CmdSet& Set)
{
	// 1st create a new list, or get an already existing one and clear it
	DefList *l = lists.Get(_NormalizeVarName(Set.arg[0],Set.myname));
    l->clear();
	if(Set.defaultarg.empty()) // we cant split an empty string, return nothing, and keep empty list
		return "";

    // special case: empty delimiter -> split in chars
    if(Set.arg[1].empty())
    {
        for(unsigned int i=0; i<Set.defaultarg.length(); i++)
        {
            std::string tmp;
            tmp = Set.defaultarg[i];
            l->push_back(tmp);
        }
        return toString((uint64)l->size());
    }

	unsigned int p,q=0; // p=position of substr; q=next pos to start searching at
    while( (p = Set.defaultarg.find(Set.arg[1].c_str(),q)) != std::string::npos)
	{
		l->push_back( Set.defaultarg.substr(q,p - q) );
		q = p + Set.arg[1].length();
	}
	if(q < Set.defaultarg.length()) // also append the last string fragment (that has no delimiter)
    {
		l->push_back(Set.defaultarg.c_str() + q);
    }
	return toString((uint64)l->size());
}

// multi-split by chars
// returns the amount of string fragments added to the list
// arg0: list name; args: delimit; defaultarg: string to split
DefReturnResult DefScriptPackage::func_lcsplit(CmdSet& Set)
{
	// 1st create a new list, or get an already existing one and clear it
	DefList *l = lists.Get(_NormalizeVarName(Set.arg[0],Set.myname));
    l->clear();
	if(Set.defaultarg.empty()) // we cant split an empty string, return nothing, and keep empty list
		return "";

	unsigned int p,q=0; // p=position of substr; q=next pos to start searching at
    while( (p = Set.defaultarg.find_first_of(Set.arg[1].c_str(),q)) != std::string::npos)
	{
		l->push_back( Set.defaultarg.substr(q,p - q) );
		q = p + 1;
	}
	if(q < Set.defaultarg.length()) // also append the last string fragment (that has no delimiter)
    {
		l->push_back(Set.defaultarg.c_str() + q);
    }
	return toString((uint64)l->size());
}

// create a string from a list, using <defaultarg> as delimiter
DefReturnResult DefScriptPackage::func_ljoin(CmdSet& Set)
{
    std::string r;
    DefList *l = lists.GetNoCreate(_NormalizeVarName(Set.arg[0],Set.myname));
	if(!l)
		return "";
    unsigned int start_from = (unsigned int)toUint64(Set.arg[1]);
    unsigned int end_at = (unsigned int)toUint64(Set.arg[2]);
    if(!end_at)
        end_at = l->size();
	for(unsigned int i = start_from; i < end_at; i++)
	{
		r += (*l)[i];
		if( i+1 != l->size() )
			r += Set.defaultarg;
	}			
	return r;
}

// return list item at position xx
DefReturnResult DefScriptPackage::func_lindex(CmdSet& Set)
{
    DefList *l = lists.GetNoCreate(_NormalizeVarName(Set.arg[0],Set.myname));
    if(!l)
        return "";
    unsigned int pos = (unsigned int)toNumber(Set.defaultarg);
    if(pos+1 > l->size()) // ot of bounds?
        return "";
    return (*l)[pos];
}

// clean list: remove every element that matches @def
// use _only_ this function to remove empty strings from a list
DefReturnResult DefScriptPackage::func_lclean(CmdSet& Set)
{
    unsigned int r=0;
    DefList *l = lists.GetNoCreate(_NormalizeVarName(Set.arg[0],Set.myname));
    if(!l)
        return "";
    for(DefList::iterator i=l->begin(); i!=l->end(); )
    {
        if(*i == Set.defaultarg)
        {
            i = l->erase(i);
            r++;
            continue;
        }
        i++;
    }
    return toString((uint64)r);
}

// multi-clean list: remove every element that matches any of the args, if it isn't empty
// NOTE: arg0 = list name, this one is skipped!
// this func does NOT remove empty strings from a list, use lclean instead!!
DefReturnResult DefScriptPackage::func_lmclean(CmdSet& Set)
{
    unsigned int r=0;
    DefList *l = lists.GetNoCreate(_NormalizeVarName(Set.arg[0],Set.myname));
    if(!l)
        return "";

    _CmdSetArgMap::iterator it=Set.arg.begin();
    advance(it,1); // skip list name

    for( ; it != Set.arg.end(); it++)
    {
        if(it->second.length())
        {
            for(DefList::iterator i=l->begin(); i!=l->end(); i++)
            {
                if(*i == it->second)
                {
                    l->erase(i);
                    r++;
                    if(i==l->end()) // must be checked here, else causing crash (?!)
                        break;
                }
            }
        }
    }

    // erase defaultarg if given
    if(Set.defaultarg.length())
    {
        for(DefList::iterator i=l->begin(); i!=l->end(); i++)
        {
            if(*i == Set.defaultarg)
            {
                l->erase(i);
                r++;
            }
        }
    }
    return toString((uint64)r);
}

// erase element at position @def, return erased element
DefReturnResult DefScriptPackage::func_lerase(CmdSet& Set)
{
    DefList *l = lists.GetNoCreate(_NormalizeVarName(Set.arg[0],Set.myname));
    if(!l)
        return "";
    std::string r;
    unsigned int pos = (unsigned int)toNumber(Set.defaultarg);
    if(pos > l->size()) // if the list is too short to erase at that pos...
        return ""; // ... return nothing
 
    DefList::iterator it = l->begin();
    advance(it,pos);
    r = *it;
    l->erase(it); // ... else erase at correct position

    return r;
}

DefReturnResult DefScriptPackage::func_lsort(CmdSet& Set)
{
    DefList *l = lists.GetNoCreate(_NormalizeVarName(Set.defaultarg,Set.myname));
    if(!l)
        return false;
    sort(l->begin(),l->end());
    return true;
}






//DefReturnResult DefScriptPackage::func_lmerge(CmdSet& Set)
//{

// to be continued...

	
		
		
	
	
	
	
	

