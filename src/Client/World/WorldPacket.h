#ifndef _WORLDPACKET_H
#define _WORLDPACKET_H

#include "SysDefs.h"
#include "ByteBuffer.h"

class WorldPacket : public ByteBuffer
{
public:
    WorldPacket() { ByteBuffer(10); _opcode=0; }
    WorldPacket(uint32 r) { reserve(r); _opcode=0; }
    WorldPacket(uint16 opcode, uint32 r) { _opcode=opcode; reserve(r); }
    WorldPacket(uint16 opcode) { _opcode=opcode; reserve(10); }
    inline void SetOpcode(uint16 opcode) { _opcode=opcode; }
    inline uint16 GetOpcode(void) { return _opcode; }
    uint64 GetPackedGuid(void);

private:
    uint16 _opcode;

};


#endif
