#ifndef STUFFEXTRACT_H
#define STUFFEXTRACT_H

#define _COMMON_SKIP_THREADS
#include "common.h"

#define SE_VERSION 2
#define MAPS_VERSION ((uint32)0)
#define OUTDIR "extractedstuff"
#define SCPDIR OUTDIR "/data/scp"
#define MAPSDIR OUTDIR "/data/maps"
#define SOUNDDIR OUTDIR "/data/sound"

typedef std::map< uint32,std::list<std::string> > SCPStorageMap;
typedef std::map<std::string,uint8*> MD5FileMap;

// this struct is used to resolve conflicting names when extracting archives.
// the problem is that some files stored in different folders in mpq archives will be extracted into one folder,
// overwriting each other.
// thus thats resolved by putting the mpq file name into `name` and the name it should be saved as in `alt`.
// leave `empty` to use filename specified in `name`
struct NameAndAlt
{
    NameAndAlt(std::string a) { name=a; }
    NameAndAlt(std::string a, std::string b) { name=a; alt=b; }
    bool operator<(NameAndAlt const & other) const { return name < other.name; } // required to be used in std::set
    std::string name;
    std::string alt;
};

int main(int argc, char *argv[]);
void ProcessCmdArgs(int argc, char *argv[]);
void PrintConfig(void);
void PrintHelp(void);
void OutSCP(const char*, SCPStorageMap&, std::string);
void OutMD5(const char*, MD5FileMap&);
bool ConvertDBC(void);
void ExtractMaps(void);
void ExtractMapDependencies(void);
void ExtractSoundFiles(void);

void FetchTexturesFromModel(ByteBuffer);

void WMO_Parse_Data(ByteBuffer, const char*, bool, bool, bool);

void ADT_ExportStringSetByOffset(const uint8*, uint32, std::set<NameAndAlt>&, const char*);
void ADT_FillTextureData(const uint8*,std::set<NameAndAlt>&);
void ADT_FillWMOData(const uint8*,std::set<NameAndAlt>&);
void ADT_FillModelData(const uint8*,std::set<NameAndAlt>&);

#endif
