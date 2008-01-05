#ifndef MEMORYDATAHOLDER_H
#define MEMORYDATAHOLDER_H

#include "common.h"

namespace MemoryDataHolder
{
    typedef void (*callback_func)(void*,bool);


    uint8 *GetFile(std::string&,bool,callback_func,void*);
};

#endif
