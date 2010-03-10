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
#include "ProgressBar.h"
#include "../../Client/GUI/CM2MeshFileLoader.h"
#include "../../Client/GUI/CWMOMeshFileLoader.h"

int replaceSpaces (int i) { return i==(int)' ' ? (int)'_' : i; }

std::map<uint32,std::string> mapNames;

std::set<NameAndAlt> texNames;
std::set<NameAndAlt> modelNames;
std::set<NameAndAlt> wmoNames;
std::set<NameAndAlt> wmoGroupNames;
std::set<NameAndAlt> soundFileSet;


// default config; SCPs are done always
bool doMaps=true, doSounds=false, doTextures=false, doWmos=false, doWmogroups=false, doModels=false, doMd5=true, doAutoclose=false;



int main(int argc, char *argv[])
{
    char input[200];
    printf("StuffExtract [version %u]\n",SE_VERSION);
    printf("Use -help or -? to display help about command line arguments and config.\n\n");
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
        CreateDir("extractedstuff");
        CreateDir("extractedstuff/data");
		ConvertDBC();
        if(doMaps) ExtractMaps();
        if(doTextures || doModels || doWmos || doWmogroups) ExtractMapDependencies();
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
            else if(!stricmp(what,"wmogroups"))   doWmogroups = on;
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
        doWmos = false;
    }
    if(!doWmos)
    {
        doWmogroups = false;
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
    printf("config: Do wmogroups: %s\n",doWmogroups?"yes":"no");
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
    printf("textures  - extract textures\n");
    printf("wmos      - extract map WMOs (requires maps extraction)\n");
    printf("wmogroups - extract map WMO group files (requires maps and wmos extraction)\n");
    printf("models    - extract models\n");
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
std::string AutoGetDataString(DBCFile::Iterator& it, const char* format, uint32 field, bool skip_null = true)
{
    if(format[field]=='i')
    {
        if((*it).getInt(field) == 0 && skip_null)
            return ""; // do not explicitly write int fields that are 0
        std::stringstream s;
        s << (*it).getInt(field);
        return s.str();
    }
    else if(format[field]=='f')
    {
        if((*it).getFloat(field) == 0 && skip_null)
            return ""; // do not explicitly write float fields that are 0
        std::stringstream s;
        s << (*it).getFloat(field);
        return s.str();
    }
    else if(format[field]=='c')
    {
        if((*it).getUChar(field) == 0 && skip_null)
            return ""; // do not explicitly write float fields that are 0
        std::stringstream s;
        s << (int)(*it).getUChar(field);
        return s.str();
    }
    else if(format[field]=='s' && (*it).getUInt(field))
    {
        return (*it).getString(field);
    }

    return "";
}


// output a formatted scp file
void OutSCP(const char *fn, SCPStorageMap& scp, std::string dbName="")
{
    std::fstream f;
    f.open(fn, std::ios_base::out);
    if(f.is_open())
    {
        if(dbName.length())
        {
            f << "#dbname=" << dbName << "\n";
        }
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

void OutMD5(const char *path, MD5FileMap& fm)
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
    std::map<uint8,uint32> classmask; //from CharBaseInfo.dbc
    SCPStorageMap EmoteDataStorage,RaceDataStorage,SoundDataStorage,MapDataStorage,ZoneDataStorage,ItemDisplayInfoStorage,
        CreatureModelStorage,CreatureDisplayInfoStorage,NPCSoundStorage,CharSectionStorage, GameObjectDisplayInfoStorage, ChrBaseInfoStorage; // will store the converted data from dbc files
    DBCFile EmotesText,EmotesTextData,EmotesTextSound,ChrRaces,SoundEntries,Map,AreaTable,ItemDisplayInfo,
        CreatureModelData,CreatureDisplayInfo,NPCSounds,CharSections,GameObjectDisplayInfo, ChrBaseInfo;
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
    CreatureModelData.openmem(mpq.ExtractFile("DBFilesClient\\CreatureModelData.dbc"));
    CreatureDisplayInfo.openmem(mpq.ExtractFile("DBFilesClient\\CreatureDisplayInfo.dbc"));
    GameObjectDisplayInfo.openmem(mpq.ExtractFile("DBFilesClient\\GameObjectDisplayInfo.dbc"));
    NPCSounds.openmem(mpq.ExtractFile("DBFilesClient\\NPCSounds.dbc"));
    CharSections.openmem(mpq.ExtractFile("DBFilesClient\\CharSections.dbc"));
    ChrBaseInfo.openmem(mpq.ExtractFile("DBFilesClient\\CharBaseInfo.dbc"));
    //...
    printf("DBC files opened.\n");
    //...
    printf("Reading data: chrbaseinfo..");
    for(DBCFile::Iterator it = ChrBaseInfo.begin(); it != ChrBaseInfo.end(); ++it)
    {
        uint32 race = (uint32)(*it).getUChar(CBI_RACE);
        uint32 cclass = (uint32)(*it).getUChar(CBI_CLASS);
        classmask[race] |= 1<<cclass;

    }

    printf("races..");
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
        if (doModels && classmask[id]) // skip nonplayable races
        {
            // corpse models
            
            modelNames.insert(NameAndAlt("World\\Generic\\PassiveDoodads\\DeathSkeletons\\" + racemap[id] + "MaleDeathSkeleton.m2"));
            modelNames.insert(NameAndAlt("World\\Generic\\PassiveDoodads\\DeathSkeletons\\" + racemap[id] + "FemaleDeathSkeleton.m2"));
        }

        std::stringstream temp;
        temp << classmask[id];
        RaceDataStorage[id].push_back(std::string("classmask").append("=").append(temp.str()));
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
                    if(textid == (*it).getUInt(field))
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
                        soundFileSet.insert(NameAndAlt(path + value));
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

    printf("zonedata..");
    for(DBCFile::Iterator it = AreaTable.begin(); it != AreaTable.end(); ++it)
    {
        uint32 id = it->getUInt(MAP_ID);
        for(uint32 field=AREATABLE_ID; field < AREATABLE_END; field++)
        {
            if(strlen(AreaTableFieldNames[field]))
            {
                std::string value = AutoGetDataString(it,AreaTableFormat,field,false);
                if(value.size()) // only store if not null
                    ZoneDataStorage[id].push_back(std::string(AreaTableFieldNames[field]) + "=" + value);
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
                // TODO: need to get
                std::string value = AutoGetDataString(it,ItemDisplayInfoFormat,field);
                if(value.size()) // only store if not null
                    ItemDisplayInfoStorage[id].push_back(std::string(ItemDisplayInfoFieldNames[field]) + "=" + value);
            }
        }
    }

    printf("creaturemodeldata..");
    for(DBCFile::Iterator it = CreatureModelData.begin(); it != CreatureModelData.end(); ++it)
    {
        uint32 id = it->getUInt(CREATUREMODELDATA_ID);
        for(uint32 field=CREATUREMODELDATA_ID; field < CREATUREMODELDATA_END; field++)
        {
            if(strlen(CreatureModelDataFieldNames[field]))
            {
                std::string value = AutoGetDataString(it,CreatureModelDataFormat,field);
                if(value.size()) // only store if not null
                {
                    if(doModels)
                        modelNames.insert(NameAndAlt(value)); // we need to extract model later, store it
                    std::string fn = _PathToFileName(value);
                    if(stricmp(fn.c_str()+fn.length()-4, "mdx"))
                        fn = fn.substr(0,fn.length()-3) + "m2";
                    CreatureModelStorage[id].push_back(std::string(CreatureModelDataFieldNames[field]) + "=" + fn);
                }
            }
        }
    }

    printf("creaturedisplayinfo..");
    for(DBCFile::Iterator it = CreatureDisplayInfo.begin(); it != CreatureDisplayInfo.end(); ++it)
    {
        uint32 id = it->getUInt(CREATUREDISPLAYINFO_ID);
        uint32 modelid = it->getUInt(CREATUREDISPLAYINFO_MODEL);

        for(uint32 field=CREATUREDISPLAYINFO_ID; field < CREATUREDISPLAYINFO_END; field++)
        {
            if(strlen(CreatureDisplayInfoFieldNames[field]))
            {
                std::string value = AutoGetDataString(it,CreatureDisplayInfoFormat,field);
                if(value.size()) // only store if not null
                {
                    if(doTextures && field >= CREATUREDISPLAYINFO_NAME1 && field <= CREATUREDISPLAYINFO_NAME3)
                    {
                        // lookup for model path
                        DBCFile::Iterator itm = CreatureModelData.begin();
                        for(; itm != CreatureDisplayInfo.end() && itm->getUInt(CREATUREMODELDATA_ID) != modelid;) ++itm;

                        std::string str = itm->getString(CREATUREMODELDATA_FILE);
                        uint32 pathend = str.find_last_of("/\\");
                        if(pathend != std::string::npos) // replace model with texture name
                            str = str.substr(0, pathend);
                        str += "\\";
                        str += value;
                        str += ".blp";
                        texNames.insert(NameAndAlt(str));

                        value = NormalizeFilename(str);
                    }

                    CreatureDisplayInfoStorage[id].push_back(std::string(CreatureDisplayInfoFieldNames[field]) + "=" + value);
                }
            }
        }
    }

    printf("gameobjectdisplayinfo..");
    for(DBCFile::Iterator it = GameObjectDisplayInfo.begin(); it != GameObjectDisplayInfo.end(); ++it)
    {
        uint32 id = it->getUInt(GAMEOBJECTDISPLAYINFO_ID);

        for(uint32 field=GAMEOBJECTDISPLAYINFO_ID; field < GAMEOBJECTDISPLAYINFO_END; field++)
        {
            if(strlen(GameObjectDisplayInfoFieldNames[field]))
            {
                std::string value = AutoGetDataString(it,GameObjectDisplayInfoFormat,field);
                if(value.size()) // only store if not null
                {
                    // TODO: add check for wmo model files ?
                    if(doModels && stricmp(value.c_str()+value.length()-4,".wmo"))
                        modelNames.insert(NameAndAlt(value)); // we need to extract model later, store it
                    else if (doWmos && !stricmp(value.c_str()+value.length()-4,".wmo"))
                        wmoNames.insert(NameAndAlt(value)); //this is a WMO

                    //Interestingly, some of the files referenced here have MDL extension - WTF?
                    std::string fn = _PathToFileName(value);
                    if(!stricmp(fn.c_str()+fn.length()-3, "mdx") || !stricmp(fn.c_str()+fn.length()-3, "mdl"))
                        fn = fn.substr(0,fn.length()-3) + "m2";
                    else
                        printf("This should be a WMO: %s\n",fn.c_str());
                    GameObjectDisplayInfoStorage[id].push_back(std::string(GameObjectDisplayInfoFieldNames[field]) + "=" + fn);

                    std::string texture = value.substr(0,value.length()-3) + "blp";
                    if (mpq.FileExists((char*)texture.c_str()))
                    {
                        if(doTextures)
                            texNames.insert(NameAndAlt(texture));
                        GameObjectDisplayInfoStorage[id].push_back("texture=" + NormalizeFilename(texture));
                    }
                }
            }
        }
    }

    printf("npcsounds..");
    for(DBCFile::Iterator it = NPCSounds.begin(); it != NPCSounds.end(); ++it)
    {
        uint32 id = it->getUInt(NPCSOUNDS_ID);
        for(uint32 field=NPCSOUNDS_ID; field < NPCSOUNDS_END; field++)
        {
            if(strlen(NPCSoundsFieldNames[field]))
            {
                std::string value = AutoGetDataString(it,NPCSoundsFormat,field);
                if(value.size()) // only store if not null
                    NPCSoundStorage[id].push_back(std::string(NPCSoundsFieldNames[field]) + "=" + value);
            }
        }
    }

    printf("charsections..");
    for(DBCFile::Iterator it = CharSections.begin(); it != CharSections.end(); ++it)
    {
        uint32 id = it->getUInt(CHARSECTIONS_ID);
        for(uint32 field=CHARSECTIONS_ID; field < CHARSECTIONS_END; field++)
        {
            if(strlen(CharSectionsFieldNames[field]))
            {
                std::string value = AutoGetDataString(it,CharSectionsFormat,field);
                if(value.size()) // only store if not null
                {
                    // ok we have a little problem here:
                    // some textures used for different races have the same file name, but we are storing them all
                    // in one directory. Texture path format is: "Character\<race>\<texture>
                    // so we have to use good names to store all textures without overwriting each other
                    if(field >= CHARSECTIONS_TEXTURE1 && field <= CHARSECTIONS_TEXTURE3)
                    {
                        /*char buf[100];
                        sprintf(buf,"charsection_%u_%u_%u_%u_%u_%u.blp",
                            it->getUInt(CHARSECTIONS_RACE_ID),
                            it->getUInt(CHARSECTIONS_GENDER),
                            it->getUInt(CHARSECTIONS_TYPE),
                            it->getUInt(CHARSECTIONS_SECTION),
                            it->getUInt(CHARSECTIONS_COLOR),
                            field - CHARSECTIONS_TEXTURE1); // texture ID */
                        texNames.insert(NameAndAlt(value));
                    }
                    CharSectionStorage[id].push_back(std::string(CharSectionsFieldNames[field]) + "=" + NormalizeFilename(value));
                }
            }
        }
    }


    //...
    printf("DONE!\n");
    //...

    CreateDir("extractedstuff/data/scp");

    printf("Writing SCP files:\n");
    printf("emote.."); OutSCP(SCPDIR "/emote.scp",EmoteDataStorage, "emote");
    printf("race.."); OutSCP(SCPDIR "/race.scp",RaceDataStorage, "race");
    printf("sound.."); OutSCP(SCPDIR "/sound.scp",SoundDataStorage, "sound");
    printf("map.."); OutSCP(SCPDIR "/map.scp",MapDataStorage, "map");
    printf("area.."); OutSCP(SCPDIR "/zone.scp",ZoneDataStorage, "zone");
    printf("itemdisplayinfo.."); OutSCP(SCPDIR "/itemdisplayinfo.scp",ItemDisplayInfoStorage, "itemdisplayinfo");
    printf("creaturemodeldata.."); OutSCP(SCPDIR "/creaturemodeldata.scp",CreatureModelStorage,"creaturemodeldata");
    printf("creaturedisplayinfo.."); OutSCP(SCPDIR "/creaturedisplayinfo.scp",CreatureDisplayInfoStorage,"creaturedisplayinfo");
    printf("gameobjectdisplayinfo.."); OutSCP(SCPDIR "/gameobjectdisplayinfo.scp",GameObjectDisplayInfoStorage,"gameobjectdisplayinfo");
    printf("npcsound.."); OutSCP(SCPDIR "/npcsound.scp",NPCSoundStorage,"npcsound");
    printf("charsections.."); OutSCP(SCPDIR "/charsections.scp",CharSectionStorage,"charsections");
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
    CreateDir("extractedstuff/data/maps");
    for(std::map<uint32,std::string>::iterator it = mapNames.begin(); it != mapNames.end(); it++)
    {
        // extract the WDT file that stores tile information
        char wdt_name[300], wdt_out[300];
        sprintf(wdt_name,"World\\Maps\\%s\\%s.wdt",it->second.c_str(),it->second.c_str());
        sprintf(wdt_out,MAPSDIR"/%lu.wdt",it->first);
        const ByteBuffer& wdt_bb = mpq.ExtractFile(wdt_name);
        std::fstream wdt_fh;
        wdt_fh.open(wdt_out, std::ios_base::out|std::ios_base::binary);
        if(!wdt_fh.is_open())
        {
            printf("\nERROR: Map extraction failed: could not save file %s\n",wdt_out);
            return;
        }
        if (wdt_bb.size())
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
                sprintf(namebuf,"World\\Maps\\%s\\%s_%lu_%lu.adt",it->second.c_str(),it->second.c_str(),x,y);
                sprintf(outbuf,MAPSDIR"/%lu_%lu_%lu.adt",it->first,x,y);
                if(mpq.FileExists(namebuf))
                {
                    const ByteBuffer& bb = mpq.ExtractFile(namebuf);
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
                        printf("[%lu:%lu] %s; %lu new deps.\n",extr,it->first,namebuf,depdiff);
                    }
                }
            }
        }
        extrtotal+=extr;
        printf("\n");
    }

    printf("\nDONE - %lu maps extracted, %u total dependencies.\n",extrtotal, texNames.size() + modelNames.size() + wmoNames.size());
    OutMD5(MAPSDIR,md5map);
}

void ExtractMapDependencies(void)
{
    barGoLink *bar;
    printf("\nExtracting map dependencies...\n\n");
    printf("- Preparing to read MPQ arcives...\n");
    MPQHelper mpqmodel("model");
    MPQHelper mpqtex("texture");
    MPQHelper mpqwmo("wmo");
    std::string path = "extractedstuff/data";
    std::string pathtex = path + "/texture";
    std::string pathmodel = path + "/model";
    std::string pathwmo = path + "/wmo";
    std::string mpqfn,realfn,altfn;
    MD5FileMap md5Tex, md5Wmo, md5Wmogroup, md5Model;
    CreateDir(pathtex.c_str());
    CreateDir(pathmodel.c_str());
    CreateDir(pathwmo.c_str());
    uint32 wmosdone=0,texdone=0,mdone=0;

    if(doWmos)
    {
        printf("Extracting %u WMOS...\n",wmoNames.size());

        bar = new barGoLink(wmoNames.size(),true);
        for(std::set<NameAndAlt>::iterator i = wmoNames.begin(); i != wmoNames.end(); i++)
        {
            bar->step();
            mpqfn = i->name;
            altfn = i->alt;
            if(altfn.empty())
                altfn = mpqfn;
            if(!mpqwmo.FileExists((char*)mpqfn.c_str()))
                continue;
            realfn = pathwmo + "/" + NormalizeFilename(_PathToFileName(altfn));
            std::fstream fh;
            fh.open(realfn.c_str(),std::ios_base::out | std::ios_base::binary);
            if(fh.is_open())
            {
                const ByteBuffer& bb = mpqwmo.ExtractFile((char*)mpqfn.c_str());
                fh.write((const char*)bb.contents(),bb.size());
                //Extract number of group files, Texture file names and M2s from WMO
                if(doWmogroups || doTextures || doModels) WMO_Parse_Data(bb,mpqfn.c_str(),doWmogroups,doTextures,doModels);
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
            }
            else
                printf("Could not write WMO %s\n",realfn.c_str());
            fh.close();
        }
        printf("\n");
        if(wmoNames.size())
            OutMD5((char*)pathwmo.c_str(),md5Wmo);
        delete bar;
    }

    if(doWmogroups)
    {
        printf("Extracting WMO Group Files...\n");
        bar = new barGoLink(wmoGroupNames.size(),true);
        for(std::set<NameAndAlt>::iterator i = wmoGroupNames.begin(); i != wmoGroupNames.end(); i++)
        {
            bar->step();
            mpqfn = i->name;
            altfn = i->alt;
            if(altfn.empty())
                altfn = mpqfn;
            if(!mpqwmo.FileExists((char*)mpqfn.c_str()))
                continue;
            realfn = pathwmo + "/" + NormalizeFilename(_PathToFileName(altfn));
            std::fstream fh;
            fh.open(realfn.c_str(),std::ios_base::out | std::ios_base::binary);
            if(fh.is_open())
            {
                const ByteBuffer& bb = mpqwmo.ExtractFile((char*)mpqfn.c_str());
                fh.write((const char*)bb.contents(),bb.size());
                if(doMd5)
                {
                    MD5Hash h;
                    h.Update((uint8*)bb.contents(), bb.size());
                    h.Finalize();
                    uint8 *md5ptr = new uint8[MD5_DIGEST_LENGTH];
                    md5Wmogroup[_PathToFileName(realfn)] = md5ptr;
                    memcpy(md5ptr, h.GetDigest(), MD5_DIGEST_LENGTH);
                }
                wmosdone++;
            }
            else
                printf("Could not write WMO %s\n",realfn.c_str());
            fh.close();
        }
        printf("\n");
        if(wmoGroupNames.size())
            OutMD5((char*)pathwmo.c_str(),md5Wmogroup);
        delete bar;
    }




    if(doModels)
    {
        printf("Extracting models...\n");
        bar = new barGoLink(modelNames.size(),true);
        for(std::set<NameAndAlt>::iterator i = modelNames.begin(); i != modelNames.end(); i++)
        {
            bar->step();
            mpqfn = i->name;
            // no idea what bliz intended by this. the ADT files refer to .mdx models,
            // however there are only .m2 files in the MPQ archives.
            // so we just need to check if there is a .m2 file instead of the .mdx file, and load that one.
            if(!mpqmodel.FileExists((char*)mpqfn.c_str()))
            {
                std::string alt = mpqfn.substr(0,mpqfn.length()-3) + "m2";
                if(!mpqmodel.FileExists((char*)alt.c_str()))
                {
                    printf("Failed to extract model: '%s'\n",alt.c_str());
                    continue;
                }
                else
                {
                    mpqfn = alt;
                }
            }
            altfn = i->alt;
            if(altfn.empty())
                altfn = mpqfn;
            realfn = pathmodel + "/" + NormalizeFilename(_PathToFileName(altfn));
            std::fstream fh;
            fh.open(realfn.c_str(),std::ios_base::out | std::ios_base::binary);
            if(fh.is_open())
            {
                ByteBuffer bb = mpqmodel.ExtractFile((char*)mpqfn.c_str());
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

                // model ok, now extract skins
                // for now first skin is all what we need
                std::string copy = mpqfn;
                std::transform(copy.begin(), copy.end(), copy.begin(), tolower);
                if (copy.find(".wmo") == std::string::npos)
                {
                    if (doTextures)
                        FetchTexturesFromModel(bb);

                    std::string skin = mpqfn.substr(0,mpqfn.length()-3) + "00.skin";
                    std::string skinrealfn = pathmodel + "/" + NormalizeFilename(_PathToFileName(skin));
                    if (mpqmodel.FileExists((char*)skin.c_str()))
                    {
                        std::fstream fhs;
                        fhs.open(skinrealfn.c_str(),std::ios_base::out | std::ios_base::binary);
                        if(fhs.is_open())
                        {
                            ByteBuffer bbs = mpqmodel.ExtractFile((char*)skin.c_str());
                            fhs.write((const char*)bbs.contents(),bbs.size());
                        }
                        else
                            printf("Could not write skin %s\n",skinrealfn.c_str());

                        fhs.close();
                    }
                    else
                        printf("Could not open skin %s\n",skin.c_str());
                }
            }
            else
                printf("Could not write model %s\n",realfn.c_str());
            fh.close();
        }
        printf("\n");
        if(modelNames.size())
            OutMD5((char*)pathmodel.c_str(),md5Model);
        delete bar;
    }

    if(doTextures)
    {
        printf("Extracting textures...\n");
        bar = new barGoLink(texNames.size(), true);
        for(std::set<NameAndAlt>::iterator i = texNames.begin(); i != texNames.end(); i++)
        {
            bar->step();
            mpqfn = i->name;
            altfn = i->alt;
            if(altfn.empty())
                altfn = mpqfn;
            if(!mpqtex.FileExists((char*)mpqfn.c_str()))
                continue;

            // prepare lowercased and "underlined" path for file
            std::string copy = NormalizeFilename(mpqfn);
            if (copy.find_first_of("/") != std::string::npos)
            {
                std::string copy2 = copy.c_str();
                char* tok = strtok((char*)copy2.c_str(),"/");
                std::string fullpath = pathtex;
                while (tok && !strstr(tok, ".blp"))
                {
                    fullpath += "/";
                    fullpath += tok;
                    CreateDir(fullpath.c_str());
                    tok = strtok(NULL, "/");
                }
            }

            realfn = pathtex + "/" + copy; //_PathToFileName(altfn);
            std::fstream fh;
            fh.open(realfn.c_str(),std::ios_base::out | std::ios_base::binary);
            if(fh.is_open())
            {
                const ByteBuffer& bb = mpqtex.ExtractFile((char*)mpqfn.c_str());
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
            }
            else
                printf("Could not write texture %s\n",realfn.c_str());
            fh.close();
        }
        printf("\n");
        if(texNames.size())
            OutMD5((char*)pathtex.c_str(),md5Tex);
        delete bar;
    }


}

void ExtractSoundFiles(void)
{
    MD5FileMap md5data;
    uint32 done = 0;
    printf("\nExtracting game audio files, %u found in DBC...\n",soundFileSet.size());
    CreateDir(SOUNDDIR);
    MPQHelper smpq("sound");
    std::string outfn, altfn;
    barGoLink bar(soundFileSet.size(),true);
    for(std::set<NameAndAlt>::iterator i = soundFileSet.begin(); i != soundFileSet.end(); i++)
    {
        bar.step();
        if(!smpq.FileExists((char*)i->name.c_str()))
        {
            DEBUG( printf("MPQ: File not found: '%s'\n",i->name.c_str()) );
            continue;
        }
        altfn = i->alt.empty() ? _PathToFileName(i->name) : i->alt;

        outfn = std::string(SOUNDDIR) + "/" + NormalizeFilename(altfn);
        std::fstream fh;
        fh.open(outfn.c_str(), std::ios_base::out | std::ios_base::binary);
        if(fh.is_open())
        {
            const ByteBuffer& bb = smpq.ExtractFile((char*)i->name.c_str());
            if(bb.size())
            {
                fh.write((const char*)bb.contents(),bb.size());
                if(doMd5)
                {
                    MD5Hash h;
                    h.Update((uint8*)bb.contents(), bb.size());
                    h.Finalize();
                    uint8 *md5ptr = new uint8[MD5_DIGEST_LENGTH];
                    md5data[altfn] = md5ptr;
                    memcpy(md5ptr, h.GetDigest(), MD5_DIGEST_LENGTH);
                }
                done++;
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

void WMO_Parse_Data(ByteBuffer bb, const char* _filename, bool groups, bool textures, bool models)
{
    bb.rpos(20); //Skip MVER chunk and header of MHDR
    irr::scene::RootHeader header;
    if (bb.size() < sizeof(header))
        return;
    bb.read((uint8*)&header, sizeof(header));
    if(groups)
    {
        std::string filename=_filename;
        for(uint32 i=0; i<header.nGroups; i++)
        {
            char grpfilename[255];
            sprintf(grpfilename,"%s_%03lu.wmo",filename.substr(0,filename.length()-4).c_str(),i);
            wmoGroupNames.insert(NameAndAlt(grpfilename));
        }

    }
    if(models || textures)
    {
        uint32 size;
        uint8 _cc[5];
        uint8 *fourcc = &_cc[0];
        fourcc[4]=0;

        while(bb.rpos()<bb.size())
        {
            bb.read((uint8*)fourcc,4);
            bb.read((uint8*)&size,4);
            if(!strcmp((char*)fourcc,"XTOM") && textures)
            {
                std::string temp;
                char c;
                uint32 read=0;
                while(read<size)
                {
                    bb.read((uint8*)&c,sizeof(char));
                    if(c=='\x0' && temp.size()>0)
                    {
                        texNames.insert(NameAndAlt(temp));
                        temp.clear();
                    }
                    else if(c!=0)
                        temp += c;
                    read++;
                }
            }
            else if(!strcmp((char*)fourcc,"NDOM") && models)
            {
                std::string temp;
                char c;
                uint32 read=0;
                while(read<size)
                {
                    bb.read((uint8*)&c,sizeof(char));
                    if(c=='\x0' && temp.size()>0)
                    {
                        modelNames.insert(NameAndAlt(temp));
                        temp.clear();
                    }
                    else if(c!=0)
                        temp += c;
                    read++;
                }
            }
            else
                bb.rpos(bb.rpos()+size);
        }
    }

}
void ADT_ExportStringSetByOffset(const uint8* data, uint32 off, std::set<NameAndAlt>& st,const char* stop)
{
    data += ((uint32*)data)[off]; // seek to correct absolute offset
    data += 28; // move ptr to real start of data
    uint32 offset=0;
    std::string s;
    char c;
    while(memcmp(data+offset,stop,4))
    {
        c = data[offset];
        if(!c)
        {
            if(s.length())
            {
                DEBUG(printf("DEP: %s\n",s.c_str()));
                st.insert(NameAndAlt(s));
                s.clear();
            }
        }
        else
            s += c;
        offset++;
    }
}

void ADT_FillTextureData(const uint8* data,std::set<NameAndAlt>& st)
{
    ADT_ExportStringSetByOffset(data,OFFSET_TEXTURES,st,"XDMM");
}

void ADT_FillWMOData(const uint8* data,std::set<NameAndAlt>& st)
{
    ADT_ExportStringSetByOffset(data,OFFSET_WMOS,st,"DIWM");
}

void ADT_FillModelData(const uint8* data,std::set<NameAndAlt>& st)
{
    ADT_ExportStringSetByOffset(data,OFFSET_MODELS,st,"DIMM");
}

void FetchTexturesFromModel(ByteBuffer bb)
{
    bb.rpos(0);
    irr::scene::ModelHeader header;
    if (bb.size() < sizeof(header))
        return;

    bb.read((uint8*)&header, sizeof(header));

    if (header.version[0] != 8 || header.version[1] != 1 || header.version[2] != 0 || header.version[3] != 0) {
        printf("Not M2 model file!");
        return;
    }

    irr::core::array<irr::scene::TextureDefinition> M2MTextureDef;
    M2MTextureDef.clear();
    irr::scene::TextureDefinition tempM2TexDef;

    bb.rpos(header.ofsTextures);
    for(irr::u32 i=0;i<header.nTextures;i++)
    {
        bb.read((uint8*)&tempM2TexDef,sizeof(irr::scene::TextureDefinition));
        M2MTextureDef.push_back(tempM2TexDef);
    }

    std::string tempTexFileName="";
    for(irr::u32 i=0; i<M2MTextureDef.size(); i++)
    {
        bb.rpos(M2MTextureDef[i].texFileOfs);
        tempTexFileName.resize(M2MTextureDef[i].texFileLen+1);
        bb.read((uint8*)&tempTexFileName[0],M2MTextureDef[i].texFileLen);

        if (tempTexFileName.empty())
            continue;
        // printf(tempTexFileName.c_str()); // for debug
        texNames.insert(NameAndAlt(tempTexFileName));
    }

}
