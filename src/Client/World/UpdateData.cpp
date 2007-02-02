#include "common.h"
#include "ZCompressor.h"
#include "WorldSession.h"
#include "UpdateData.h"

void WorldSession::_HandleCompressedUpdateObjectOpcode(WorldPacket& recvPacket)
{
    printf("-> COMPRESSED_UPDATE_OBJECT, pktlen=%u\n",recvPacket.size());
    //recvPacket.hexlike();
    uint32 realsize;
    recvPacket >> realsize;
    ZCompressor z;
    z.append(recvPacket.contents() + sizeof(uint32),recvPacket.size() - sizeof(uint32));
    z.Compressed(true);
    z.RealSize(realsize);
    z.Inflate();
    printf("-> Uncompressed to %u bytes\n",z.size());
    if(z.Compressed())
    {
        log("ERROR: _HandleCompressedUpdateObjectOpcode(): Inflate() failed!");
        return;
    }
    WorldPacket wp;
    wp.SetOpcode(recvPacket.GetOpcode());
    wp.append(z.contents(),z.size());
 
    _HandleUpdateObjectOpcode(wp);
}

void WorldSession::_HandleUpdateObjectOpcode(WorldPacket& recvPacket)
{
     printf("-> UPDATE_OBJECT, pktlen=%u\n",recvPacket.size());
     //recvPacket.hexlike();
     uint8 utype;
     uint32 usize;
     uint64 uguid;
     //while(true)
     {
         recvPacket >> utype >> usize;
         switch(utype)
         {
            case UPDATETYPE_OUT_OF_RANGE_OBJECTS:
            for(uint16 i=0;i<usize;i++)
            {
                recvPacket >> uguid;
                printf("DEBUG: GUID "I64FMT" out of range\n",uguid);
                // TODO: delete object from known objects list
            }
            break;

            default:
                break;
         }
     }
}