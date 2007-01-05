


#ifndef _REALMSOCKET_H
#define _REALMSOCKET_H

#include "Network/ResolvSocket.h"

class RealmSocket : public TcpSocket
{
public:
    RealmSocket(SocketHandler &h);
	~RealmSocket();
	PseuInstance *GetInstance(void);
    bool IsValid(void);
    void SetInstance(PseuInstance*);

    void SetHost(std::string);
    void SetPort(uint16);

    
	
	void Start(void);
	void Stop(void);
    void Update(void);
    void SendLogonChallenge(void);
	
    // dont use from outside!!
    void _HandleRealmList(void);
    void _HandleLogonProof(void);
    void _HandleLogonChallenge(void);

    void OnRead(void);
    //void OnAccept(void);
    void OnConnect(void);
    void RealmSocket::OnConnectFailed(void);


private:
    
    

    uint16 _rport;
    std::string _rhost;
	uint8 _m2[20];
    PseuInstance *_instance;
    BigNumber _key;
    bool _valid;
    ByteBuffer _data;


};
	
	
	
	




#endif