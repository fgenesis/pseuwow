#include <fstream>
#define _COMMON_NO_THREADS
#include "common.h"
#include "MPQHelper.h"
#include "dbcfile.h"
#include "StuffExtract.h"
#include "DBCFieldData.h"
int main(int argc, char *argv[])
{
    printf("StuffExtract [version %u]\n",SE_VERSION);
    ConvertDBC();


    printf("\n -- finished --\n");
    char crap[200];
    fgets(crap,sizeof(crap),stdin);

    //while(true);
    return 0;
}

// be careful using this, that you supply correct format string
std::string AutoGetDataString(DBCFile::Iterator& it, const char* format, uint32 field)
{
    if(format[field]=='i' || format[field]=='f')
        return toString( (*it).getInt(field) );
    if(format[field]=='s')
        return (*it).getString(field);
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
    SCPStorageMap EmoteDataStorage,RaceDataStorage,SoundDataStorage; // will store the converted data from dbc files
    DBCFile EmotesText,EmotesTextData,EmotesTextSound,ChrRaces,SoundEntries;
    printf("Opening DBC archive...\n");
    MPQHelper mpq;
    if(!mpq.AssignArchive("Data/dbc.MPQ"))
    {
        printf("ConvertDBC: Could not open 'Data/dbc.MPQ'\n");
        return false;
    }
    printf("Opening DBC files...\n");
    EmotesText.openmem(mpq.ExtractFile("DBFilesClient\\EmotesText.dbc"));
    EmotesTextData.openmem(mpq.ExtractFile("DBFilesClient\\EmotesTextData.dbc"));
    EmotesTextSound.openmem(mpq.ExtractFile("DBFilesClient\\EmotesTextSound.dbc"));
    ChrRaces.openmem(mpq.ExtractFile("DBFilesClient\\ChrRaces.dbc"));
    SoundEntries.openmem(mpq.ExtractFile("DBFilesClient\\SoundEntries.dbc"));
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
                        EmoteDataStorage[em].push_back( fname + "=" + (*ix).getString(EMOTESTEXTDATA_STRING) );
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

    //...
    printf("DONE!\n");
    //...
    printf("Writing SCP files:\n");    
    printf("emote.."); OutSCP(SCPDIR "/emote.scp",EmoteDataStorage);
    printf("race.."); OutSCP(SCPDIR "/race.scp",RaceDataStorage);
    printf("sound.."); OutSCP(SCPDIR "/sound.scp",SoundDataStorage);
    //...
    printf("DONE!\n");

    



    return true;
}