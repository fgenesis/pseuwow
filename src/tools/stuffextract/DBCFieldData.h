#ifndef DBCFIELDDATA_H
#define DBCFIELDDATA_H

// NOTE: fields checked for 1.12.2 deDE

// defines fields numbers in EmotesText.dbc
// unk fields are always 0, EMOTESTEXT_EMOTE_STRING is string and the rest is uint32.
enum EmotesTextEnum
{
    EMOTESTEXT_EMOTE_ID = 0,
    EMOTESTEXT_EMOTE_STRING,
    EMOTESTEXT_ANIM,
    EMOTESTEXT_SOMEONE_TO_SOMEONE,
    EMOTESTEXT_SOMEONE_TO_ME,
    EMOTESTEXT_ME_TO_SOMEONE,
    EMOTESTEXT_UNK_1,
    EMOTESTEXT_SOMEONE_TO_NONE,
    EMOTESTEXT_UNK_2,
    EMOTESTEXT_ME_TO_NONE,
    EMOTESTEXT_UNK_3,
    EMOTESTEXT_SOMEONE_TO_SOMEONE_FEMALE,
    EMOTESTEXT_SOMEONE_TO_ME_FEMALE,
    EMOTESTEXT_UNK_4, // me to someone (female)? dbc is empty.
    EMOTESTEXT_UNK_5,
    EMOTESTEXT_SOMEONE_TO_NONE_FEMALE,
    EMOTESTEXT_UNK_6,
    EMOTESTEXT_UNK_7, // me to none (female)? dbc is empty.
    EMOTESTEXT_UNK_8,
    EMOTESTEXT_END
};

static const char *EmotesTextFieldNames[] =
{
    "", // EMOTESTEXT_EMOTE_ID
    "", // EMOTESTEXT_EMOTE_STRING
    "", // EMOTESTEXT_ANIM
    "OneToOne", // EMOTESTEXT_SOMEONE_TO_SOMEONE
    "OneToMe", // EMOTESTEXT_SOMEONE_TO_ME
    "MeToOne", // EMOTESTEXT_ME_TO_SOMEONE
    "", // EMOTESTEXT_UNK_1
    "OneGeneral", // EMOTESTEXT_SOMEONE_TO_NONE
    "", // EMOTESTEXT_UNK_2
    "MeGeneral", // EMOTESTEXT_ME_TO_NONE
    "", // EMOTESTEXT_UNK_3
    "OneToOneFemale", // EMOTESTEXT_SOMEONE_TO_SOMEONE_FEMALE
    "OneToMeFemale", // EMOTESTEXT_SOMEONE_TO_ME_FEMALE
    "", // EMOTESTEXT_UNK_4
    "", // EMOTESTEXT_UNK_5
    "OneGeneralFemale", // EMOTESTEXT_SOMEONE_TO_NONE_FEMALE
    "", // EMOTESTEXT_UNK_6
    "", // EMOTESTEXT_UNK_7
    "", // EMOTESTEXT_UNK_8
    "" // EMOTESTEXT_END
};

enum EmotesTextDataEnum
{
    EMOTESTEXTDATA_TEXTID = 0,
    EMOTESTEXTDATA_STRING = 4
    // rest of the fields is 0 always
};

enum EmotesTextSoundEnum
{
    EMOTESTEXTSOUND_TEXTID = 0,
    EMOTESTEXTSOUND_EMOTEID,
    EMOTESTEXTSOUND_RACE,
    EMOTESTEXTSOUND_ISFEMALE,
    EMOTESTEXTSOUND_SOUNDID,
    EMOTESTEXTSOUND_END
};

enum SoundEntriesEnum
{
    SOUNDENTRY_SOUNDID = 0,
    SOUNDENTRY_UNK_1, // seems to be a flag or something
    SOUNDENTRY_NAME_STRING,
    SOUNDENTRY_FILE_1,
    SOUNDENTRY_FILE_2,
    SOUNDENTRY_FILE_3,
    SOUNDENTRY_FILE_4,
    SOUNDENTRY_FILE_5,
    SOUNDENTRY_FILE_6,
    SOUNDENTRY_FILE_7,
    SOUNDENTRY_FILE_8,
    SOUNDENTRY_FILE_9,
    SOUNDENTRY_FILE_10,
    SOUNDENTRY_BOOL_1, // these fields store either 0 or 1
    SOUNDENTRY_BOOL_2, // no idea what they are good for..
    SOUNDENTRY_BOOL_3,
    SOUNDENTRY_BOOL_4,
    SOUNDENTRY_BOOL_5,
    SOUNDENTRY_BOOL_6,
    SOUNDENTRY_BOOL_7,
    SOUNDENTRY_BOOL_8,
    SOUNDENTRY_BOOL_9,
    SOUNDENTRY_BOOL_10,
    SOUNDENTRY_PATH, // path where SOUNDENTRY_FILE_x is stored. has no trailing '\'.
    SOUNDENTRY_UNK_2, // seems to be a float between 0...1 (sound volume?)
    SOUNDENTRY_UNK_3, // another field storing flags?
    SOUNDENTRY_UNK_4, // probably float data
    SOUNDENTRY_UNK_5, // probably float data
    SOUNDENTRY_UNK_6, // int value, mostly 0 or 2. only 2 cases where this field is 1
    SOUNDENTRY_END
};

static const char *SoundEntriesFormat = "ixsssssssssssiiiiiiiiiisxxxxx";

static const char *SoundEntriesFieldNames[] =
{
    "",
    "",
    "name",
    "file1", "file2", "file3", "file4", "file5", "file6", "file7", "file8", "file9", "file10", // 10 file names
    "","","","","","","","","","", // 10 unk bools
    "path",
    "volume", // ok lets just assume its volume (unk2)
    "", "", "", "", "" // unk3 - unk6
};

enum ChrRacesEnum
{
    CHRRACES_RACEID = 0,
    CHRRACES_UNK_1,
    CHRRACES_UNK_2,
    CHRRACES_UNK_3,
    CHRRACES_MODELM, // model for male player
    CHRRACES_MODELF, // model for female player
    CHRRACES_NAME_SHORT, // 2 chars name abbrev (Hu,Or,Tr, etc)
    CHRRACES_UNK_4,
    CHRRACES_FACTION, // 1=Alliance, 7=Horde
    CHRRACES_UNK_5,
    CHRRACES_UNK_6,
    CHRRACES_UNK_7,
    CHRRACES_UNK_8,
    CHRRACES_UNK_9,
    CHRRACES_UNK_10,
    CHRRACES_NAME_GENERAL, // always the english(?) name (without spaces). used in texture names etc.
    CHRRACES_UNK_11,
    CHRRACES_UNK_12,
    CHRRACES_UNK_13,
    CHRRACES_UNK_14,
    CHRRACES_NAME_LOCAL, // localized(?) name (spanish,german or whatever)
    CHRRACES_UNK_15,
    CHRRACES_UNK_16,
    CHRRACES_UNK_17,
    CHRRACES_UNK_18,
    CHRRACES_UNK_19,
    CHRRACES_UNK_20,
    CHRRACES_TRAITS, // string that defines the face decorations on char create
    CHRRACES_UNK_21,
    CHARRACES_END
};

static const char *ChrRacesFormat = "ixxxiisxixxxxxxsxxxxsxxxxxxsx";

static const char *ChrRacesFieldNames[] =
{
    "",
    "",
    "",
    "",
    "model_m",
    "model_f",
    "name_short",
    "",
    "faction",
    "",
    "",
    "",
    "",
    "",
    "",
    "name_general",
    "",
    "",
    "",
    "",
    "name",
    "",
    "",
    "",
    "",
    "",
    "",
    "traits",
    "",
    ""
};

#endif
