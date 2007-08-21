#ifndef _WORLDSESSION_H
#define _WORLDSESSION_H

#include "common.h"
#include "PseuWoW.h"
#include "Network/SocketHandler.h"
#include "Player.h"
#include "Auth/AuthCrypt.h"
#include "SharedDefines.h"
#include "ObjMgr.h"
#include "CacheHandler.h"

class WorldSocket;
class WorldPacket;
class Channel;
class RealmSession;
struct OpcodeHandler;
class World;


class WorldSession
{
public:
    WorldSession(PseuInstance *i);
    ~WorldSession();
    void Init(void);

    PseuInstance *GetInstance(void) { return _instance; }
    SCPDatabaseMgr& GetDBMgr(void) { return GetInstance()->dbmgr; }

    void AddToPktQueue(WorldPacket *pkt);
    void Update(void);
    void Start(void);
    bool MustDie(void) { return _mustdie; }
    void SetMustDie(void) { _mustdie = true; }
    void SendWorldPacket(WorldPacket&);
    bool InWorld(void) { return _logged; }

    void SetTarget(uint64 guid);
    uint64 GetTarget(void) { return GetMyChar()->GetTarget(); }
    uint64 GetGuid(void) { return _myGUID; }
    Channel *GetChannels(void) { return _channels; }
    MyCharacter *GetMyChar(void) { ASSERT(_myGUID > 0); return (MyCharacter*)objmgr.GetObj(_myGUID); }
    World *GetWorld(void) { return _world; }


    // CMSGConstructor
    void SendChatMessage(uint32 type, uint32 lang, std::string msg, std::string to);
    void SendQueryPlayerName(uint64 guid);
    void SendPing(uint32);
    void SendEmote(uint32);
    void SendQueryItem(uint32, uint64);
    void SendSetSelection(uint64);
    void SendCastSpell(uint32 spellid, bool nocheck=false);

    PlayerNameCache plrNameCache;
    ObjMgr objmgr;

private:
    OpcodeHandler *_GetOpcodeHandlerTable(void) const;

    // Helpers
    void _OnEnterWorld(void); // = login
    void _OnLeaveWorld(void); // = logout
    void _DoTimedActions(void);
    
    // Opcode Handlers
    void _HandleAuthChallengeOpcode(WorldPacket& recvPacket);
    void _HandleAuthResponseOpcode(WorldPacket& recvPacket);
    void _HandleCharEnumOpcode(WorldPacket& recvPacket);
    void _HandleSetProficiencyOpcode(WorldPacket& recvPacket);
    void _HandleAccountDataMD5Opcode(WorldPacket& recvPacket);
    void _HandleMessageChatOpcode(WorldPacket& recvPacket);
    void _HandleNameQueryResponseOpcode(WorldPacket& recvPacket);
    void _HandleMovementOpcode(WorldPacket& recvPacket);
    void _HandlePongOpcode(WorldPacket& recvPacket);
    void _HandleTradeStatusOpcode(WorldPacket& recvPacket);
    void _HandleGroupInviteOpcode(WorldPacket& recvPacket);
	void _HandleTelePortAckOpcode(WorldPacket& recvPacket);
    void _HandleChannelNotifyOpcode(WorldPacket& recvPacket);
	void _HandleCastResultOpcode(WorldPacket& recvPacket);
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

	void _MovementUpdate(uint8 objtypeid, uint64 guid, WorldPacket& recvPacket); // Helper for _HandleUpdateObjectOpcode
    void _ValuesUpdate(uint64 uguid, WorldPacket& recvPacket); // ...
    void _QueryObjectInfo(uint64 guid);

    void _LoadCache(void);

    PseuInstance *_instance;
    WorldSocket *_socket;
    ZThread::LockedQueue<WorldPacket*,ZThread::FastMutex> pktQueue;
    bool _logged,_mustdie; // world status
    SocketHandler _sh; // handles the WorldSocket
    Channel *_channels;
    uint64 _myGUID;
    World *_world;
};

#endif