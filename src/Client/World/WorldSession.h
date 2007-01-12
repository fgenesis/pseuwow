#ifndef _WORLDSESSION_H
#define _WORLDSESSION_H

#include <deque>
#include "PseuWoW.h"
#include "Network/SocketHandler.h"
#include "common.h"
#include "Player.h"
#include "Auth/AuthCrypt.h"
#include "SharedDefines.h"

class WorldSocket;
class WorldPacket;
class PingerThread;

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


    // CMSGConstructor
    void SendChatMessage(uint32 type, uint32 lang, std::string msg, std::string to);
    void SendQueryPlayerName(uint64 guid);
    void SendPing(uint32);
    void SendEmote(uint32);

    PlayerNameCache plrNameCache;

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


    PseuInstance *_instance;
    WorldSocket *_socket;
    ZThread::LockedQueue<WorldPacket*,ZThread::FastMutex> pktQueue;
    bool _valid,_authed,_logged,_deleteme; // world status
    SocketHandler _sh; // handles the WorldSocket
    uint64 _targetGUID,_followGUID,_myGUID;
    
};

#endif