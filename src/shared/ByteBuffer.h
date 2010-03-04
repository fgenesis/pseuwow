/* 
 * Copyright (C) 2005,2006 MaNGOS <http://www.mangosproject.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _BYTEBUFFER_H
#define _BYTEBUFFER_H

#include <vector>
#include <list>
#include <map>
#include <string>
#if defined( __GNUC__ ) && (__GNUC__ * 10000 + __GNUC_MINOR__ * 100)>=40300
  #include <cstring>
  #include <stdio.h>
#endif

class ByteBufferException
{
public:
    ByteBufferException(const char *act, uint32 rp, uint32 wp, uint32 rs, uint32 cs)
    {
        action = act;
        rpos = rp;
        wpos = wp;
        readsize = rs;
        cursize = cs;
    }
    uint32 rpos, wpos, readsize, cursize;
    const char *action;
};

class ByteBuffer
{
    public:
        class error
        {
        };

        const static size_t DEFAULT_SIZE = 0xFF;

        ByteBuffer(): _rpos(0), _wpos(0)
        {
            _storage.reserve(DEFAULT_SIZE);
        }
        ByteBuffer(size_t res): _rpos(0), _wpos(0)
        {
            _storage.reserve(res);
        }
        ByteBuffer(const ByteBuffer &buf): _rpos(buf._rpos), _wpos(buf._wpos), _storage(buf._storage) { }

        void clear()
        {
            _storage.clear();
            _rpos = _wpos = 0;
        }

        template <typename T> void append(T value)
        {
            append((uint8 *)&value, sizeof(value));
        }
        template <typename T> void put(size_t pos,T value)
        {
            put(pos,(uint8 *)&value,sizeof(value));
        }

        ByteBuffer &operator<<(bool value)
        {
            append<char>((char)value);
            return *this;
        }
        ByteBuffer &operator<<(uint8 value)
        {
            append<uint8>(value);
            return *this;
        }
        ByteBuffer &operator<<(uint16 value)
        {
            append<uint16>(value);
            return *this;
        }
        ByteBuffer &operator<<(int32 value)
        {
            append<int32>(value);
            return *this;
        }
        ByteBuffer &operator<<(uint32 value)
        {
            append<uint32>(value);
            return *this;
        }
        ByteBuffer &operator<<(uint64 value)
        {
            append<uint64>(value);
            return *this;
        }
        ByteBuffer &operator<<(float value)
        {
            append<float>(value);
            return *this;
        }
        ByteBuffer &operator<<(double value)
        {
            append<double>(value);
            return *this;
        }
        ByteBuffer &operator<<(const std::string &value)
        {
            append((uint8 *)value.c_str(), value.length());
            append((uint8)0);
            return *this;
        }
        ByteBuffer &operator<<(const char *str)
        {
            append((uint8 *)str, str ? strlen(str) : 0);
            append((uint8)0);
            return *this;
        }

        ByteBuffer &operator>>(bool &value)
        {
            value = read<char>() > 0 ? true : false;
            return *this;
        }
        ByteBuffer &operator>>(uint8 &value)
        {
            value = read<uint8>();
            return *this;
        }
        ByteBuffer &operator>>(uint16 &value)
        {
            value = read<uint16>();
            return *this;
        }
        ByteBuffer &operator>>(int32 &value)
        {
            value = read<int32>();
            return *this;
        }
        ByteBuffer &operator>>(uint32 &value)
        {
            value = read<uint32>();
            return *this;
        }
        ByteBuffer &operator>>(uint64 &value)
        {
            value = read<uint64>();
            return *this;
        }
        ByteBuffer &operator>>(float &value)
        {
            value = read<float>();
            return *this;
        }
        ByteBuffer &operator>>(double &value)
        {
            value = read<double>();
            return *this;
        }
        ByteBuffer &operator>>(std::string& value)
        {
            value.clear();
            while (true)
            {
                char c=read<char>();
                if (c==0)
                    break;
                value+=c;
            }
            return *this;
        }

        uint8 operator[](size_t pos)
        {
            return read<uint8>(pos);
        }

        size_t rpos()
        {
            return _rpos;
        };

        size_t rpos(size_t rpos)
        {
            _rpos = rpos < _storage.capacity() ? rpos : _storage.capacity();
            return _rpos;
        };

        size_t wpos()
        {
            return _wpos;
        }

        size_t wpos(size_t wpos)
        {
            _wpos = wpos < _storage.capacity() ? wpos : _storage.capacity();
            return _wpos;
        }

        template <typename T> T read()
        {
            T r=read<T>(_rpos);
            _rpos += sizeof(T);
            return r;
        };
        template <typename T> T read(size_t pos) const
        {
            if(pos + sizeof(T) > size())
                throw ByteBufferException("read", pos, _wpos, sizeof(T), size());
            return *((T*)&_storage[pos]);
        }

        void read(uint8 *dest, size_t len)
        {
            if (_rpos + len <= size())
            {
                memcpy(dest, &_storage[_rpos], len);
            }
            else
            {
                throw ByteBufferException("read-into", _rpos, _wpos, len, size());
            }
            _rpos += len;
        }

        const uint8 *contents() const { return &_storage[0]; };

        inline size_t size() const { return _storage.size(); };

        void resize(size_t newsize)
        {
            _storage.resize(newsize);
            _rpos = 0;
            _wpos = size();
        };
        void reserve(size_t ressize)
        {
            if (ressize > size()) _storage.reserve(ressize);
        };

        void append(const std::string& str)
        {
            append((const uint8 *)str.c_str(),str.size() + 1);
        }
        void append(const char *src, size_t cnt)
        {
            return append((const uint8 *)src, cnt);
        }
        void append(const uint8 *src, size_t cnt)
        {
            if (!cnt) return;
            if (_storage.size() < _wpos + cnt)
                _storage.resize(_wpos + cnt);
            memcpy(&_storage[_wpos], src, cnt);
            _wpos += cnt;
        }
        void append(const ByteBuffer& buffer)
        {
            if(buffer.size()) append(buffer.contents(),buffer.size());
        }

        void appendPackGUID(uint64 guid)
        {
            if (_storage.size() < _wpos + sizeof(guid) + 1)
                _storage.resize(_wpos + sizeof(guid) + 1);

            size_t mask_position = wpos();
            *this << uint8(0);
            for(uint8 i = 0; i < 8; ++i)
            {
                if(guid & 0xFF)
                {
                    _storage[mask_position] |= uint8(1 << i);
                    *this << uint8(guid & 0xFF);
                }

                guid >>= 8;
            }
        }
        
        void put(size_t pos, const uint8 *src, size_t cnt)
        {
            memcpy(&_storage[pos], src, cnt);
        }
        void print_storage()
        {
            printf("STORAGE_SIZE: %u\n", size() );
            for(uint32 i = 0; i < size(); i++)
                printf("%u - ", read<uint8>(i) );
            printf("\n");
        }

        void textlike()
        {
            printf("STORAGE_SIZE: %u\n", size() );
            for(uint32 i = 0; i < size(); i++)
                printf("%c", read<uint8>(i) );
            printf("\n");
        }

        void hexlike()
        {
            uint32 j = 1, k = 1;
            printf("STORAGE_SIZE: %u\n", size() );
            for(uint32 i = 0; i < size(); i++)
            {
                if ((i == (j*8)) && ((i != (k*16))))
                {
                    if (read<uint8>(i) < 0x0F)
                    {
                        printf("| 0%X ", read<uint8>(i) );
                    }
                    else
                    {
                        printf("| %X ", read<uint8>(i) );
                    }

                    j++;
                }
                else if (i == (k*16))
                {
                    if (read<uint8>(i) < 0x0F)
                    {
                        printf("\n0%X ", read<uint8>(i) );
                    }
                    else
                    {
                        printf("\n%X ", read<uint8>(i) );
                    }

                    k++;
                    j++;
                }
                else
                {
                    if (read<uint8>(i) < 0x0F)
                    {
                        printf("0%X ", read<uint8>(i) );
                    }
                    else
                    {
                        printf("%X ", read<uint8>(i) );
                    }
                }
            }
            printf("\n");

        }
        
        void print(void)
        {
            uint32 line = 1;
            uint32 countpos = 0;
            printf("STORAGE_SIZE: %u\n", size() );
            printf("|------------------------------------------------|----------------|\r\n");
            printf("|00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F |0123456789ABCDEF|\r\n");
            printf("|------------------------------------------------|----------------|\r\n");
            if (size() > 0)
            {
                printf("|");
                for (uint32 count = 0; count < size(); count++)
                {
                    if (countpos == 16)
                    {
                        countpos = 0;
                        printf("|");
                        for (uint32 a = count-16; a < count; a++)
                        {
                            if ((read<uint8>(a) < 32) || (read<uint8>(a) > 126))
                                printf(".");
                            else
                                printf("%c", read<uint8>(a));
                        }
                        printf("|\r\n");
                        line++;
                        printf("|");
                    }
                    printf("%02x ", read<uint8>(count));

                    // Fix to parse packets with length < OR = to 16 bytes.
                    if (count+1 == size() && size() <= 16)
                    {
                        for (uint32 b = countpos+1; b < 16; b++)
                            printf("   ");

                        printf("|");

                        for (uint32 a = 0; a < size(); a++)
                        {
                            if ((read<uint8>(a) < 32) || (read<uint8>(a) > 126))
                                printf(".");
                            else
                                printf("%c", read<uint8>(a));
                        }

                        for (uint32 c = count; c < 15; c++)
                            printf(" ");

                        printf("|\r\n");
                    }

                    // Fix to parse the last line of the packets when the length is > 16 and its in the last line
                    if (count+1 == size() && size() > 16)
                    {
                        for (uint32 b = countpos+1; b < 16; b++)
                            printf("   ");

                        printf("|");
                        uint16 print = 0;

                        for (uint32 a = line*16 - 16; a < size(); a++)
                        {
                            if ((read<uint8>(a) < 32) || (read<uint8>(a) > 126))
                                printf(".");
                            else
                                printf("%c", read<uint8>(a));
                            print++;
                        }

                        for (uint32 c = print; c < 16; c++)
                            printf(" ");

                        printf("|\r\n");
                    }
                    countpos++;
                }
            }
            printf("-------------------------------------------------------------------\r\n\r\n");
        }

    protected:

        size_t _rpos, _wpos;
        std::vector<uint8> _storage;
};

template <typename T> ByteBuffer &operator<<(ByteBuffer &b, std::vector<T> v)
{
    b << (uint32)v.size();
    for (typename std::vector<T>::iterator i = v.begin(); i != v.end(); i++)
    {
        b << *i;
    }
    return b;
}

template <typename T> ByteBuffer &operator>>(ByteBuffer &b, std::vector<T> &v)
{
    uint32 vsize;
    b >> vsize;
    v.clear();
    while(vsize--)
    {
        T t;
        b >> t;
        v.push_back(t);
    }
    return b;
}

template <typename T> ByteBuffer &operator<<(ByteBuffer &b, std::list<T> v)
{
    b << (uint32)v.size();
    for (typename std::list<T>::iterator i = v.begin(); i != v.end(); i++)
    {
        b << *i;
    }
    return b;
}

template <typename T> ByteBuffer &operator>>(ByteBuffer &b, std::list<T> &v)
{
    uint32 vsize;
    b >> vsize;
    v.clear();
    while(vsize--)
    {
        T t;
        b >> t;
        v.push_back(t);
    }
    return b;
}

template <typename K, typename V> ByteBuffer &operator<<(ByteBuffer &b, std::map<K, V> &m)
{
    b << (uint32)m.size();
    for (typename std::map<K, V>::iterator i = m.begin(); i != m.end(); i++)
    {
        b << i->first << i->second;
    }
    return b;
}

template <typename K, typename V> ByteBuffer &operator>>(ByteBuffer &b, std::map<K, V> &m)
{
    uint32 msize;
    b >> msize;
    m.clear();
    while(msize--)
    {
        K k;
        V v;
        b >> k >> v;
        m.insert(make_pair(k, v));
    }
    return b;
}

#endif
