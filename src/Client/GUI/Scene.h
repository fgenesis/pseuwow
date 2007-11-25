#ifndef _SCENE_H
#define _SCENE_H

#include "irrlicht/irrlicht.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

class PseuGUI;

// base class
class Scene
{
    friend class PseuGUI;
public:
    Scene(PseuGUI *g);
    ~Scene();
    virtual void OnUpdate(f32);
    virtual void OnDraw(void);
    virtual void OnDelete(void);    
protected:

    PseuGUI *gui;
    irr::IrrlichtDevice *device;
    irr::video::IVideoDriver* driver;
    irr::scene::ISceneManager* smgr;
    irr::gui::IGUIEnvironment* guienv;
};

class SceneGuiStart : public Scene
{
public:
    SceneGuiStart(PseuGUI *gui);
    void OnDelete(void);
private:
    IGUIImage *irrlogo, *driverlogo;

};


class ShTlTerrainSceneNode;
class MCameraFPS;
class MyEventReceiver;

class SceneWorld : public Scene
{
public:
    SceneWorld(PseuGUI *gui);
    void OnDraw(void);
    void OnDelete(void);
    void OnUpdate(f32);
private:
    ShTlTerrainSceneNode *terrain;
    MCameraFPS *camera;
    MyEventReceiver *eventrecv;
    ZThread::FastMutex mutex;
};



#endif