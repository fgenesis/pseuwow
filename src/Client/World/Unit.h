#ifndef _UNIT_H
#define _UNIT_H

#include "Object.h"

class Unit : public WorldObject
{
public:
    Unit();
    void Create(uint64);
    uint8 GetGender(void);
private:

};


#endif