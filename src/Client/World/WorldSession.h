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



struct OpcodeHandler
{
    uint16 opcode;
    void (WorldSession::*handler)(WorldPacket& recvPacket);
};


class WorldSession
{
public:
    WorldSession(PseuInstance *i);
    ~WorldSession();
    void Init(void);

    PseuInstance *GetInstance(void) { return _instance; }

    OpcodeHandler *_GetOpcodeHandlerTable(void) const;

    void SetSocket(WorldSocket *sock);

    void AddToPktQueue(WorldPacket *pkt);
    void Connect(std::string addr,uint16 port);
    void Update(void);
    void Start(void);
    bool IsValid(void) { return _valid; }
    bool DeleteMe(void);
    void SendWorldPacket(WorldPacket&);

    void SetTarget(uint64 guid);
    uint64 GetTarget(void) { return _targetGUID; }
    void SetFollowTarget(uint64 guid);
    uint64 GetFollowTarget(void) { return _followGUID; }
    uint64 GetGuid(void) { return _myGUID; }
    Channel *GetChannels(void) { return _channels; }
    MyCharacter *GetMyChar(void) { ASSERT(_myGUID > 0); return (MyCharacter*)objmgr.GetObj(_myGUID); }


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

	void _MovementUpdate(uint8 objtypeid, uint64 guid, WorldPacket& recvPacket); // Helper for _HandleUpdateObjectOpcode
    void _ValuesUpdate(uint64 uguid, WorldPacket& recvPacket); // ...
    void _QueryObjectInfo(uint64 guid);

    PseuInstance *_instance;
    WorldSocket *_socket;
    ZThread::LockedQueue<WorldPacket*,ZThread::FastMutex> pktQueue;
    bool _valid,_authed,_logged,_deleteme; // world status
    SocketHandler _sh; // handles the WorldSocket
    Channel *_channels;
    uint64 _targetGUID,_followGUID,_myGUID;
};

#endif