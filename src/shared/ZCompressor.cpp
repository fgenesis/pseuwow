#include "common.h"
#if PLATFORM == PLATFORM_UNIX
#include "zlib.h"
#else
#include "zlib/zlib.h"
#endif
#include "ZCompressor.h"

ZCompressor::ZCompressor()
{
    _iscompressed=false;
    _real_size=0;
}

void ZCompressor::_compress(void* dst, uint32 *dst_size, void* src, uint32 src_size, uint8 level)
{
    z_stream c_stream;

    c_stream.zalloc = (alloc_func)Z_NULL;
    c_stream.zfree = (free_func)Z_NULL;
    c_stream.opaque = (voidpf)Z_NULL;

    // default Z_BEST_SPEED (1)
    if (Z_OK != deflateInit(&c_stream, level))
    {
        logdebug("ZLIB: Can't compress (zlib: deflateInit).\n");
        *dst_size = 0;
        return;
    }

    c_stream.next_out = (Bytef*)dst;
    c_stream.avail_out = *dst_size;
    c_stream.next_in = (Bytef*)src;
    c_stream.avail_in = (uInt)src_size;

    if (Z_OK != deflate(&c_stream, Z_NO_FLUSH))
    {
        logdebug("ZLIB: Can't compress (zlib: deflate)\n");
        *dst_size = 0;
        return;
    }

    if (c_stream.avail_in != 0)
    {
        logdebug("Can't compress (zlib: deflate not greedy)\n");
        *dst_size = 0;
        return;
    }

    if (Z_STREAM_END != deflate(&c_stream, Z_FINISH))
    {
        logdebug("Can't compress (zlib: deflate should report Z_STREAM_END)\n");
        *dst_size = 0;
        return;
    }

    if (Z_OK != deflateEnd(&c_stream))
    {
        logdebug("Can't compress (zlib: deflateEnd)\n");
        *dst_size = 0;
        return;
    }

    *dst_size = c_stream.total_out;
}


void ZCompressor::Deflate(uint8 level)
{
    if( _iscompressed || (!size()) || level>9 )
        return;

    char *buf;
    buf=new char[size()+8];

    uint32 newsize=size(),oldsize=size();
    reserve(size()+8);

    _compress((void*)buf, &newsize, (void*)contents(),size(),level);

    if(!newsize)
        return;

    resize(newsize);
    rpos(0);
    wpos(0);
    append(buf,newsize);
    delete [] buf;

    _iscompressed=true;

    _real_size=oldsize;
}

void ZCompressor::Inflate(void)
{
    if( (!_iscompressed) || (!_real_size) || (!size()))
        return;

    uLongf origsize=_real_size;
    int8 result;
    uint8 *target=new uint8[_real_size];
    wpos(0);
    rpos(0);
    result = uncompress(target, &origsize, (uint8*)contents(), size());
    if( result!=Z_OK || origsize!=_real_size)
    {
        logerror("ZCompressor: Inflate error! result=%d cursize=%u origsize=%u realsize=%u\n",result,size(),origsize,_real_size);
        delete [] target;
        return;
    }
    clear();
    append(target,origsize);
    delete [] target;
    _real_size=0;
    _iscompressed=false;

}

void ZCompressor::clear(void)
{
    ByteBuffer::clear();
    _real_size=0;
    _iscompressed=false;
}
    
