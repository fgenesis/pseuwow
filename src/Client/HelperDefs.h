#ifndef _HELPERDEFS_H
#define _HELPERDEFS_H

#define GUID_HIPART(x)   (*(((uint32*)&(x))+1))
#define GUID_LOPART(x)   (*((uint32*)&(x)))
#define MAKE_GUID(l, h)  uint64( uint32(l) | ( uint64(h) << 32 ) )

#define CHAT_ITEM_BEGIN_STRING "|Hitem:"

#endif
