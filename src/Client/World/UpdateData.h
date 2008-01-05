#ifndef _UPDATEDATA_H
#define _UPDATEDATA_H

enum OBJECT_UPDATE_TYPE
{
	UPDATETYPE_VALUES               = 0,
	UPDATETYPE_MOVEMENT             = 1,
	UPDATETYPE_CREATE_OBJECT        = 2,
	UPDATETYPE_CREATE_OBJECT2       = 3,
	UPDATETYPE_OUT_OF_RANGE_OBJECTS = 4,
	UPDATETYPE_NEAR_OBJECTS         = 5
};

enum OBJECT_UPDATE_FLAGS
{
	UPDATEFLAG_SELF         = 0x01,
	UPDATEFLAG_TRANSPORT    = 0x02,
	UPDATEFLAG_FULLGUID     = 0x04,
	UPDATEFLAG_HIGHGUID     = 0x08,
	UPDATEFLAG_ALL          = 0x10,
	UPDATEFLAG_LIVING       = 0x20,
	UPDATEFLAG_HASPOSITION  = 0x40
};

// not sure about those flags, mangos hasnt a description for it either
enum FLAGS2_UPDATE_FLAGS
{
	FLAGS2_TRANSPORT = 0x200,
	FLAGS2_SPIRITHEALER = 0x10000000,
};

bool IsFloatField(uint8, uint32);

#endif