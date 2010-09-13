#define _COMMON_NO_THREADS
#include "common.h"
#include <fstream>
#include "dbcfile.h"

DBCFile::DBCFile()
{
    data = NULL;
}

DBCFile::DBCFile(const std::string &fn)
{
	DBCFile();
	filename = fn;
}

bool DBCFile::open()
{
    std::fstream f;
    f.open(filename.c_str(), std::ios_base::binary | std::ios_base::in);
    if(!f.is_open())
    {
        printf("DBC: %s failed to open!\n",filename.c_str());
        f.close();
        return false;
    }
	char header[4];
	unsigned int na,nb,es,ss;

	f.read(header,4); // Number of records
	if(!(header[0]=='W' && header[1]=='D' && header[2]=='B' && header[3] == 'C'))
    {
        printf("DBC: %s is no DBC file!\n",filename.c_str());
        f.close();
        return false;
    }
	f.read((char*)&na,4); // Number of records
	f.read((char*)&nb,4); // Number of fields
	f.read((char*)&es,4); // Size of a record
	f.read((char*)&ss,4); // String size
	
	recordSize = es;
	recordCount = na;
	fieldCount = nb;
	stringSize = ss;
	if(fieldCount*4 != recordSize)
    {
        printf("DBC: %s is corrupt!\n",filename.c_str());
        f.close();
        return false;
    }

	data = new unsigned char[recordSize*recordCount+stringSize];
	stringTable = data + recordSize*recordCount;
	f.read((char*)data,recordSize*recordCount+stringSize);
	f.close();
    return true;
}

bool DBCFile::openmem(ByteBuffer bb)
{
	if(bb.size() < 4+4+4+4+4)
	{
		printf("DBCFile::openmem(): ByteBuffer too small!");
		return false;
	}
    uint32 hdr;
    bb >> hdr;
    
    if(memcmp(&hdr,"WDBC",4)) // check if its a valid dbc file
    {
        printf("not a valid WDB File??\n");
        return false;
    }

    bb >> recordCount >> fieldCount >> recordSize >> stringSize;

    if(fieldCount*4 != recordSize)
    {
       printf("DBCFile::openmem():Nonstandard record size\n");
       // return false;//records in CharBaseData are 2*1byte
    }

    data = new unsigned char[recordSize*recordCount+stringSize];
    stringTable = data + recordSize*recordCount;
    memcpy(data,bb.contents()+bb.rpos(),recordSize*recordCount+stringSize);
    return true;
}
DBCFile::~DBCFile()
{
	delete [] data;
}

DBCFile::Record DBCFile::getRecord(size_t id)
{
	assert(data);
	return Record(*this, data + id*recordSize);
}

DBCFile::Iterator DBCFile::begin()
{
	assert(data);
	return Iterator(*this, data);
}
DBCFile::Iterator DBCFile::end()
{
	assert(data);
	return Iterator(*this, stringTable);
}

