#ifndef _HELPERDEFS_H
#define _HELPERDEFS_H

#define GUID_HIPART(x)   (*(((uint16*)&(x))+3))
#define GUID_LOPART(x)   ((uint32)(uint64(x) & 0x00FFFFFF))
#define MAKE_GUID(l, h)  uint64( uint32(l) | ( uint64(h) << 32 ) )

#define CHAT_ITEM_BEGIN_STRING "|Hitem:"

#endif
