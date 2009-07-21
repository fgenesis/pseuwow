#ifndef _WORLDSESSION_H
#define _WORLDSESSION_H

#include <deque>
#include <bitset>

#include "common.h"
#include "PseuWoW.h"
#include "Network/SocketHandler.h"
#include "Player.h"
#include "Auth/AuthCrypt.h"
#include "SharedDefines.h"
#include "ObjMgr.h"
#include "CacheHandler.h"
#include "Opcodes.h"

class WorldSocket;
class WorldPacket;
class Channel;
class RealmSession;
struct OpcodeHandler;
class World;

struct WhoListEntry
{
    std::string name;
    std::string gname;
    uint32 level;
    uint32 classId;
    uint32 raceId;
    uint32 zoneId;
};

struct DelayedWorldPacket
{
    DelayedWorldPacket() { pkt = NULL; when = clock(); }
    DelayedWorldPacket(WorldPacket *p, uint32 ms) { pkt = p; when = ms + clock(); }
    WorldPacket *pkt;
    clock_t when;
};

// helper used for GUI
struct CharacterListExt
{
    PlayerEnum p;
    std::string zone;
    std::string class_;
    std::string race;
    std::string map_;
};


typedef std::vector<WhoListEntry> WhoList;
typedef std::vector<CharacterListExt> CharList;
typedef std::deque<DelayedWorldPacket> DelayedPacketQueue;

class WorldSession
{
    friend class Channel;

public:
    WorldSession(PseuInstance *i);
    ~WorldSession();
    void Init(void);

    inline PseuInstance *GetInstance(void) { return _instance; }
    inline SCPDatabaseMgr& GetDBMgr(void) { return GetInstance()->dbmgr; }

    void AddToPktQueue(WorldPacket *pkt);
    void Update(void);
    void Start(void);
    inline bool MustDie(void) { return _mustdie; }
    void SetMustDie(void);
    void SendWorldPacket(WorldPacket&);
    void AddSendWorldPacket(WorldPacket *pkt);
    void AddSendWorldPacket(WorldPacket& pkt);
    inline bool InWorld(void) { return _logged; }
    inline uint32 GetLagMS(void) { return _lag_ms; }

    void SetTarget(uint64 guid);
    inline uint64 GetTarget(void) { return GetMyChar() ? GetMyChar()->GetTarget() : 0; }
    inline uint64 GetGuid(void) { return _myGUID; }
    inline Channel *GetChannels(void) { return _channels; }
    inline MyCharacter *GetMyChar(void) { ASSERT(_myGUID > 0); return (MyCharacter*)objmgr.GetObj(_myGUID); }
    inline World *GetWorld(void) { return _world; }

    std::string GetOrRequestPlayerName(uint64);
    std::string DumpPacket(WorldPacket& pkt, int errpos = -1, const char *errstr = NULL);

    inline uint32 GetCharsCount(void) { return _charList.size(); }
    inline CharacterListExt& GetCharFromList(uint32 id) { return _charList[id]; }
    void EnterWorldWithCharacter(std::string);
    void PreloadDataBeforeEnterWorld(PlayerEnum&);


    // CMSGConstructor
    void SendChatMessage(uint32 type, uint32 lang, std::string msg, std::string to="");
    void SendQueryPlayerName(uint64 guid);
    void SendPing(uint32);
    void SendEmote(uint32);
    void SendQueryItem(uint32 id, uint64 guid = 0);
    void SendSetSelection(uint64);
    void SendCastSpell(uint32 spellid, bool nocheck=false);
    void SendWhoListRequest(uint32 minlvl=0, uint32 maxlvl=100, uint32 racemask=-1, uint32 classmask=-1, std::string name="", std::string guildname="", std::vector<uint32> *zonelist=NULL, std::vector<std::string> *strlist=NULL);
    void SendQueryCreature(uint32 entry, uint64 guid = 0);
    void SendQueryGameobject(uint32 entry, uint64 guid = 0);
    void SendCharCreate(std::string name, uint8 race, uint8 class_, uint8 gender=0, uint8 skin=0, uint8 face=0, uint8 hairstyle=0, uint8 haircolor=0, uint8 facial=0, uint8 outfit=0);

    void HandleWorldPacket(WorldPacket*);

    inline void DisableOpcode(uint16 opcode) { _disabledOpcodes[opcode] = true; }
    inline void EnableOpcode(uint16 opcode) { _disabledOpcodes[opcode] = false; }
    inline bool IsOpcodeDisabled(uint16 opcode) { return _disabledOpcodes[opcode]; }

    PlayerNameCache plrNameCache;
    ObjMgr objmgr;


private:

    OpcodeHandler *_GetOpcodeHandlerTable(void) const;

    // Helpers
    void _OnEnterWorld(void); // = login
    void _OnLeaveWorld(void); // = logout
    void _DoTimedActions(void);
    void _DelayWorldPacket(WorldPacket&, uint32);
    void _HandleDelayedPackets(void);

    // Opcode Handlers
    void _HandleAuthChallengeOpcode(WorldPacket& recvPacket);
    void _HandleAuthResponseOpcode(WorldPacket& recvPacket);
    void _HandleCharEnumOpcode(WorldPacket& recvPacket);
    void _HandleSetProficiencyOpcode(WorldPacket& recvPacket);
    void _HandleAccountDataMD5Opcode(WorldPacket& recvPacket);
    void _HandleMessageChatOpcode(WorldPacket& recvPacket);
    void _HandleNameQueryResponseOpcode(WorldPacket& recvPacket);
    void _HandleMovementOpcode(WorldPacket& recvPacket);
    void _HandleSetSpeedOpcode(WorldPacket& recvPacket);
    void _HandleForceSetSpeedOpcode(WorldPacket& recvPacket);
    void _HandlePongOpcode(WorldPacket& recvPacket);
    void _HandleTradeStatusOpcode(WorldPacket& recvPacket);
    void _HandleGroupInviteOpcode(WorldPacket& recvPacket);
	void _HandleTelePortAckOpcode(WorldPacket& recvPacket);
    void _HandleChannelNotifyOpcode(WorldPacket& recvPacket);
	void _HandleCastResultOpcode(WorldPacket& recvPacket);
    void _HandleCastSuccessOpcode(WorldPacket& recvPacket);
    void _HandleCompressedUpdateObjectOpcode(WorldPacket& recvPacket);
    void _HandleUpdateObjectOpcode(WorldPacket& recvPacket);
    void _HandleItemQuerySingleResponseOpcode(WorldPacket& recvPacket);
    void _HandleDestroyObjectOpcode(WorldPacket& recvPacket);
    void _HandleInitialSpellsOpcode(WorldPacket& recvPacket);
    void _HandleLearnedSpellOpcode(WorldPacket& recvPacket);	
    void _HandleRemovedSpellOpcode(WorldPacket& recvPacket);
	void _HandleChannelListOpcode(WorldPacket& recvPacket);
    void _HandleEmoteOpcode(WorldPacket& recvPacket);
    void _HandleTextEmoteOpcode(WorldPacket& recvPacket);
    void _HandleNewWorldOpcode(WorldPacket& recvPacket);
    void _HandleLoginVerifyWorldOpcode(WorldPacket& recvPacket);
    void _HandleMotdOpcode(WorldPacket& recvPacket);
    void _HandleNotificationOpcode(WorldPacket& recvPacket);
    void _HandleWhoOpcode(WorldPacket& recvPacket);
    void _HandleCreatureQueryResponseOpcode(WorldPacket& recvPacket);
    void _HandleGameobjectQueryResponseOpcode(WorldPacket& recvPacket);
    void _HandleCharCreateOpcode(WorldPacket& recvPacket);
    void _HandleMonsterMoveOpcode(WorldPacket& recvPacket);

    // helper functions to keep SMSG_(COMPRESSED_)UPDATE_OBJECT easy to handle
	void _MovementUpdate(uint8 objtypeid, uint64 guid, WorldPacket& recvPacket); // Helper for _HandleUpdateObjectOpcode
    void _ValuesUpdate(uint64 uguid, WorldPacket& recvPacket); // ...
    void _QueryObjectInfo(uint64 guid);

    void _LoadCache(void);

    PseuInstance *_instance;
    WorldSocket *_socket;
    ZThread::LockedQueue<WorldPacket*,ZThread::FastMutex> pktQueue, sendPktQueue;
    DelayedPacketQueue delayedPktQueue;
    bool _logged,_mustdie; // world status
    SocketHandler _sh; // handles the WorldSocket
    Channel *_channels;
    uint64 _myGUID;
    World *_world;
    WhoList _whoList;
    CharList _charList;
    uint32 _lag_ms;
    std::bitset<MAX_OPCODE_ID> _disabledOpcodes;

};

#endif
