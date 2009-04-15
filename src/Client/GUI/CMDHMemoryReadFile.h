// Copyright (C) 2002-2007 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_MDH_READ_FILE_H_INCLUDED__
#define __C_MDH_READ_FILE_H_INCLUDED__

#include "irrlicht/IReadFile.h"
#include "irrlicht/irrString.h"

namespace irr
{
namespace io
{

/*!
	Class for reading from memory.
*/
class CMDHReadFile : public IReadFile
{
public:

    CMDHReadFile(void* memory, long len, const irr::c8* fileName);

	virtual ~CMDHReadFile();

	//! returns how much was read
	virtual irr::s32 read(void* buffer, irr::u32 sizeToRead);

	//! changes position in file, returns true if successful
	//! if relativeMovement==true, the pos is changed relative to current pos,
	//! otherwise from begin of file
	virtual bool seek(long finalPos, bool relativeMovement = false);

	//! returns size of file
	virtual long getSize() const;

	//! returns where in the file we are.
	virtual long getPos() const;

	//! returns name of file
	virtual const c8* getFileName() const;

private:

	irr::core::stringc Filename;
	void *Buffer;
	long Len;
	long Pos;
};


}
}

#endif

