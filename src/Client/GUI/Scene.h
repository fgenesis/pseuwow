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
    virtual void Draw(void);
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
    ~SceneGuiStart();
private:
    IGUIImage *irrlogo, *driverlogo;

};

class SceneWorld : public Scene
{
public:
    SceneWorld(PseuGUI *gui);
    ~SceneWorld();
    void Draw(void);
};



#endif