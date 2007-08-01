#ifndef DRAWOBJECT_H
#define DRAWOBJECT_H

#include "common.h"
#include "irrlicht/irrlicht.h"

class Object;

class DrawObject
{
public:
    DrawObject(irr::scene::ISceneManager*, Object*);
    ~DrawObject();
    void Draw(void); // call only in threadsafe environment!! (ensure the obj ptr is still valid!)
    // additionally, we dont use a GetObject() func - that would fuck things up if the object was already deleted.

private:
    Object *_obj;
    irr::scene::ISceneManager *_smgr;

};

#endif
