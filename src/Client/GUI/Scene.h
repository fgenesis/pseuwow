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
    inline void SetState(SceneState sc) { _scenestate = sc; }
    inline SceneState GetState(void) { return _scenestate; }
    virtual void OnUpdate(s32);
    virtual void OnDraw(void);
    virtual void OnDelete(void);    
protected:

    PseuGUI *gui;
    irr::IrrlichtDevice *device;
    irr::video::IVideoDriver* driver;
    irr::scene::ISceneManager* smgr;
    irr::gui::IGUIEnvironment* guienv;
    SceneState _scenestate;
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
class MCameraOrbit;
class MyEventReceiver;
class MapMgr;
class WorldSession;

class SceneWorld : public Scene
{
public:
    SceneWorld(PseuGUI *gui);
    void OnDraw(void);
    void OnDelete(void);
    void OnUpdate(s32);
    void UpdateTerrain(void);
    void InitTerrain(void);

    WorldPosition GetWorldPosition(void);
    void SetWorldPosition(WorldPosition);

private:
    ShTlTerrainSceneNode *terrain;
    MCameraFPS *camera;
    MyEventReceiver *eventrecv;
    ZThread::FastMutex mutex;
    PseuGUI *gui;
    uint32 map_gridX, map_gridY;
    s32 mapsize, meshsize;
    f32 tilesize;
    WorldSession *wsession;
    MapMgr *mapmgr;
    IGUIStaticText *debugText;
    bool debugmode;
};



#endif