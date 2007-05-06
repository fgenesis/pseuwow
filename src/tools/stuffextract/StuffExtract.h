#ifndef STUFFEXTRACT_H
#define STUFFEXTRACT_H

#define _COMMON_SKIP_THREADS
#include "common.h"

#define SE_VERSION 1
#define MAPS_VERSION ((uint32)0)
#define OUTDIR "stuffextract"
#define SCPDIR OUTDIR "/data/scp"

typedef std::map< uint32,std::list<std::string> > SCPStorageMap;

int main(int argc, char *argv[]);
void OutSCP(char*, SCPStorageMap&);
bool ConvertDBC(void);


#endif