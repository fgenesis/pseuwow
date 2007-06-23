#ifndef STUFFEXTRACT_H
#define STUFFEXTRACT_H

#define _COMMON_SKIP_THREADS
#include "common.h"

#define SE_VERSION 2
#define MAPS_VERSION ((uint32)0)
#define OUTDIR "stuffextract"
#define SCPDIR OUTDIR "/data/scp"
#define MAPSDIR OUTDIR "/data/maps"

typedef std::map< uint32,std::list<std::string> > SCPStorageMap;

int main(int argc, char *argv[]);
void OutSCP(char*, SCPStorageMap&);
bool ConvertDBC(void);
void ExtractMaps(void);
void ExtractMapDependencies(void);

#endif