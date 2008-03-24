#ifndef MD5HASH_H
#define MD5HASH_H

#define MD5_DIGEST_LENGTH 16

#include "Auth/md5.h"

class MD5Hash
{
public:
    MD5Hash()
    {
        md5_init(&_state);
        memset(_digest,0,MD5_DIGEST_LENGTH);
    }

    
    void Update(uint8 *buf,uint32 len)
    {
        md5_append(&_state,buf,len);
    }

    void Update(std::string s)
    {
        Update((uint8*)s.c_str(),s.length());
    }

    void Finalize(void)
    {
        md5_finish(&_state,_digest);
    }

    uint8 *GetDigest(void)
    {
        return _digest;
    }

    uint8 GetLength(void) { return MD5_DIGEST_LENGTH; }

private:
    md5_state_t _state;
    uint8 _digest[MD5_DIGEST_LENGTH];
};

#endif
