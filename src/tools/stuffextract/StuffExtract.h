#ifndef STUFFEXTRACT_H
#define STUFFEXTRACT_H

#define _COMMON_SKIP_THREADS
#include "common.h"

#define SE_VERSION 2
#define MAPS_VERSION ((uint32)0)
#define OUTDIR "stuffextract"
#define SCPDIR OUTDIR "/data/scp"
#define MAPSDIR OUTDIR "/data/maps"
#define SOUNDDIR OUTDIR "/data/sound"

typedef std::map< uint32,std::list<std::string> > SCPStorageMap;
typedef std::map<std::string,uint8*> MD5FileMap;

int main(int argc, char *argv[]);
void ProcessCmdArgs(int argc, char *argv[]);
void PrintConfig(void);
void PrintHelp(void);
void OutSCP(char*, SCPStorageMap&, std::string);
void OutMD5(char*, MD5FileMap&);
bool ConvertDBC(void);
void ExtractMaps(void);
void ExtractMapDependencies(void);
void ExtractSoundFiles(void);

#endif