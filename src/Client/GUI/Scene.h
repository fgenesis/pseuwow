#ifndef _SCENE_H
#define _SCENE_H

#include "irrlicht/irrlicht.h"
#include "SceneData.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


class PseuGUI;
class CCursorController;

// base class
class Scene
{
    friend class PseuGUI;
public:
    Scene(PseuGUI *g);
    ~Scene();
    core::stringw GetStringFromDB(u32 index, u32 entry);
    inline void SetState(SceneState sc) { _scenestate = sc; }
    inline SceneState GetState(void) { return _scenestate; }
    virtual void OnUpdate(s32);
    virtual void OnDraw(void);
    virtual void OnDrawBegin(void);
    virtual void OnDelete(void);
    virtual video::SColor GetBackgroundColor(void);
    virtual void SetData(uint32 index, uint32 value) { scenedata[index] = value; }
protected:
    PseuInstance *instance;
    PseuGUI *gui;
    irr::IrrlichtDevice *device;
    irr::video::IVideoDriver* driver;
    irr::scene::ISceneManager* smgr;
    irr::gui::IGUIEnvironment* guienv;
    CCursorController *cursor;
    SceneState _scenestate;
    uint32 scenedata[SCENEDATA_SIZE]; // generic storage for anything the PseuInstance thread wants to tell us
    SCPDatabase *textdb;
};

class SceneGuiStart : public Scene
{
public:
    SceneGuiStart(PseuGUI *gui);
    void OnDelete(void);
private:
    IGUIImage *irrlogo, *driverlogo;

};

class GUIEventReceiver;

class SceneLogin : public Scene
{
public:
    SceneLogin(PseuGUI *gui);
    void OnUpdate(s32);
    void OnDelete(void);

private:
    gui::IGUIElement* root;
    IGUIImage *irrlogo, *background;
    GUIEventReceiver *eventrecv;
    PseuGUI* _gui;
    gui::IGUIElement *msgbox;
    uint32 msgbox_textid;
};

class ShTlTerrainSceneNode;
class MCameraFPS;
class MCameraOrbit;
class MyEventReceiver;
class MapMgr;
class WorldSession;

class SceneWorld : public Scene
{
    struct SceneNodeWithGridPos
    {
        scene::ISceneNode *scenenode;
        uint32 gx,gy;
    };

public:
    SceneWorld(PseuGUI *gui);
    void OnDraw(void);
    void OnDrawBegin(void);
    void OnDelete(void);
    void OnUpdate(s32);
    void UpdateTerrain(void);
    void InitTerrain(void);
    void RelocateCamera(void);
    void UpdateDoodads(void);
    video::SColor GetBackgroundColor(void);

    WorldPosition GetWorldPosition(void);

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
    World *world;
    MapMgr *mapmgr;
    IGUIStaticText *debugText;
    bool debugmode;
    std::map<uint32,SceneNodeWithGridPos> _doodads;
    scene::ISceneNode *sky;
    scene::ISceneNode *selectedNode, *oldSelectedNode, *focusedNode, *oldFocusedNode;
    video::SColor envBasicColor;
};



#endif
