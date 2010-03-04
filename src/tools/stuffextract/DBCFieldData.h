#ifndef DBCFIELDDATA_H
#define DBCFIELDDATA_H

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
    EMOTESTEXTDATA_STRING1,
	EMOTESTEXTDATA_STRING2,
	EMOTESTEXTDATA_STRING3,
	EMOTESTEXTDATA_STRING4,
	EMOTESTEXTDATA_STRING5,
	EMOTESTEXTDATA_STRING6,
	EMOTESTEXTDATA_STRING7,
	EMOTESTEXTDATA_STRING8,
	EMOTESTEXTDATA_UNK,
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
    CHRRACES_FACTION, // 7=Alliance, 1=Horde
    CHRRACES_UNK_5, // always 7 = Humanoid
    CHRRACES_UNK_6, // SpellID? was always 836 in 1.12.x, since 2.0.x its 15007
    CHRRACES_UNK_7,
    CHRRACES_NAME_GENERAL, // always the english(?) name (without spaces). used in texture names etc.
    CHRRACES_UNK_9,
    CHRRACES_UNK_10,
	// the following 8 fields contain either 0 or the race name, depending on the locale. 
    CHRRACES_NAME1, // english
    CHRRACES_NAME2, // <need info here>
    CHRRACES_NAME3, // <need info here>
    CHRRACES_NAME4, // german
    CHRRACES_NAME5, // <need info here>
    CHRRACES_NAME6, // <need info here>
    CHRRACES_NAME7, // <need info here>
    CHRRACES_NAME8, // <need info here>
    CHRRACES_UNK_11,
    CHRRACES_UNK_19,
    CHRRACES_UNK_20,
    CHRRACES_UNK_21,
    CHARRACES_END
};
                                     
static const char *ChrRacesFormat = "ixxxiisixxxsxxssssssssssssssssxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxi";

static const char *ChrRacesFieldNames[] =
{
    "",
    "",
    "",
    "",
    "model_m",
    "model_f",
    "name_short",
    "faction",
    "",
    "",
    "",
    "name_general",
    "",
    "name",
    "name",
    "name",
    "name",
    "name",
    "name",
    "name",
    "name",
    "",
    "",
    "",
    "",
    "",
	""
};

enum MapEnum
{
    MAP_ID = 0,
    MAP_NAME_GENERAL = 1,
    MAP_NAME1 = 4,
    MAP_NAME2,
    MAP_NAME3,
    MAP_NAME4,
    MAP_NAME5,
    MAP_NAME6,
    MAP_NAME7,
    MAP_NAME8,
    MAP_END = 75
};

static const char *MapFieldNames[] = 
{
    "","name_general","","","name","name","name","name","name","name",
    "name","name","","","","","","","","",
    "","","","","","","","","","",
    "","","","","","","","","","",
    "","","","","","","","","","",
    "","","","","","","","","","",
    "","","","","","","","","","",
    "","","","",
    ""
};

static const char *MapFormat = 
{
    "isxxssssss"
    "ssxxxxxxxx"
    "xxxxxxxxxx"
    "xxxxxxxxxx"
    "xxxxxxxxxx"
    "xxxxxxxxxx"
    "xxxxxxxxxx"
    "xxxxx"
};

enum AreaTableEnum
{
    AREATABLE_ID = 0,
    AREATABLE_PARENTMAP = 2,
    AREATABLE_FLAGS = 4, // 64 in normal (?) areas
    AREATABLE_LEVEL = 10,
    AREATABLE_NAME1 = 11,
    AREATABLE_NAME2,
    AREATABLE_NAME3,
    AREATABLE_NAME4,
    AREATABLE_NAME5,
    AREATABLE_NAME6,
    AREATABLE_NAME7,
    AREATABLE_NAME8,
    AREATABLE_FACTION = 20, // 2 (alliance?) or 4 (horde?)
    AREATABLE_END = 27,
};

static const char *AreaTableFieldNames[] = {
    "","","map","","flags","","","","","", // 0-9
    "level","name","name","name","name","name","name","name","name","", // 10-19
    "faction","","","","","", // 20-26
    ""
};

static const char *AreaTableFormat = {
    "ixixixxxxx"
    "issssssssx"
    "xxxxxxx"
};

enum ItemDisplayInfoEnum
{
    ITEMDISPLAYINFO_ID = 0,
    ITEMDISPLAYINFO_MODEL_L = 1, // model file (left) (default)
    ITEMDISPLAYINFO_MODEL_R = 2, // model file (right) (for example for shoulders)
    ITEMDISPLAYINFO_NAME_L = 3, // (internal name?)
    ITEMDISPLAYINFO_NAME_R = 4, // (internal name?)
    ITEMDISPLAYINFO_ICON = 5, // icon filename
    ITEMDISPLAYINFO_GEOSET_1 = 6, // Geoset, which M2 submeshes this item will cover when equipped, depeding on type
    ITEMDISPLAYINFO_GEOSET_2 = 7, 
    ITEMDISPLAYINFO_GEOSET_3 = 8, 
    ITEMDISPLAYINFO_GEOSET_4 = 9, // some flag? 0, 1 or 2.
    ITEMDISPLAYINFO_FLAG1 = 10, // some flag? mostly 0, sometimes other values (for ex. polearms have 4081 ?!)
    ITEMDISPLAYINFO_ITEMGROUPSOUND = 11, // these is NOT the inventorytype...
    ITEMDISPLAYINFO_HELM1 = 12, // reference to HelmetGeosetVisData. only set if item is a helm
    ITEMDISPLAYINFO_HELM2 = 13, // ^ - (this ID points to a nearer specification which face parts are covered by a helm)
                             // -- the following fields store texture names for different body parts
    ITEMDISPLAYINFO_AU = 14, // arm upper
    ITEMDISPLAYINFO_AL = 15, // arm lower
    ITEMDISPLAYINFO_HA = 16, // hand
    ITEMDISPLAYINFO_TU = 17, // torso upper
    ITEMDISPLAYINFO_TL = 18, // torso lower
    ITEMDISPLAYINFO_LU = 19, // leg upper
    ITEMDISPLAYINFO_LL = 20, // leg lower
    ITEMDISPLAYINFO_FO = 21, // foot
    ITEMDISPLAYINFO_VISUAL = 22, // reference to ItemVisuals: specifies the ID of up to 5 glow effects

    ITEMDISPLAYINFO_END = 23
};

static const char *ItemDisplayInfoFieldNames[] = {
    "","model_l","model_r","tex_l","tex_r","icon","geo1","geo2","geo3","geo4", // 0-9
    "flag1","sound","helm1","helm2","AU","AL","HA","TU","TL","LU", // 10-19
    "LL","FO","unk", // 20-22
    ""
};

static const char *ItemDisplayInfoFormat = {
    "isssssiiii"
    "iiiissssss"
    "ssi"
};

enum CreatureModelDataEnum
{
    CREATUREMODELDATA_ID = 0,
    CREATUREMODELDATA_FILE = 2,
    // other fields unknown
    CREATUREMODELDATA_END = 24
};

static const char *CreatureModelDataFieldNames[] = {
    "","","file","","","","","","", // 0-9
    "","","","","","","","","", // 10-19
    "","","","","",
    ""
};

static const char *CreatureModelDataFormat = {
    "ixsxxxxxx" // 0-9
    "xxxxxxxxx" // 10-19
    "xxxxx"
};

enum CreatureDisplayInfoEnum
{
    CREATUREDISPLAYINFO_ID = 0, // <<-- this is the so-called DisplayID!!
    CREATUREDISPLAYINFO_MODEL = 1, // id from CreatureModelData
    CREATUREDISPLAYINFO_UNK = 2,
    CREATUREDISPLAYINFO_EXTRA = 3, // id from CreatureDisplayInfoExtra
    CREATUREDISPLAYINFO_SCALE = 4, // default scale, float
    CREATUREDISPLAYINFO_OPACITY = 5, // 0 = 100% tansparent, 255 = 100% solid
    CREATUREDISPLAYINFO_NAME1 = 6, // next 3 fields are some names, not always present
    CREATUREDISPLAYINFO_NAME2 = 7,
    CREATUREDISPLAYINFO_NAME3 = 8, 
    CREATUREDISPLAYINFO_NPCSOUNDS = 11, // id from NPCSounds
    CREATUREDISPLAYINFO_END = 21
};

static const char *CreatureDisplayInfoFieldNames[] = {
    "","model","unk","extra","scale","opacity","name1","name2","name3","", // 0-9
    "","sound","","","","","","","","", // 10-19
    "",
    "", "", ""
};

static const char *CreatureDisplayInfoFormat = {
    "iiiifisssx" // 0-9
    "xixxxxxxxx" // 10-19
    "xxx"
};

enum NPCSoundsEnum
{
    NPCSOUNDS_ID = 0,
    NPCSOUNDS_GREETINGS = 1,
    NPCSOUNDS_FAREWELLS = 2,
    NPCSOUNDS_PISSED = 3,
    NPCSOUNDS_UNK = 4, // always 0
    NPCSOUNDS_END = 5
};

static const char *NPCSoundsFieldNames[] = {
    "","greet","bye","pissed","",
    ""
};

static const char *NPCSoundsFormat = {
    "iiiix"
};

enum CharSectionsEnum
{
    CHARSECTIONS_ID = 0, // the sectionID, not sure if this is a reference from/to other fields
    CHARSECTIONS_RACE_ID = 1,
    CHARSECTIONS_GENDER = 2, // 0=male, 1=female
    CHARSECTIONS_TYPE = 3, // 0=base skin, 1=face, 2=facial traits, 3=hair, 4=underwear
    CHARSECTIONS_SECTION = 4, // depends, see table below
    CHARSECTIONS_COLOR = 5, // skin/hair color
    CHARSECTIONS_TEXTURE1 = 6, 
    CHARSECTIONS_TEXTURE2 = 7,
    CHARSECTIONS_TEXTURE3 = 8,
    CHARSECTIONS_IS_SPECIAL_NPC = 9, // 0=player, 1=special npc
    CHARSECTIONS_END
};
/*
Type 	 Description 	           Section 	        Color 	     Texture 1            Texture 2                    Texture 3
0 	     Base skin 	               -                Skin color   Skin texture 	      Fur texture (Tauren only) 	-
1 	     Face 	                   Face type        Skin color   Face lower texture   Face upper texture            -
2 	     Facial hair / decoration  Facial hair type Hair color 	 Face lower texture   Face upper texture            -
3 	     Hair 	                   Hair style 	    Hair color   Hair texture         Face lower texture           Face upper texture
4 	     Underwear 	               -                Skin color   Legs upper texture   Chest upper texture           -
The textures are all overlaid on the base skin texture, according to the alpha channel in the sub textures. The texture region positions and sizes are fixed.
*/

static const char *CharSectionsFieldNames[] = {
    "","race","gender","type","section","color","tex1","tex2","tex3","special", // 0-9
    ""
};

static const char *CharSectionsFormat = {
    "iiiiiisssi"
};


// GameObjectDisplayInfo

enum GameObjectDisplayInfoEnum
{
    GAMEOBJECTDISPLAYINFO_ID = 0, // <<-- this is the so-called DisplayID!!
    GAMEOBJECTDISPLAYINFO_MODEL = 1,
    GAMEOBJECTDISPLAYINFO_END = 18
};

static const char *GameObjectDisplayInfoFieldNames[] = {
    "","model","","","","","","","","", // 0-9
    "","","","","","","","" // 10-17
};

static const char *GameObjectDisplayInfoFormat = {
    "isxxxxxxxx" // 0-9
    "xxxxxxxx" // 10-17
};


// GameObjectDisplayInfo

enum ChrBaseInfoEnum
{
    CBI_RACE = 0, 
    CBI_CLASS = 1,
    CBI_END = 2
};




#endif
