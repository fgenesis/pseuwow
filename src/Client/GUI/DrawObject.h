#ifndef DRAWOBJECT_H
#define DRAWOBJECT_H

#include "common.h"
#include "irrlicht/irrlicht.h"

class Object;
class PseuInstance;

class DrawObject
{
public:
    DrawObject(irr::IrrlichtDevice *device, Object*, PseuInstance *ins);
    ~DrawObject();
    void Draw(void); // call only in threadsafe environment!! (ensure the obj ptr is still valid!)
    void Unlink(void);
    // additionally, we dont use a GetObject() func - that would fuck things up if the object was already deleted.

private:
    void _Init(void);
    Object *_obj;
    bool _initialized : 1;
    irr::IrrlichtDevice *_device;
    irr::scene::ISceneManager *_smgr;
    irr::gui::IGUIEnvironment* _guienv;
    irr::scene::ISceneNode* cube;
    irr::scene::ITextSceneNode *text;
    PseuInstance *_instance;
    irr::core::vector3df rotation;

};

#endif
