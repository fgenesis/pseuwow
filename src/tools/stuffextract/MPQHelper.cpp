#include "common.h"
#include "MPQHelper.h"
#include "MPQFile.h"


MPQHelper::MPQHelper()
{
}

bool MPQHelper::AssignArchive(char *fn)
{
    // first, check which patch files are avalible.
    // store patches in reversed order, that patch-9.mpq is checked first, and patch.mpq checked last
    if(FileExists("Data/patch.MPQ"))
        _patches.push_front("Data/patch.MPQ");
    for(uint32 i=1; i<MAX_PATCH_NUMBER; i++)
    {
        char buf[20];
        sprintf(buf,"patch-%u.MPQ",i);
        if(FileExists(buf))
            _patches.push_front(buf);
    }
    // then assign the original archive name
    _archive = fn;
    return FileExists(fn);
}

ByteBuffer MPQHelper::ExtractFile(char* fn)
{
    ByteBuffer bb;
    for(std::list<std::string>::iterator i = _patches.begin(); i != _patches.end(); i++)
    {
        MPQFile mpq((*i).c_str());
        if(mpq.IsOpen() && mpq.HasFile(fn))
        {
            printf("MPQE: Using %s from %s\n",fn,i->c_str());
            bb = mpq.ReadFile(fn);
            return bb;
        }
    }
    MPQFile arch(_archive.c_str());
    if(arch.IsOpen() && arch.HasFile(fn))
    {
        printf("MPQE: Using %s from %s\n",fn,_archive.c_str());
        bb = arch.ReadFile(fn);
    }
    return bb;
}




