#include "MPQFile.h"

// MPQ file to be opened
MPQFile::MPQFile(const char *fn)
{
    _mpq = NULL;
    _isopen = SFileOpenArchive(fn,0,0,&_mpq);
}

MPQFile::~MPQFile()
{
	Close();
}

bool MPQFile::HasFile(const char *fn)
{
    return SFileHasFile(_mpq,fn);
}

// get size of a file within an mpq archive
ByteBuffer MPQFile::ReadFile(const char *fn)
{
    ByteBuffer bb;
    HANDLE fh;
    if(!SFileOpenFileEx(_mpq, fn, 0, &fh))
        return bb;
    uint32 size = SFileGetFileSize(fh);
    bb.resize(size);
    SFileReadFile(fh, (void*)bb.contents(), size, NULL, NULL);
	SFileCloseFile(fh);
    return bb;
}

uint32 MPQFile::GetFileSize(const char *fn)
{
    HANDLE fh;
    if(!SFileOpenFileEx(_mpq, fn, 0, &fh))
        return 0;
    uint32 size = SFileGetFileSize(fh);
    SFileCloseFile(fh);
    return size;
}

void MPQFile::Close(void)
{
	if(_isopen)
		FreeMPQArchive((TMPQArchive*&)_mpq);
}

