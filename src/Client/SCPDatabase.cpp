#include <fstream>
#include "common.h"
#include "Auth/MD5Hash.h"
#include "SCPDatabase.h"

#define HEADER_SIZE (21*sizeof(uint32))

inline char *gettypename(uint32 ty)
{
    return (char*)(ty==0 ? "INT" : (ty==1 ? "FLOAT" : "STRING"));
}

// file-globally declared pointer holder. NOT multi-instance-safe for now!!
struct memblock
{
    memblock() : ptr(NULL), size(0) {}
    memblock(uint8 *p, uint32 s) : size(s), ptr(p) {}
    ~memblock() { if(ptr) delete [] ptr; }
    uint8 *ptr;
    uint32 size;
};
TypeStorage<memblock> Pointers; // stores filename -> file content
std::map<std::string,std::string> FileRelation; // stores filename -> DB name

SCPDatabase::~SCPDatabase()
{
    DEBUG(logdebug("Deleting SCPDatabase '%s'",_name.c_str()));
    DropAll();
}

SCPDatabase::SCPDatabase()
{
    _stringbuf = NULL;
    _intbuf = NULL;
    _compact = false;
}

void SCPDatabase::DropAll(void)
{
    DropTextData();
    if(_stringbuf)
        delete [] _stringbuf;
    if(_intbuf)
        delete [] _intbuf;
    _indexes.clear();
    _indexes_reverse.clear();
    _fielddefs.clear();
    _stringbuf = NULL;
    _intbuf = NULL;
    _stringsize = 0;
    _compact = false;
}

void SCPDatabase::DropTextData(void)
{
    DEBUG(logdebug("Dropping plaintext parts of DB '%s'",_name.c_str()));
    for(SCPSourceList::iterator it = sources.begin(); it != sources.end(); it++)
        Pointers.Delete(*it);
    sources.clear();
    fields.clear();
}

void *SCPDatabase::GetPtr(uint32 index, const char *entry)
{
    std::map<uint32,uint32>::iterator it = _indexes.find(index);
    if(it == _indexes.end())
        return NULL;
    std::map<std::string,SCPFieldDef>::iterator fi = _fielddefs.find(entry);
    if(fi == _fielddefs.end())
        return NULL;

    uint32 target_row = _indexes[index];
    uint32 field_id = _fielddefs[entry].id;
    return (void*)&_intbuf[(_fields_per_row * target_row) + field_id];
}

void *SCPDatabase::GetPtrByField(uint32 index, uint32 entry)
{
    std::map<uint32,uint32>::iterator it = _indexes.find(index);
    if(it == _indexes.end())
        return NULL;

    uint32 target_row = _indexes[index];
    return (void*)&_intbuf[(_fields_per_row * target_row) + entry];
}

uint32 SCPDatabase::GetFieldByUint32Value(const char *entry, uint32 val)
{
    std::map<std::string,SCPFieldDef>::iterator fi = _fielddefs.find(entry);
    if(fi == _fielddefs.end())
        return SCP_INVALID_INT;

    uint32 field_id = _fielddefs[entry].id;
    return GetFieldByUint32Value(field_id,val);
}

uint32 SCPDatabase::GetFieldByUint32Value(uint32 entry, uint32 val)
{
    for(uint32 row = 0; row < _rowcount; row++)
        if(_intbuf[row * _fields_per_row + entry] == val)
            return _indexes_reverse[row];
    return SCP_INVALID_INT;
}

uint32 SCPDatabase::GetFieldByIntValue(const char *entry, int32 val)
{
    std::map<std::string,SCPFieldDef>::iterator fi = _fielddefs.find(entry);
    if(fi == _fielddefs.end())
        return SCP_INVALID_INT;

    uint32 field_id = _fielddefs[entry].id;
    return GetFieldByIntValue(field_id,val);
}

uint32 SCPDatabase::GetFieldByIntValue(uint32 entry, int32 val)
{
    for(uint32 row = 0; row < _rowcount; row++)
        if((int)_intbuf[row * _fields_per_row + entry] == val)
            return _indexes_reverse[row];
    return (int)SCP_INVALID_INT;
}

uint32 SCPDatabase::GetFieldByStringValue(const char *entry, const char *val)
{
    std::map<std::string,SCPFieldDef>::iterator fi = _fielddefs.find(entry);
    if(fi == _fielddefs.end())
        return SCP_INVALID_INT;
    
    uint32 field_id = _fielddefs[entry].id;
    return GetFieldByStringValue(field_id,val);
}

uint32 SCPDatabase::GetFieldByStringValue(uint32 entry, const char *val)
{
    for(uint32 row = 0; row < _rowcount; row++)
        if(!stricmp(GetStringByOffset(_intbuf[row * _fields_per_row + entry]), val))
            return _indexes_reverse[row];
    return SCP_INVALID_INT;
}

uint32 SCPDatabase::GetFieldType(const char *entry)
{
    std::map<std::string,SCPFieldDef>::iterator it = _fielddefs.find(entry);
    if(it != _fielddefs.end())
        return it->second.type;
    return SCP_INVALID_INT;
}

uint32 SCPDatabase::GetFieldId(const char *entry)
{
    std::map<std::string,SCPFieldDef>::iterator it = _fielddefs.find(entry);
    if(it != _fielddefs.end())
        return it->second.id;
    return SCP_INVALID_INT;
}

SCPDatabase *SCPDatabaseMgr::GetDB(std::string n, bool create)
{
    return create ? _map.Get(n) : _map.GetNoCreate(n);
}

uint32 SCPDatabaseMgr::AutoLoadFile(const char *fn)
{
    char *buf;
    uint32 size;

    // check if file was loaded before; use memory data if this is the case
    if(memblock *mb = Pointers.GetNoCreate(fn))
    {
        size = mb->size;
        buf = (char*)mb->ptr;
    }
    else // and if not, read the file from disk
    {
        std::fstream fh;
        size = GetFileSize(fn);
        if(!size)
            return 0;

        fh.open(fn,std::ios_base::in | std::ios_base::binary);
        if( !fh.is_open() )
            return 0;

        buf = new char[size];

        fh.read(buf,size);
        fh.close();

        // store the loaded file buffer so we can reuse it later if necessary
        Pointers.Assign(fn, new memblock((uint8*)buf,size));
    }

    std::string line,dbname,entry,value;
    SCPDatabase *db = NULL;
    uint32 id = 0, sections = 0;
    for(uint32 pos = 0; pos < size; pos++)
    {
        if(buf[pos] == '\n' || buf[pos] == 10 || buf[pos] == 13)
        {
            if(line.empty())
                continue;
            while(line.size() && (line[0]==' ' || line[0]=='\t'))
                line.erase(0,1);
            if(line.size() < 2 || (line[0] == '/' && line[1] == '/'))
            {
                line.clear();
                continue;
            }
            size_t eq = line.find("=");
            if(eq != std::string::npos)
            {
                entry=stringToLower(line.substr(0,eq));
                value=line.substr(eq+1,line.length()-1);

                if(!stricmp(entry.c_str(),"#dbname") && value.size())
                {
                    dbname = value;
                    db = GetDB(dbname,true); // create db if not existing
                    FileRelation[fn] = dbname;
                }
                else if(db)
                        db->fields[id][entry] = value;
            }
            else if(line[0]=='[')
            {
                id=(uint32)toInt(line.c_str()+1); // start reading after '['
                sections++;
            }
            line.clear();
        }
        else
            line += buf[pos];
    }
    db->sources.insert(fn);
    return sections;
}

////////////////////////////////////////////////////////
// SCP compiler
////////////////////////////////////////////////////////


// check the datatype that will be used for this string value
uint32 SCPDatabaseMgr::GetDataTypeFromString(const char *s)
{
    bool isint = true, first = true;
    for(;*s;s++) // check every char until \0 is reached
    {
        if(!isdigit(*s) && !(first && (*s=='+' || *s=='-'))) // if not in 0-9 or first beeing "+" or "-" it cant be int...
        {
            isint = false;
            if(*s != '.') // and if the char beeing not int isnt a dot (3.1415), it cant be float either
                return SCP_TYPE_STRING;
        }
        first = false;
    }
    return isint ? SCP_TYPE_INT : SCP_TYPE_FLOAT;
}

bool SCPDatabaseMgr::Compact(const char *dbname, const char *outfile, uint32 compression)
{
    logdebug("Compacting database '%s' into file '%s'", dbname, outfile);
    SCPDatabase *db = GetDB(dbname);
    if(!db || db->fields.empty() || db->sources.empty())
    {
        logerror("Compact(\"%s\",\"%s\") failed, DB doesn't exist or is empty",dbname,outfile);
        return false;
    }
    std::map<std::string, SCPFieldDef> fieldIdMap;
    std::map<uint32,uint32> idToSectionMap;
    ByteBuffer stringdata(5000);
    uint32 cur_idx, pass = 0;
    std::string line;
    uint32 section=0;
    uint32 *membuf = NULL; // stores ints, floats and string offsets from ByteBuffer stringdata
    uint32 blocksize;
    std::string entry,value;
    uint32 field_id;

    // some preparations
    uint32 nStrings = 0, nRows = db->fields.size(), nFields; // pre-declared
    stringdata << ""; // write an empty string so that offset 0 will always return empty string

    // the whole process is divided into 2 passes:
    // - pass 0 autodetects the datatypes stored in the different entries (SCPFieldDef)
    // - pass 1 creates the data field holding the values and string offsets, and also stuffs string values into the ByteBuffer
    while(pass < 2)
    {
        cur_idx = 1; // stores the current index of the last entry added. index 0 is always field_id!
        section = 0; // stores how many sections are done already
        for(SCPFieldMap::iterator fmit = db->fields.begin(); fmit != db->fields.end(); fmit++)
        {
            SCPEntryMap& emap = fmit->second;
            field_id = fmit->first; // stores the field id (e.g. "[154]" in the scp file)
            idToSectionMap[field_id] = section; // since not every field id exists, store field id and its section number
                                                // for faster lookup, less overhead, and smaller files

            // write the field id into row position 0 of the data field
            if(pass == 1)
            {
                ASSERT(membuf != NULL);
                uint32 pos = section * nFields;
                ASSERT(pos < blocksize);
                membuf[pos] = field_id;
            }

            for(SCPEntryMap::iterator eit = emap.begin(); eit != emap.end(); eit++)
            {
                // "entry=value" in the scp file
                entry = eit->first;
                value = eit->second;

                 // pass 0 - autodetect field types
                if(pass == 0)
                {
                    if(fieldIdMap.find(entry) == fieldIdMap.end())
                    {
                        SCPFieldDef d;
                        d.id = cur_idx++;
                        d.type = GetDataTypeFromString((char*)value.c_str());
                        fieldIdMap[entry] = d;
                        DEBUG(logdebug("Found new key: '%s' id: %u type: %s", entry.c_str(), d.id, gettypename(d.type) ));
                    }
                    else
                    {
                        SCPFieldDef& d = fieldIdMap[entry];
                        uint32 _oldtype = d.type;
                        // int can be changed to float or string, float only to string. changing back not allowed.
                        if(d.type == SCP_TYPE_INT)
                        {
                            d.type = GetDataTypeFromString((char*)value.c_str());
                        }
                        else if(d.type == SCP_TYPE_FLOAT)
                        {
                            uint32 newtype = GetDataTypeFromString((char*)value.c_str());
                            if(newtype == SCP_TYPE_STRING)
                                d.type = newtype;
                        }

                        if(_oldtype != d.type)
                        {
                            logdebug("Key '%s' id %u changed from %s to %s (field_id: %u)", entry.c_str(), d.id, gettypename(_oldtype), gettypename(d.type),field_id);
                        }
                    }
                }
                // pass 1 - create the data field
                else if(pass == 1)
                {
                    ASSERT(membuf != NULL);
                    SCPFieldDef d = fieldIdMap[entry];
                    uint32 pos = (section * nFields) + d.id;
                    ASSERT(pos < blocksize);
                    if(d.type == SCP_TYPE_STRING)
                    {
                        membuf[pos] = stringdata.wpos();
                        stringdata << value;
                        nStrings++;
                    }
                    else if(d.type == SCP_TYPE_INT)
                    {
                        membuf[pos] = atoi(value.c_str());
                    }
                    else if(d.type == SCP_TYPE_FLOAT)
                    {
                        ((float*)membuf)[pos] = atof(value.c_str());
                    }
                }
            }
            section++;
        }
        // if the first pass is done, allocate the data field for the second pass
        if(!pass)
        {
            nFields = fieldIdMap.size() + 1; // add the one field used for the field ID
            ASSERT(section == nRows);
            blocksize = nRows * nFields; // +1 because we store the field id here also
            DEBUG(logdebug("SCP: allocating %u*%u = %u integers (%u bytes)",nRows,fieldIdMap.size()+1,blocksize, blocksize*sizeof(uint32)));
            membuf = new uint32[blocksize];
            memset(membuf, 0, blocksize * sizeof(uint32));
        }
        pass++;
    }

    // used header fields, some are pre-declared above, if needed
    uint32 offsMD5, nMD5, sizeMD5;
    uint32 offsIndexes, nIndexes, sizeIndexes;
    uint32 offsFields, sizeFields;
    uint32 offsData, sizeData;
    uint32 offsStrings, sizeStrings;

    // MD5 hashes of source files
    SCPSourceList& src = db->sources;
    ByteBuffer md5buf;
    nMD5 = 0;
    for(SCPSourceList::iterator it = src.begin(); it != src.end(); it++)
    {
        memblock *mb = Pointers.GetNoCreate(*it);
        if(!mb)
        {
            // if we reach this point there was really some big f*** up
            logerror("SCP Compact: memblock for file '%s' doesn't exist",it->c_str());
            continue;
        }

        MD5Hash md5;
        md5.Update(mb->ptr,mb->size);
        md5.Finalize();
        md5buf << *it;
        md5buf.append(md5.GetDigest(),md5.GetLength());
        nMD5++;
    }
    sizeMD5 = md5buf.size();

    // field types, sorted by IDs
    ByteBuffer fieldbuf;
    // put the entries and their type into ByteBuffer, sorted by their position in the membuf rows
    // note that the first field in the data row is always the field id, so the values start from 1
    for(std::map<std::string,SCPFieldDef>::iterator itf = fieldIdMap.begin(); itf != fieldIdMap.end(); itf++)
    {
        fieldbuf << itf->first << itf->second.id << itf->second.type; // entry name, id, type.
    }
    sizeFields = fieldbuf.size();

    // index -> ID lookup table, e.g. data with ID 500 will have field index 214, because some IDs in between are missing
    // it *could* be calculated at load-time from the existing data field, but this way is faster when random-accessing the file itself
    // (what we dont do anyway, for now)
    ByteBuffer indexbuf;
    for(std::map<uint32,uint32>::iterator itx = idToSectionMap.begin(); itx != idToSectionMap.end(); itx++)
    {
        indexbuf << itx->first << itx->second; // field id; row number
    }
    nIndexes = idToSectionMap.size();
    sizeIndexes = indexbuf.size();

    // string data
    // -- most of it is handled somewhere above
    sizeStrings = stringdata.size();

    // calc data field size in bytes
    sizeData = blocksize * sizeof(uint32);

    // precalc relative offsets, after header (min 21 bytes, maybe some extra bytes)
    offsMD5 = 0; // directly after header
    offsIndexes = offsMD5 + sizeMD5;
    offsFields = offsIndexes + sizeIndexes;
    offsData = offsFields + sizeFields;
    offsStrings = offsData + sizeData;

    // buffer the file header
    ByteBuffer hbuf(HEADER_SIZE);
    hbuf.append("SCPC",4); // identifier

    uint32 flags = 0;

    hbuf << flags; // flags, placeholder; real value is put below
    hbuf << (uint32)0 << (uint32)0 << (uint32)0 << (uint32)0; // padding, not yet used
    hbuf << offsMD5 << nMD5 << sizeMD5;
    hbuf << offsIndexes << nIndexes << sizeIndexes;
    hbuf << offsFields << nFields << sizeFields;
    hbuf << offsData << section << sizeData;
    hbuf << offsStrings << nStrings << sizeStrings;

    ZCompressor z;
    z.reserve(sizeMD5 + sizeIndexes + sizeFields + sizeData + sizeStrings);
    z.append(md5buf);
    z.append(indexbuf);
    z.append(fieldbuf);
    z.append((uint8*)membuf, sizeData);
    z.append(stringdata);

    if(compression)
    {
        z.Deflate(compression);
        if(z.Compressed())
        {
            hbuf << z.RealSize();
            flags |= SCP_FLAG_COMPRESSED;
        }
        else
        {
            logdebug("SCP Compact: Unable to compress '%s' (too small?)",outfile);
        }
    }

    hbuf.put<uint32>(4,flags); // first 4 bytes are 'SCPC', then flags...

    FILE *fh = fopen(outfile,"wb");
    if(!fh)
        return false;

    fwrite(hbuf.contents(), hbuf.size(), 1, fh);
    fwrite(z.contents(), z.size(), 1, fh);
    fclose(fh);

    if(!db)
        db = GetDB(dbname,true); // create if not exist

    db->_compact = true;
    db->_name = dbname;

    // drop all data no longer needed if the database is compacted
    db->DropTextData();

    // we keep the membuf, since the compiled data are now usable as if loaded directly from a file
    // associate it with the buffers used by the db accessing functions
    db->_stringbuf = new char[sizeStrings];
    db->_stringsize = sizeStrings;
    memcpy(db->_stringbuf,stringdata.contents(),sizeStrings);
    db->_intbuf = membuf; // <<-- do NOT drop the membuf, its still used and will be deleted with ~SCPDatabase()!!
    db->_fields_per_row = nFields;
    db->_rowcount = nRows;
    db->_indexes = idToSectionMap;
    db->_fielddefs = fieldIdMap;
    for(std::map<uint32,uint32>::iterator it = idToSectionMap.begin(); it != idToSectionMap.end(); it++)
        db->_indexes_reverse[it->second] = it->first;

    return true;
}

void SCPDatabaseMgr::_FilterFiles(std::deque<std::string>& files, std::string dbname)
{
    for(std::deque<std::string>::iterator it = files.begin(); it != files.end(); )
    {
        std::map<std::string,std::string>::iterator w;
        bool load_it = false;
        // first check if the file was already loaded once, in this case use cached data
        if( (w = FileRelation.find(*it)) != FileRelation.end() )
        {
            if(w->second == dbname)
                load_it = true;
        }
        else // if not previously loaded, load now and cache
        {
            std::fstream fh;
            fh.open( it->c_str() , std::ios_base::in | std::ios_base::binary);
            if( !fh.is_open() )
            {
                logerror("SCP: Can't open file '%s'", it->c_str());
                continue;
            }

            uint32 size = 1000; // search for #dbname tag in first 1000 bytes
            char *buf = new char[size];
            memset(buf,0,size);

            fh.read(buf,size);
            fh.close();

            std::string line,dbn;
            for(uint32 pos = 0; pos < size; pos++)
            {
                if(buf[pos] == '\n' || buf[pos] == 10 || buf[pos] == 13)
                {
                    if(line.empty())
                        continue;
                    while(line.size() && (line[0]==' ' || line[0]=='\t'))
                        line.erase(0,1);
                    if(line[0] == '#')
                    {
                        if(!strnicmp(line.c_str(),"#dbname=",8))
                        {
                            std::string t = line.c_str() + 8; // current db name
                            FileRelation[*it] = t;
                            if(!stricmp(t.c_str(), dbname.c_str()))
                            {
                                load_it = true;
                                break;
                            }
                        }
                    }
                    line.clear();
                }
                else
                    line += buf[pos];
            }
            delete [] buf;
        }

        if(load_it)
            it++;
        else
        {
            //DEBUG(logdebug("SCP: '%s' not used for [%s]", it->c_str(), dbname.c_str()));
            it = files.erase(it);
        }
    }
    DEBUG(logdebug("-> %u files belong to this DB",files.size()));
}

uint32 SCPDatabaseMgr::SearchAndLoad(const char *dbname, bool no_compiled)
{
    uint32 count = 0;
    std::deque<std::string> goodfiles;
    std::string ccpFile;

    for(std::deque<std::string>::iterator it = _paths.begin(); it != _paths.end(); it++)
    {
        std::deque<std::string> files = GetFileList(*it);
        sort(files.begin(),files.end()); // rough alphabetical sort
        for(std::deque<std::string>::iterator itf = files.begin(); itf != files.end(); itf++)
        {
            std::string& fn = *itf;
            if(fn.length() < 5)
                continue;
            std::string filepath = *it + fn;
            // check for special case: <dbname>.ccp in this directory? load it!
            // others must be checked only for MD5-match and if new files are there not yet recorded in MD5
            if(!no_compiled && !stricmp(std::string(dbname).append(".ccp").c_str(), fn.c_str()))
            {
                ccpFile = filepath;
            }
            else if(!stricmp(fn.c_str() + fn.length() - 4, ".scp"))
            {
                // skip 0-byte files
                if(GetFileSize(filepath.c_str()))
                    goodfiles.push_back(filepath);
                else
                    FileRelation[filepath] = ""; // empty files cant belong to a DB
            }
        }
    }

    // goodfiles stores a list of all scp files found, we need to remove those that are not required for this DB
    _FilterFiles(goodfiles,dbname);

    if(!goodfiles.size())
    {
        logerror("SCP: No files found that contain database [%s]", dbname);
        return 0;
    }

    // string only exists if CCP file was found and if it should no be skipped
    if(ccpFile.size())
    {
        logdebug("Loading pre-compacted database '%s'", ccpFile.c_str());
        DropDB(dbname); // if sth got loaded before, remove that
        // load SCC database file
        if(LoadCompactSCP((char*)ccpFile.c_str(), dbname, goodfiles.size()))
        {
            logdebug("Loaded '%s' -> %s",ccpFile.c_str(),dbname);
            return goodfiles.size();
        }
        else
        {
            logdetail("Pre-compacted SCC file for '%s' outdated, creating from SCP (%u files total)",dbname,goodfiles.size());
        }
    }

    for(std::deque<std::string>::iterator it = goodfiles.begin(); it != goodfiles.end(); it++)
    {
        logdebug("File '%s' matching database '%s', loading", it->c_str(), dbname);
        count++;
        uint32 sections = AutoLoadFile((char*)it->c_str());
        logdebug("%u sections loaded", sections);
    }

    char fn[100];
    sprintf(fn,"./cache/%s.ccp",dbname);
    if (!Compact(dbname, fn, _compr))
    {
        logerror("Can't compact database %s, dropping it.", dbname);
        DropDB(dbname);
        return 0;
    }

    logdetail("Database '%s' loaded from source and compacted with compression %u", dbname, _compr);

    return count;
}

void SCPDatabaseMgr::AddSearchPath(const char *path)
{
    std::string p;

    // normalize path, use '/' instead of '\'. needed for that check below
    uint32 len = strlen(path);
    for(uint32 i = 0; i < len; i++)
    {
        if(path[i] == '\\')
            p += '/';
        else
            p += path[i];
    }

    if(p[p.size()-1] != '/')
        p += '/';
    for(std::deque<std::string>::iterator it = _paths.begin(); it != _paths.end(); it++)
    {
        // windows doesnt care about UPPER/lowercase, while *nix does;
        // a path shouldnt be added twice or unexpected behavior is likely to occur
#if PLATFORM == PLATFORM_WIN32
        if(!stricmp(p.c_str(), it->c_str()))
            return;
#else
        if(p == *it)
            return;
#endif
    }
        
    _paths.push_back(p);
}

bool SCPDatabaseMgr::LoadCompactSCP(const char *fn, const char *dbname, uint32 nSourcefiles)
{
    uint32 filesize = GetFileSize(fn);
    if(filesize < HEADER_SIZE)
    {
        logerror("Database file '%s' is too small!",fn);
        return false;
    }
    std::fstream fh;
    fh.open(fn, std::ios_base::in | std::ios_base::binary);
    if(!fh.is_open())
    {
        logerror("Error opening '%s'",fn);
        return false;
    }

    ByteBuffer hbuf;
    hbuf.resize(HEADER_SIZE);
    fh.read((char*)hbuf.contents(), HEADER_SIZE);

    char tag[4];
    uint32 flags, padding[4];
    uint32 offsMD5, nMD5, sizeMD5;
    uint32 offsIndexes, nIndexes, sizeIndexes;
    uint32 offsFields, nFields, sizeFields;
    uint32 offsData, nRows, sizeData;
    uint32 offsStrings, nStrings, sizeStrings;

    uint32 realsize; // used when compressed

    hbuf.read((uint8*)&tag[0],4);
    if(memcmp(tag,"SCPC",4))
    {
        logerror("'%s' is not a compact database file!",fn);
        return false;
    }
    hbuf >> flags;
    hbuf >> padding[0] >> padding[1] >> padding[2] >> padding[3];
    hbuf >> offsMD5 >> nMD5 >> sizeMD5;
    hbuf >> offsIndexes >> nIndexes >> sizeIndexes;
    hbuf >> offsFields >> nFields >> sizeFields;
    hbuf >> offsData >> nRows >> sizeData;
    hbuf >> offsStrings >> nStrings >> sizeStrings;

    // read some extra bytes depending on flags
    if(flags & SCP_FLAG_COMPRESSED)
        fh.read((char*)&realsize, sizeof(uint32));

    ZCompressor z;
    uint32 remain = sizeMD5 + sizeIndexes + sizeFields + sizeData + sizeStrings;
    z.resize(remain);
    fh.read((char*)z.contents(), remain);

    fh.close(); // All data are read from the file now, it can safely be closed

    if(flags & SCP_FLAG_COMPRESSED)
    {
        z.Compressed(true);
        z.RealSize(realsize);
        z.Inflate();
        if(z.Compressed())
        {
            logerror("LoadCompactSCP: Unable to uncompress '%s'",fn);
            return false;
        }
    }

    SCPDatabase *db = GetDB(dbname,true);
    db->_name = dbname;
    db->_compact = true;

    ByteBuffer md5buf(sizeMD5);
    md5buf.resize(sizeMD5);
    // read MD5 block
    z.rpos(offsMD5);
    if(z.rpos() == offsMD5)
    {
        z.read((uint8*)md5buf.contents(),sizeMD5);
    }
    else
    {
        logerror("'%s' has wrong MD5 offset, can't load",fn);
        return false;
    }

    for(uint32 i = 0; i < nMD5; i++)
    {
        // read filename and MD5 hash from compiled database
        uint8 buf[MD5_DIGEST_LENGTH];
        std::string refFn;
        md5buf >> refFn;
        md5buf.read(buf,MD5_DIGEST_LENGTH);

        // load the file referred to
        uint32 refFileSize = GetFileSize(refFn.c_str());
        FILE *refFile = fopen(refFn.c_str(), "rb");
        if(!refFile)
        {
            logdebug("Not loading '%s', file doesn't exist",fn);
            return false;
        }
        uint8 *refFileBuf = new uint8[refFileSize];
        fread(refFileBuf,sizeof(uint8),refFileSize,refFile);
        fclose(refFile);
        db->sources.insert(refFn);
        Pointers.Assign(refFn, new memblock(refFileBuf,refFileSize));
        MD5Hash md5;
        md5.Update(refFileBuf,refFileSize);
        md5.Finalize();
        if(memcmp(buf, md5.GetDigest(), MD5_DIGEST_LENGTH))
        {
            logdebug("MD5-check: '%s' has changed!", refFn.c_str());
            return false;
        }
        else
        {
            logdebug("MD5-check: '%s' -> OK",refFn.c_str());
        }
    }

    // check if there are any new files matching this database, that are not yet compacted and hashed.
    // if the size differs now, and no changes were detected so far, there are probably new files added
    if(nSourcefiles > nMD5)
    {
        logdebug("There are more source files existing then hashed in the CCP file, must recompact.");
        return false;
    }
    ASSERT(nMD5 == nSourcefiles); // if we didnt return until now, something isnt good

    // everything good so far? we reached this point? then its likely that the rest of the file is ok, alloc remaining buffers
    ByteBuffer indexbuf(sizeIndexes);
    ByteBuffer fieldsbuf(sizeFields);
    indexbuf.resize(sizeIndexes);
    fieldsbuf.resize(sizeFields);

    // read indexes block
    z.rpos(offsIndexes);
    if(z.rpos() == offsIndexes)
        z.read((uint8*)indexbuf.contents(),sizeIndexes);
    else
    {
        logerror("'%s' has wrong indexes offset, can't load",fn);
        return false;
    }

    // read field definitions buf
    z.rpos(offsFields);
    if(z.rpos() == offsFields)
        z.read((uint8*)fieldsbuf.contents(),sizeFields);
    else
    {
        logerror("'%s' has wrong field defs offset, can't load",fn);
        return false;
    }

    // main data and string blocks follow below

    for(uint32 i = 0; i < nIndexes; i++)
    {
        uint32 field_id, row;
        indexbuf >> field_id >> row;
        db->_indexes[field_id] = row;
        db->_indexes_reverse[row] = field_id;
    }

    for(uint32 i = 0; i < nFields - 1; i++) // the first field (index column) is never written to the file!
    {
        SCPFieldDef fieldd;
        std::string fieldn;
        fieldsbuf >> fieldn >> fieldd.id >> fieldd.type;
        db->_fielddefs[fieldn] = fieldd;
    }

    // read main data block
    ASSERT(nRows * nFields == sizeData / sizeof(uint32));
    z.rpos(offsData);
    if(z.rpos() == offsData)
    {
        db->_intbuf = new uint32[nRows * nFields];
        z.read((uint8*)db->_intbuf, sizeData); // load this somewhat fast and without a for loop
    }
    else
    {
        logerror("'%s' has wrong data offset, can't load",fn);
        return false;
    }

    // read strings
    z.rpos(offsStrings);
    if(z.rpos() == offsStrings)
    {
        db->_stringbuf = new char[sizeStrings];
        z.read((uint8*)db->_stringbuf,sizeStrings);
    }
    else
    {
        logerror("'%s' has wrong strings offset, can't load",fn);
        return false;
    }
    db->_stringsize = sizeStrings;
    db->_rowcount = nRows;
    db->_fields_per_row = nFields;

    db->DropTextData(); // delete pointers to file content created at md5 comparison

    // all fine, DB loaded

    return true;
}

// used only for debugging
void SCPDatabase::DumpStructureToFile(const char *fn)
{
    std::ofstream f;
    f.open(fn);
    if(!f.is_open())
        return;

    uint32 *ftype = new uint32[_fields_per_row];
    ftype[0] = SCP_TYPE_INT;

    f << "Fields: (0 is always index field)\n";
    for(std::map<std::string,SCPFieldDef>::iterator it = _fielddefs.begin(); it != _fielddefs.end(); it++)
    {
        f << "-> Name: " << it->first << ", ID: " << it->second.id << ", type: " << gettypename(it->second.type) << "\n";
        ftype[it->second.id] = it->second.type;
    }
    f << "\n";

    for(uint32 row = 0; row < _rowcount; row++)
    {
        for(uint32 column = 0; column < _fields_per_row; column++)
        {
            if(ftype[column] == SCP_TYPE_INT)
                f << *((int*)&_intbuf[row * _fields_per_row + column]) << "\t";
            else if(ftype[column] == SCP_TYPE_FLOAT)
                f << *((float*)&_intbuf[row * _fields_per_row + column]) << "\t";
            else
                f << "S_" << _intbuf[row * _fields_per_row + column] << "\t";
        }
        f << "\n";
    }

    f << "\nStrings:\n";
    for(uint32 i = 0; i < _stringsize; i++)
    {
        if(!_stringbuf[i])
        {
            i++;
            if(i >= _stringsize)
                break;
            f << "\n" << i << ": ";
        }
        f << _stringbuf[i];
    }
}





