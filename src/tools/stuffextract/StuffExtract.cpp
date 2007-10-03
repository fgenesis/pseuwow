#include <fstream>
#include <set>
#define _COMMON_NO_THREADS
#include "common.h"
#include "Auth/MD5Hash.h"
#include "tools.h"
#include "MPQHelper.h"
#include "dbcfile.h"
#include "ADTFile.h"
#include "WDTFile.h"
#include "StuffExtract.h"
#include "DBCFieldData.h"
#include "Locale.h"

std::map<uint32,std::string> mapNames;
std::set<std::string> texNames;
std::set<std::string> modelNames;
std::set<std::string> wmoNames;
std::set<std::string> soundFileSet;

// default config; SCPs are dont always
bool doMaps=true, doSounds=false, doTextures=false, doWmos=false, doModels=false, doMd5=true, doAutoclose=false;



int main(int argc, char *argv[])
{
    char input[200];
    printf("StuffExtract [version %u]\n\n",SE_VERSION);
    ProcessCmdArgs(argc, argv);
    PrintConfig();
    if(!GetLocale())
    {
	    printf("Enter your locale (enUS, enGB, deDE, ...) or leave blank to autodetect: ");
	    fgets(input,sizeof(input),stdin);
	    char loc[5];
        input[strlen(input)-1] = 0;
	    memcpy(loc,input,4); loc[4]=0;	
	    SetLocale(loc);
    }
	if(GetLocale() && FileExists(std::string("Data/")+GetLocale()+"/locale-"+GetLocale()+".MPQ"))
	{
		printf("Locale \"%s\" seems valid, starting conversion...\n",GetLocale());
        CreateDir("stuffextract");
        CreateDir("stuffextract/data");
		ConvertDBC();
        if(doMaps) ExtractMaps();
        if(doTextures || doModels || doWmos) ExtractMapDependencies();
        if(doSounds) ExtractSoundFiles();
		//...
		if (!doAutoclose)
            printf("\n -- finished, press enter to exit --\n");
	}
	else
	{
		printf("ERROR: Invalid locale \"%s\"! Press Enter to exit...\n",GetLocale());
	}
    if (!doAutoclose)
        fgets(input,sizeof(input),stdin);

    //while(true);
    return 0;
}

void ProcessCmdArgs(int argc, char *argv[])
{
    bool on,help=false;
    char *what;
    for(int i = 1; i < argc; i++)
    {
        if(strlen(argv[i]) > 1)
        {
            if(argv[i][0] == '-')
                on = false;
            else if(argv[i][0] == '+')
                on = true;
            else if(!stricmp(argv[i],"/?") || !stricmp(argv[i],"/help"))
            {
                help = true;
                break;
            }
            else
            {
                printf("Incorrect cmd arg: \"%s\"\n",argv[i]);
                continue;
            }

            what = argv[i]+1; // skip first byte (+/-)
            if     (!stricmp(what,"maps"))        doMaps = on;
            else if(!stricmp(what,"textures"))    doTextures = on;
            else if(!stricmp(what,"wmos"))        doWmos = on;
            else if(!stricmp(what,"models"))      doModels = on;
            else if(!stricmp(what,"sounds"))      doSounds = on;
            else if(!stricmp(what,"md5"))         doMd5 = on;
            else if(!stricmp(what,"autoclose"))   doAutoclose = on;
            // autodetect or use given locale.   + or - as arg start doesnt matter here
            else if(!strnicmp(what,"locale:",7))
            {
                if(!stricmp(what+7,"auto"))
                    SetLocale(NULL);
                else 
                    SetLocale(what+7);
            }
            else if(!stricmp(what,"?") || !stricmp(what,"help"))
            {
                help = true;
                break;
            }
            else
            {
                printf("Unknown cmd arg: \"%s\"\n",what);
            }
        }
    }
    // fix up wrong configs. not necessary but better for display.
    // TODO: as soon as M2 model or WMO reading is done, extract those textures to, but independent from maps!!
    if(!doMaps)
    {
        doTextures = false;
        doWmos = false;
        doModels = false;
    }
    if(help)
    {
        PrintHelp();
        printf("\n\n- Press any key to exit -\n");
        getchar();
        exit(0);
    }
}

void PrintConfig(void)
{
    printf("config: Do maps:      %s\n",doMaps?"yes":"no");
    printf("config: Do textures:  %s\n",doTextures?"yes":"no");
    printf("config: Do wmos:      %s\n",doWmos?"yes":"no");
    printf("config: Do models:    %s\n",doModels?"yes":"no");
    printf("config: Do sounds:    %s\n",doSounds?"yes":"no");
    printf("config: Calc md5:     %s\n",doMd5?"yes":"no");
    printf("config: Autoclose:    %s\n",doAutoclose?"yes":"no");
}

void PrintHelp(void)
{
    printf("Usage information:\n\n");
    printf("Use + or - to turn a feature on or off.\n");
    printf("Features are:\n");
    printf("maps      - map extraction\n");
    printf("textures  - extract textures (requires maps extraction, for now)\n");
    printf("wmos      - extract map WMOs (requires maps extraction)\n");
    printf("models    - extract models   (required maps extraction, for now)\n");
    printf("sounds    - extract sound files (wav/mp3)\n");
    printf("md5       - write MD5 checksum lists of extracted files\n");
    printf("autoclose - close program when done\n");
    printf("\n");
    printf("Use -locale:xxXX to set a locale. If you don't use this, you will be asked.\n");
    printf("Use -locale:auto to autodetect currently used locale.\n");
    printf("\n");
    printf("Examples:\n");
    printf("stuffextract +sounds +md5 -maps +autoclose -locale:enGB\n");
    printf("stuffextract +md5 -wmos -sounds -locale:auto -autoclose\n");
    printf("\nDefault is: +maps -sounds -textures -wmos -models +md5 -autoclose\n");
}


// be careful using this, that you supply correct format string
std::string AutoGetDataString(DBCFile::Iterator& it, const char* format, uint32 field)
{
    if(format[field]=='i')
    {
        if((*it).getInt(field) == 0)
            return ""; // do not explicitly write int fields that are 0
        std::stringstream s;
        s << (*it).getInt(field);
        return s.str();
    }
    else if(format[field]=='f')
    {
        if((*it).getFloat(field) == 0)
            return ""; // do not explicitly write float fields that are 0
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

void OutMD5(char *path, MD5FileMap& fm)
{
    if(!doMd5)
        return;
    std::string fullname(path);
    fullname += "/md5.txt";
    printf("Writing MD5 file checksums to '%s'\n",fullname.c_str());
    std::fstream fh;
    fh.open(fullname.c_str(), std::ios_base::out);
    if(fh.is_open())
    {
        for(MD5FileMap::iterator i = fm.begin(); i != fm.end(); i++)
        {
            fh << i->first << "|" << toHexDump(i->second,MD5_DIGEST_LENGTH,false) << std::endl; // write file content
            delete [] i->second; // and delete previously allocated memory
        }
        fh.close();
    }
    else
    {
        printf("Couldn't output MD5 list to '%s'\n",fullname.c_str());
    }
}

        


bool ConvertDBC(void)
{
    std::map<uint8,std::string> racemap; // needed to extract other dbc files correctly
    SCPStorageMap EmoteDataStorage,RaceDataStorage,SoundDataStorage,MapDataStorage,AreaDataStorage,ItemDisplayInfoStorage; // will store the converted data from dbc files
    DBCFile EmotesText,EmotesTextData,EmotesTextSound,ChrRaces,SoundEntries,Map,AreaTable,ItemDisplayInfo;
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
    ItemDisplayInfo.openmem(mpq.ExtractFile("DBFilesClient\\ItemDisplayInfo.dbc"));
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
        std::string path = (*it).getString(SOUNDENTRY_PATH); // required to fill up the filename storage
        path += "\\"; // use backslash because mpq uses backslash too
        uint32 id = (*it).getUInt(SOUNDENTRY_SOUNDID);
        for(uint32 field=SOUNDENTRY_SOUNDID; field < SOUNDENTRY_END; field++)
        {
            if(strlen(SoundEntriesFieldNames[field]))
            {
                std::string value = AutoGetDataString(it,SoundEntriesFormat,field);
                if(value.size()) // only store if a file exists in that field
                {
                    SoundDataStorage[id].push_back(std::string(SoundEntriesFieldNames[field]) + "=" + value);

                    // fill up file storage. not necessary if we dont want to extract sounds
                    if(doSounds && field >= SOUNDENTRY_FILE_1 && field <= SOUNDENTRY_FILE_10)
                        soundFileSet.insert(path + value);
                }
            }
        }
    }

    printf("map info..");
    for(DBCFile::Iterator it = Map.begin(); it != Map.end(); ++it)
    {
        mapNames[it->getInt(MAP_ID)] = it->getString(MAP_NAME_GENERAL);
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

    printf("itemdisplayinfo..");
    for(DBCFile::Iterator it = ItemDisplayInfo.begin(); it != ItemDisplayInfo.end(); ++it)
    {
        uint32 id = it->getUInt(ITEMDISPLAYINFO_ID);
        for(uint32 field=ITEMDISPLAYINFO_ID; field < ITEMDISPLAYINFO_END; field++)
        {
            if(strlen(ItemDisplayInfoFieldNames[field]))
            {
                std::string value = AutoGetDataString(it,ItemDisplayInfoFormat,field);
                if(value.size()) // only store if not null
                    ItemDisplayInfoStorage[id].push_back(std::string(ItemDisplayInfoFieldNames[field]) + "=" + value);
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
    printf("itemdisplayinfo."); OutSCP(SCPDIR "/itemdisplayinfo.scp",ItemDisplayInfoStorage);
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
    MD5FileMap md5map;
    CreateDir("stuffextract/data/maps");
    for(std::map<uint32,std::string>::iterator it = mapNames.begin(); it != mapNames.end(); it++)
    {
        // extract the WDT file that stores tile information
        char wdt_name[300], wdt_out[300];
        sprintf(wdt_name,"World\\Maps\\%s\\%s.wdt",it->second.c_str(),it->second.c_str());
        sprintf(wdt_out,MAPSDIR"/%u.wdt",it->first);
        ByteBuffer& wdt_bb = mpq.ExtractFile(wdt_name);
        std::fstream wdt_fh;
        wdt_fh.open(wdt_out, std::ios_base::out|std::ios_base::binary);
        if(!wdt_fh.is_open())
        {
            printf("\nERROR: Map extraction failed: could not save file %s\n",wdt_out);
            return;
        }
        wdt_fh.write((char*)wdt_bb.contents(),wdt_bb.size());
        wdt_fh.close();

        printf("Extracted WDT '%s'\n",wdt_name);

        // then extract all ADT files
        extr=0;
        for(uint32 x=0; x<64; x++)
        {
            for(uint32 y=0;y<64; y++)
            {
                uint32 olddeps;
                uint32 depdiff;
                sprintf(namebuf,"World\\Maps\\%s\\%s_%u_%u.adt",it->second.c_str(),it->second.c_str(),x,y);
                sprintf(outbuf,MAPSDIR"/%u_%u_%u.adt",it->first,x,y);
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
                        fh.flush();
                        fh.close();
                        olddeps = texNames.size() + modelNames.size() + wmoNames.size();

                        if(doTextures) ADT_FillTextureData(bb.contents(),texNames);
                        if(doModels)   ADT_FillModelData(bb.contents(),modelNames); 
                        if(doWmos)     ADT_FillWMOData(bb.contents(),wmoNames); 

                        depdiff = texNames.size() + modelNames.size() + wmoNames.size() - olddeps;
                        if(doMd5)
                        {
                            MD5Hash h;
                            h.Update((uint8*)bb.contents(), bb.size());
                            h.Finalize();
                            uint8 *md5ptr = new uint8[MD5_DIGEST_LENGTH];
                            md5map[_PathToFileName(outbuf)] = md5ptr;
                            memcpy(md5ptr, h.GetDigest(), MD5_DIGEST_LENGTH);
                        }
                        extr++;
                        printf("[%u:%u] %s; %u new deps.\n",extr,it->first,namebuf,depdiff);
                    }
                }
            }
        }
        extrtotal+=extr;
        printf("\n");
    }

    printf("\nDONE - %u maps extracted, %u total dependencies.\n",extrtotal, texNames.size() + modelNames.size() + wmoNames.size());
    OutMD5(MAPSDIR,md5map);
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
    MD5FileMap md5Tex, md5Wmo, md5Model;
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
            if(doMd5)
            {
                MD5Hash h;
                h.Update((uint8*)bb.contents(), bb.size());
                h.Finalize();
                uint8 *md5ptr = new uint8[MD5_DIGEST_LENGTH];
                md5Tex[_PathToFileName(realfn)] = md5ptr;
                memcpy(md5ptr, h.GetDigest(), MD5_DIGEST_LENGTH);
            }
            texdone++;
            printf("- textures... %u\r",texdone);
        }
        else
            printf("Could not write texture %s\n",realfn.c_str());
        fh.close();
    }
    printf("\n");
    OutMD5((char*)pathtex.c_str(),md5Tex);

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
            if(doMd5)
            {
                MD5Hash h;
                h.Update((uint8*)bb.contents(), bb.size());
                h.Finalize();
                uint8 *md5ptr = new uint8[MD5_DIGEST_LENGTH];
                md5Model[_PathToFileName(realfn)] = md5ptr;
                memcpy(md5ptr, h.GetDigest(), MD5_DIGEST_LENGTH);
            }
            mdone++;
            printf("- models... %u\r",mdone);
        }
        else
            printf("Could not write model %s\n",realfn.c_str());
        fh.close();
    }
    printf("\n");
    OutMD5((char*)pathmodel.c_str(),md5Model);

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
            if(doMd5)
            {
                MD5Hash h;
                h.Update((uint8*)bb.contents(), bb.size());
                h.Finalize();
                uint8 *md5ptr = new uint8[MD5_DIGEST_LENGTH];
                md5Wmo[_PathToFileName(realfn)] = md5ptr;
                memcpy(md5ptr, h.GetDigest(), MD5_DIGEST_LENGTH);
            }
            wmosdone++;
            printf("- WMOs... %u\r",wmosdone);
        }
        else
            printf("Could not write WMO %s\n",realfn.c_str());
        fh.close();
    }
    printf("\n");
    OutMD5((char*)pathwmo.c_str(),md5Wmo);

}

void ExtractSoundFiles(void)
{
    MD5FileMap md5data;
    uint32 done = 0;
    printf("\nExtracting game audio files, %u found in DBC...\n",soundFileSet.size());
    CreateDir(SOUNDDIR);
    MPQHelper smpq("sound");
    std::string outfn;
    for(std::set<std::string>::iterator i = soundFileSet.begin(); i != soundFileSet.end(); i++)
    {
        if(!smpq.FileExists((char*)(*i).c_str()))
        {
            DEBUG( printf("MPQ: File not found: '%s'\n",(*i).c_str()) );
            continue;
        }

        outfn = std::string(SOUNDDIR) + "/" + _PathToFileName(*i);
        std::fstream fh;
        fh.open(outfn.c_str(), std::ios_base::out | std::ios_base::binary);
        if(fh.is_open())
        {
            ByteBuffer& bb = smpq.ExtractFile((char*)(*i).c_str());
            if(bb.size())
            {
                fh.write((const char*)bb.contents(),bb.size());
                if(doMd5)
                {
                    MD5Hash h;
                    h.Update((uint8*)bb.contents(), bb.size());
                    h.Finalize();
                    uint8 *md5ptr = new uint8[MD5_DIGEST_LENGTH];
                    md5data[_PathToFileName(*i)] = md5ptr;
                    memcpy(md5ptr, h.GetDigest(), MD5_DIGEST_LENGTH);
                }
                done++;
                printf("- %u files done.\r",done);
            }
        }
        else
        {
            printf("Could not write sound file '%s'\n",outfn.c_str());
        }
        fh.close();
    }
    OutMD5(SOUNDDIR,md5data);
    printf("\n");
}




