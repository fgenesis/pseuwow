#ifndef _GAMEOBJECT_H
#define _GAMEOBJECT_H

#include "Object.h"

#define GAMEOBJECT_DATA_FIELDS 24

struct GameobjectTemplate
{
    uint32 entry;
    uint32 type;
    uint32 displayId;
    std::string name;
    std::string castBarCaption;
    uint32 faction;
    uint32 flags;
    float size;
    union                                                   // different GO types have different data field
    {
        //0 GAMEOBJECT_TYPE_DOOR
        struct
        {
            uint32 _data0;
            uint32 lockId;                                  //1
            uint16 _data2lo;                                //2 lower part of data2, unknown
            uint16 autoCloseTime;                           //2 (unit16)
        } door;
        //1 GAMEOBJECT_TYPE_BUTTON
        struct
        {
            uint32 _data0;
            uint32 lockId;                                  //1
            uint16 _data2lo;                                //2 lower part of data2, unknown
            uint16 autoCloseTime;                           //2 (unit16)
            uint32 _data3;
            uint32 isBattlegroundObject;                    //4
        } button;
        //3 GAMEOBJECT_TYPE_CHEST
        struct
        {
            uint32 lockId;                                  //0
            uint32 lootId;                                  //1
            uint32 _data2[2];
            uint32 minSuccessOpens;                         //4
            uint32 maxSuccessOpens;                         //5
            uint32 eventId;                                 //6
            uint32 linkedTrapId;                            //7
            uint32 questId;                                 //8 not used currently but store quest required for GO activation for player
            uint32 _data9[5];
            uint32 _data14;                                 //14 something == trap.data12 == goober.data14 (openText?)
        } chest;
        //6 GAMEOBJECT_TYPE_TRAP
        struct
        {
            uint32 _data0;                                  //0 lockid???
            uint32 _data1;
            uint32 radius;                                  //2 radius for trap activation
            uint32 spellId;                                 //3
            uint32 isNeedDespawn;                           //4 (if >0)
            uint32 _data5;                                  //5
            uint32 _data6[6];
            uint32 _data12;                                 //12 something == chest.data14 == goober.data14 (openText?)
        } trap;
        //7 GAMEOBJECT_TYPE_CHAIR
        struct
        {
            uint32 slots;                                   //0 not used currently
            uint32 height;                                  //1
        } chair;
        //8 GAMEOBJECT_TYPE_SPELL_FOCUS
        struct
        {
            uint32 focusId;                                 //0
            uint32 dist;                                    //1
            uint32 linkedTrapId;                            //2
        } spellFocus;
        //10 GAMEOBJECT_TYPE_GOOBER
        struct
        {
            uint32 _data0;                                  //0 lockid ???
            uint32 questId;                                 //1
            uint32 eventId;                                 //2
            uint32 _data3[4];
            uint32 pageId;                                  //7
            uint32 _data8[2];
            uint32 spellId;                                 //10
            uint32 _data11;
            uint32 linkedTrapId;                            //12
            uint32 _data13;
            uint32 _data14;                                 //14 something == trap.data12 == chest.data14 (openText?)
            uint32 _data15;
            uint32 isBattlegroundObject;                    //16
        } goober;
        //13 GAMEOBJECT_TYPE_CAMERA
        struct
        {
            uint32 _data0;
            uint32 cinematicId;                             //1
        } camera;
        //15 GAMEOBJECT_TYPE_MO_TRANSPORT
        struct
        {
            uint32 taxiPathId;                              //0
        } moTransport;
        //17 GAMEOBJECT_TYPE_FISHINGNODE
        struct
        {
            uint32 _data0;
            uint32 lootId;                                  //1
        } fishnode;
        //18 GAMEOBJECT_TYPE_SUMMONING_RITUAL
        struct
        {
            uint32 reqParticipants;                         //0
            uint32 spellId;                                 //1
        } summoningRitual;
        //22 GAMEOBJECT_TYPE_SPELLCASTER
        struct
        {
            uint32 spellId;                                 //0
            uint32 charges;                                 //1
            uint32 partyOnly;                               //2
            uint32 spellId2;                                //3 (is it used in this way?)
        } spellcaster;
        //23 GAMEOBJECT_TYPE_MEETINGSTONE
        struct
        {
            uint32 minLevel;                                //0
            uint32 maxLevel;                                //1
        } meetingstone;
        //25 GAMEOBJECT_TYPE_FISHINGHOLE                    // not implemented yet
        struct
        {
            uint32 radius;                                  //0 how close bobber must land for sending loot
            uint32 lootId;                                  //1
            uint32 minSuccessOpens;                         //2
            uint32 maxSuccessOpens;                         //3
            uint32 lockId;                                  //4 possibly 1628 for all?
        } fishinghole;

        // not use for specific field access (only for output with loop by all filed), also this determinate max union size
        struct                                              // GAMEOBJECT_TYPE_SPELLCASTER
        {
            uint32 data[GAMEOBJECT_DATA_FIELDS];
        } raw;
    };
};

class GameObject : public WorldObject
{
public:
    GameObject();
    void Create(uint64);

private:

};

#endif
