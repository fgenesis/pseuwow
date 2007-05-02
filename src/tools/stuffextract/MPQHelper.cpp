#include <vector>
#include "common.h"
#include "MPQHelper.h"
#include "MPQFile.h"
#include "Locale.h"

#define DATADIR "Data"


MPQHelper::MPQHelper()
{
}

// supply without ".mpq" !!
bool MPQHelper::AssignArchive(char *archive)
{
	// old code for 1.12.x and below archives order
	/* 
    // first, check which patch files are avalible.
    // store patches in reversed order, that patch-9.mpq is checked first, and patch.mpq checked last
    if(FileExists(DATADIR"/patch.MPQ"))
        _patches.push_front("Data/patch.MPQ");
    for(uint32 i=1; i<MAX_PATCH_NUMBER; i++)
    {
        char buf[20];
        sprintf(buf,DATADIR"/patch-%u.MPQ",i);
        if(FileExists(buf))
            _patches.push_front(buf);
    }
    // then assign the original archive name
    _archive = archive;
    return FileExists(archive);
	*/

	// new code for 2.0.x and above
	// TODO: check which files are needed and which are not + recheck for correct ordering
	std::string dir = "Data/";
	std::string ext = ".MPQ";
	std::string ldir = dir + GetLocale() + "/";
	
	// order goes from last opened to first opened file
	// ok maybe this is a bit too much but should work fine :)
	_patches.push_front(dir+"common"+ext);
	_patches.push_front(dir+"expansion"+ext);
	for(uint32 i=1; i<=MAX_PATCH_NUMBER; i++)
	{
		char buf[200];
		sprintf(buf,"%spatch-%u%s",dir.c_str(),i,ext.c_str());
		_patches.push_front(buf);
	}
	_patches.push_front(dir+"patch"+ext);
	_patches.push_front(ldir+archive+"-"+GetLocale()+ext);
	_patches.push_front(ldir+"locale-"+GetLocale()+ext);
	_patches.push_front(ldir+"expansion-locale-"+GetLocale()+ext);	
	_patches.push_front(ldir+"expansion-"+archive+"-"+GetLocale()+ext);

	_patches.push_front(ldir+"patch"+"-"+GetLocale()+ext);
	for(uint32 i=1; i<=MAX_PATCH_NUMBER; i++)
	{
		char buf[200];
		sprintf(buf,"%spatch-%s-%u%s",ldir.c_str(),GetLocale(),i,ext.c_str());
		//if(FileExists(buf))
			_patches.push_front(buf);
	}

	_archive = archive;
	return FileExists(dir+archive+ext);
}

ByteBuffer MPQHelper::ExtractFile(char* fn)
{
    ByteBuffer bb;
    for(std::list<std::string>::iterator i = _patches.begin(); i != _patches.end(); i++)
    {
        MPQFile mpq((*i).c_str());
        if(mpq.IsOpen() && mpq.HasFile(fn) && mpq.GetFileSize(fn) > 0)
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




