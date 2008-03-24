
#include "WorldPacket.h"


uint64 WorldPacket::GetPackedGuid(void)
{
    uint8 mask;
    *this >> mask;
    uint64 guid=0;
    for(uint8 i=0;i<8;i++)
    {
        if(mask & (1<<i) )
        {
            *this >> ((uint8*)&guid)[i];
        }
    }
    return guid;
}

