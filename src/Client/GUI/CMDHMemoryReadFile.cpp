// Copyright (C) 2002-2007 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "MemoryDataHolder.h"
#include "CMDHMemoryReadFile.h"
#include "irrlicht/irrString.h"

namespace irr
{
namespace io
{


CMDHReadFile::CMDHReadFile(void* memory, long len, const c8* fileName)
: Buffer(memory), Len(len), Pos(0)
{
	#ifdef _DEBUG
	setDebugName("CReadFile");
	#endif

	Filename = fileName;
}



CMDHReadFile::~CMDHReadFile()
{
    // We should not drop the memory here... this model will possibly be loaded more than once
    //if(getReferenceCount() <= 1)
    //    MemoryDataHolder::Delete(getFileName());
}



//! returns how much was read
irr::s32 CMDHReadFile::read(void* buffer, irr::u32 sizeToRead)
{
	irr::s32 amount = static_cast<irr::s32>(sizeToRead);
	if (Pos + amount > Len)
		amount -= Pos + amount - Len;

	if (amount <= 0)
		return 0;

	irr::c8* p = (irr::c8*)Buffer;
	memcpy(buffer, p + Pos, amount);
	
	Pos += amount;

	return amount;
}


//! changes position in file, returns true if successful
//! if relativeMovement==true, the pos is changed relative to current pos,
//! otherwise from begin of file
bool CMDHReadFile::seek(long finalPos, bool relativeMovement)
{
	if (relativeMovement)
	{
		if (Pos + finalPos > Len)
			return false;

		Pos += finalPos;
	}
	else
	{
		if (finalPos > Len)
			return false;
		
		Pos = finalPos;
	}

	return true;
}



//! returns size of file
long CMDHReadFile::getSize() const
{
	return Len;
}



//! returns where in the file we are.
long CMDHReadFile::getPos() const
{
	return Pos;
}



//! returns name of file
const irr::c8* CMDHReadFile::getFileName() const
{
	return Filename.c_str();
}


}
}

