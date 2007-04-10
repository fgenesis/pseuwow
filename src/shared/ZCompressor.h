
#ifndef _ZCOMPRESSOR_H
#define _ZCOMPRESSOR_H

#include "ByteBuffer.h"


class ZCompressor : public ByteBuffer
{
public:
    ZCompressor();
    void Deflate(uint8 level=4);
    void Inflate(void);
    bool Compressed(void) { return _iscompressed; }
    void Compressed(bool b) { _iscompressed = b; }
    uint32 RealSize(void) { return _iscompressed ? _real_size : 0; }
    void RealSize(uint32 realsize) { _real_size=realsize; }
    void clear(void);


protected:
    bool _iscompressed;
    void _compress(void* dst, uint32 *dst_size, void* src, uint32 src_size, uint8 level=4);
    uint32 _real_size;



        


};


#endif
