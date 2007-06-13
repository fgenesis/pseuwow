#include <fstream>
#include <set>
#define _COMMON_NO_THREADS
#include "common.h"
#include "MPQHelper.h"
#include "dbcfile.h"
#include "ADTFile.h"
#include "StuffExtract.h"
#include "DBCFieldData.h"
#include "Locale.h"

std::vector<std::string> mapNames;
std::set<std::string> texNames;
std::set<std::string> modelNames;
std::set<std::string> wmoNames;


int main(int argc, char *argv[])
{
    char input[200];
    printf("StuffExtract [version %u]\n\n",SE_VERSION);
	printf("Enter your locale (enUS, enGB, deDE, ...) or leave blank to autodetect: ");
	fgets(input,sizeof(input),stdin);
	char loc[5];
    input[strlen(input)-1] = 0;
	memcpy(loc,input,4); loc[4]=0;	
	SetLocale(loc);
	if(FileExists(std::string("Data/")+GetLocale()+"/locale-"+GetLocale()+".MPQ"))
	{
		printf("Locale seems valid, starting conversion...\n");
        CreateDir("stuffextract");
        CreateDir("stuffextract/data");
		ConvertDBC();
        ExtractMaps();
        ExtractMapDependencies();
		//...
		printf("\n -- finished, press enter to exit --\n");
	}
	else
	{
		printf("ERROR: Invalid locale! Press Enter to exit...\n");
	}

    fgets(input,sizeof(input),stdin);

    //while(true);
    return 0;
}

// be careful using this, that you supply correct format string
std::string AutoGetDataString(DBCFile::Iterator& it, const char* format, uint32 field)
{
    if(format[field]=='i')
    {
        std::stringstream s;
        s << (*it).getInt(field);
        return s.str();
    }
    else if(format[field]=='f')
    {
        std::stringstream s;
        s << (*it).getFloat(field);
        return s.str();
    }
    else if(format[field]=='s' && (*it).getUInt(field))
    {
        return (*it).getString(field);
    }

    return "";
}


// output a formatted scp file
void OutSCP(char *fn, SCPStorageMap& scp)
{
    std::fstream f;
    f.open(fn, std::ios_base::out);
    if(f.is_open())
    {
        for(SCPStorageMap::iterator mi = scp.begin(); mi != scp.end(); mi++)
        {
            f << "[" << mi->first << "]\n";
            for(std::list<std::string>::iterator li = mi->second.begin(); li != mi->second.end(); li++)
            {
                f << *li << "\n";
            }
            f << "\n";
        }
        f.close();
    }
    else
    {
        printf("OutSCP: unable to write '%s'\n",fn);
    }
}

bool ConvertDBC(void)
{
    std::map<uint8,std::string> racemap; // needed to extract other dbc files correctly
    SCPStorageMap EmoteDataStorage,RaceDataStorage,SoundDataStorage,MapDataStorage,AreaDataStorage; // will store the converted data from dbc files
    DBCFile EmotesText,EmotesTextData,EmotesTextSound,ChrRaces,SoundEntries,Map,AreaTable;
    printf("Opening DBC archive...\n");
    MPQHelper mpq("dbc");

    printf("Opening DBC files...\n");
    EmotesText.openmem(mpq.ExtractFile("DBFilesClient\\EmotesText.dbc"));
    EmotesTextData.openmem(mpq.ExtractFile("DBFilesClient\\EmotesTextData.dbc"));
    EmotesTextSound.openmem(mpq.ExtractFile("DBFilesClient\\EmotesTextSound.dbc"));
    ChrRaces.openmem(mpq.ExtractFile("DBFilesClient\\ChrRaces.dbc"));
    SoundEntries.openmem(mpq.ExtractFile("DBFilesClient\\SoundEntries.dbc"));
    Map.openmem(mpq.ExtractFile("DBFilesClient\\Map.dbc"));
    AreaTable.openmem(mpq.ExtractFile("DBFilesClient\\AreaTable.dbc"));
    //...
    printf("DBC files opened.\n");
    //...
    printf("Reading data: races..");
    for(DBCFile::Iterator it = ChrRaces.begin(); it != ChrRaces.end(); ++it)
    {
        uint32 id = (*it).getUInt(CHRRACES_RACEID);
        racemap[id] = (*it).getString(CHRRACES_NAME_GENERAL); // for later use
        for(uint32 field=CHRRACES_RACEID; field < CHARRACES_END; field++)
        {
            if(strlen(ChrRacesFieldNames[field]))
            {
                std::string value = AutoGetDataString(it,ChrRacesFormat,field);
				if(!value.empty())
					RaceDataStorage[id].push_back(std::string(ChrRacesFieldNames[field]).append("=").append(value));
            }
        }
    }
    
    printf("emotes..");
    for(DBCFile::Iterator it = EmotesText.begin(); it != EmotesText.end(); ++it)
    {
        uint32 em = (*it).getUInt(EMOTESTEXT_EMOTE_ID);
        EmoteDataStorage[em].push_back(std::string("name=") + (*it).getString(EMOTESTEXT_EMOTE_STRING));
        EmoteDataStorage[em].push_back(std::string("anim=") + toString( (*it).getUInt(EMOTESTEXT_ANIM)) );
        for(uint32 field=EMOTESTEXT_EMOTE_ID; field<EMOTESTEXT_END;field++)
        {
            if((*it).getInt(field) && strlen(EmotesTextFieldNames[field]))
            {
                uint32 textid;
                std::string fname;
                for(DBCFile::Iterator ix = EmotesTextData.begin(); ix != EmotesTextData.end(); ++ix)
                {
                    textid = (*ix).getUInt(EMOTESTEXTDATA_TEXTID);
                    if(textid == (*it).getInt(field))
                    {
                        fname = EmotesTextFieldNames[field];
						for(uint8 stringpos=EMOTESTEXTDATA_STRING1; stringpos<=EMOTESTEXTDATA_STRING8; stringpos++) // we have 8 locales, so...
						{
							if((*ix).getInt(stringpos)) // find out which field is used, 0 if not used
							{
                                std::string tx = (*ix).getString(stringpos);
                                uint32 fpos=0;
                                // the following block replaces %x$s (where x is a number) with %s
                                do 
                                {
                                    if(fpos+4 < tx.length() && tx[fpos]=='%' && tx[fpos+2]=='$' && tx[fpos+3]=='s' && isdigit(tx[fpos+1]))
                                    {
                                        tx.erase(fpos,4);
                                        tx.insert(fpos,"%s");
                                    }
                                    fpos++;
                                } while(fpos < tx.length());
								EmoteDataStorage[em].push_back( fname + "=" + tx );
								break;
							}
						}
						break;
                    }
                }
            }
        }
        for(DBCFile::Iterator is = EmotesTextSound.begin(); is != EmotesTextSound.end(); ++is)
        {
            if(em == (*is).getUInt(EMOTESTEXTSOUND_EMOTEID))
            {
                std::string record = "Sound";
                record += racemap[ (*is).getUInt(EMOTESTEXTSOUND_RACE) ];
                record += ((*is).getUInt(EMOTESTEXTSOUND_ISFEMALE) ? "Female" : "Male");
                record += "=";
                record += toString( (*is).getUInt(EMOTESTEXTSOUND_SOUNDID) );
                EmoteDataStorage[em].push_back(record);
            }
        }
    }

    printf("sound entries..");
    for(DBCFile::Iterator it = SoundEntries.begin(); it != SoundEntries.end(); ++it)
    {
        uint32 id = (*it).getUInt(SOUNDENTRY_SOUNDID);
        for(uint32 field=SOUNDENTRY_SOUNDID; field < SOUNDENTRY_END; field++)
        {
            if(strlen(SoundEntriesFieldNames[field]))
            {
                std::string value = AutoGetDataString(it,SoundEntriesFormat,field);
                if(value.size()) // only store if a file exists in that field
                    SoundDataStorage[id].push_back(std::string(SoundEntriesFieldNames[field]) + "=" + value);
            }
        }
    }

    printf("map info..");
    for(DBCFile::Iterator it = Map.begin(); it != Map.end(); ++it)
    {
        mapNames.push_back(it->getString(MAP_NAME_GENERAL));
        uint32 id = it->getUInt(MAP_ID);
        for(uint32 field=MAP_ID; field < MAP_END; field++)
        {
            if(strlen(MapFieldNames[field]))
            {
                std::string value = AutoGetDataString(it,MapFormat,field);
                if(value.size()) // only store if not null
                    MapDataStorage[id].push_back(std::string(MapFieldNames[field]) + "=" + value);
            }
        }
    }

    printf("area..");
    for(DBCFile::Iterator it = AreaTable.begin(); it != AreaTable.end(); ++it)
    {
        uint32 id = it->getUInt(MAP_ID);
        for(uint32 field=AREATABLE_ID; field < AREATABLE_END; field++)
        {
            if(strlen(AreaTableFieldNames[field]))
            {
                std::string value = AutoGetDataString(it,AreaTableFormat,field);
                if(value.size()) // only store if not null
                    AreaDataStorage[id].push_back(std::string(AreaTableFieldNames[field]) + "=" + value);
            }
        }
    }



    //...
    printf("DONE!\n");
    //...

    CreateDir("stuffextract/data/scp");

    printf("Writing SCP files:\n");    
    printf("emote.."); OutSCP(SCPDIR "/emote.scp",EmoteDataStorage);
    printf("race.."); OutSCP(SCPDIR "/race.scp",RaceDataStorage);
    printf("sound.."); OutSCP(SCPDIR "/sound.scp",SoundDataStorage);
    printf("map.."); OutSCP(SCPDIR "/map.scp",MapDataStorage);
    printf("area.."); OutSCP(SCPDIR "/area.scp",AreaDataStorage);
    //...
    printf("DONE!\n");

	// wait for all container destructors to finish
	printf("DBC files converted, cleaning up...\n");

    return true;
}

void ExtractMaps(void)
{
    printf("\nExtracting maps...\n");
    char namebuf[200];
    char outbuf[2000];
    uint32 extr,extrtotal=0;
    MPQHelper mpq("terrain");
    CreateDir("stuffextract/data/maps");
    for(uint32 it=0; it < mapNames.size(); it++)
    {
        extr=0;
        for(uint32 x=0; x<64; x++)
        {
            for(uint32 y=0;y<64; y++)
            {
                uint32 olddeps;
                uint32 depdiff;
                sprintf(namebuf,"World\\Maps\\%s\\%s_%u_%u.adt",mapNames[it].c_str(),mapNames[it].c_str(),x,y);
                sprintf(outbuf,MAPSDIR"/%s_%u_%u.adt",mapNames[it].c_str(),x,y);
                if(mpq.FileExists(namebuf))
                {
                    ByteBuffer& bb = mpq.ExtractFile(namebuf);
                    if(bb.size())
                    {
                        std::fstream fh;
                        //printf("Extracting map [ %s ]\n",outbuf);
                        fh.open(outbuf, std::ios_base::out|std::ios_base::binary);
                        if(!fh.is_open())
                        {
                            printf("\nERROR: Map extraction failed: could not save file %s\n",outbuf);
                            return;
                        }
                        fh.write((char*)bb.contents(),bb.size());
                        fh.close();
                        olddeps = texNames.size() + modelNames.size() + wmoNames.size();
                        ADT_FillTextureData(bb.contents(),texNames);
                        ADT_FillModelData(bb.contents(),modelNames); 
                        ADT_FillWMOData(bb.contents(),wmoNames); 
                        depdiff = texNames.size() + modelNames.size() + wmoNames.size() - olddeps;
                        extr++;
                        printf("[%u/%u]: %s; %u new deps.\n",it+1,mapNames.size(),namebuf,depdiff);
                    }
                }
            }
        }
        extrtotal+=extr;
        printf("\n");
    }

    printf("\nDONE - %u maps extracted, %u total dependencies.\n",extrtotal, texNames.size() + modelNames.size() + wmoNames.size());
}

void ExtractMapDependencies(void)
{
    printf("\nExtracting map dependencies...\n\n");
    printf("- Preparing to read MPQ arcives...\n");
    MPQHelper mpqmodel("model");
    MPQHelper mpqtex("texture");
    MPQHelper mpqwmo("wmo");
    std::string path = "stuffextract/data";
    std::string pathtex = path + "/texture";
    std::string pathmodel = path + "/model";
    std::string pathwmo = path + "/wmo";
    std::string mpqfn,realfn;
    CreateDir(pathtex.c_str());
    CreateDir(pathmodel.c_str());
    CreateDir(pathwmo.c_str());
    uint32 wmosdone=0,texdone=0,mdone=0;

    for(std::set<std::string>::iterator i = texNames.begin(); i != texNames.end(); i++)
    {
        mpqfn = *i;
        if(!mpqtex.FileExists((char*)mpqfn.c_str()))
            continue;
        realfn = pathtex + "/" + _PathToFileName(mpqfn);
        std::fstream fh;
        fh.open(realfn.c_str(),std::ios_base::out | std::ios_base::binary);
        if(fh.is_open())
        {
            ByteBuffer& bb = mpqtex.ExtractFile((char*)mpqfn.c_str());
            fh.write((const char*)bb.contents(),bb.size());
            texdone++;
            printf("- textures... %u\r",texdone);
        }
        else
            printf("Could not write texture %s\n",realfn.c_str());
        fh.close();
    }
    printf("\n");

    for(std::set<std::string>::iterator i = modelNames.begin(); i != modelNames.end(); i++)
    {
        mpqfn = *i;
        // no idea what bliz intended by this. the ADT files refer to .mdx models,
        // however there are only .m2 files in the MPQ archives.
        // so we just need to check if there is a .m2 file instead of the .mdx file, and load that one.
        if(!mpqmodel.FileExists((char*)mpqfn.c_str()))
        {
            std::string alt = i->substr(0,i->length()-3) + "m2";
            DEBUG(printf("MDX model not found, trying M2 file."));
            if(!mpqmodel.FileExists((char*)alt.c_str()))
            {
                DEBUG(printf(" fail.\n"));
                continue;
            }
            else
            {
                mpqfn = alt;
                DEBUG(printf(" success.\n"));
            }
        }
        realfn = pathmodel + "/" + _PathToFileName(mpqfn);
        std::fstream fh;
        fh.open(realfn.c_str(),std::ios_base::out | std::ios_base::binary);
        if(fh.is_open())
        {
            ByteBuffer& bb = mpqmodel.ExtractFile((char*)mpqfn.c_str());
            fh.write((const char*)bb.contents(),bb.size());
            mdone++;
            printf("- models... %u\r",mdone);
        }
        else
            printf("Could not write model %s\n",realfn.c_str());
        fh.close();
    }
    printf("\n");

    for(std::set<std::string>::iterator i = wmoNames.begin(); i != wmoNames.end(); i++)
    {
        mpqfn = *i;
        if(!mpqwmo.FileExists((char*)mpqfn.c_str()))
            continue;
        realfn = pathwmo + "/" + _PathToFileName(mpqfn);
        std::fstream fh;
        fh.open(realfn.c_str(),std::ios_base::out | std::ios_base::binary);
        if(fh.is_open())
        {
            ByteBuffer& bb = mpqwmo.ExtractFile((char*)mpqfn.c_str());
            fh.write((const char*)bb.contents(),bb.size());
            wmosdone++;
            printf("- WMOs... %u\r",wmosdone);
        }
        else
            printf("Could not write WMO %s\n",realfn.c_str());
        fh.close();
    }
    printf("\n");

}

// fix filenames for linux ( '/' instead of windows '\')
void _FixFileName(std::string& str)
{
    for(uint32 i = 0; i < str.length(); i++)
        if(str[i]=='\\')
            str[i]='/';
}

std::string _PathToFileName(std::string str)
{
    uint32 pathend = str.find_last_of("/\\");
    if(pathend != std::string::npos)
    {
        return str.substr(pathend+1);
    }
    return str;
}




