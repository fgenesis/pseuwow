#ifndef PSEUGUI_H
#define PSEUGUI_H

#include "irrlicht/irrlicht.h"
#include "irrklang/irrKlang.h"
#include "SceneData.h"
#include "DrawObjMgr.h"
#include "World.h"

class PseuGUI;
class Object;
class PseuInstance;
class Scene;

enum SceneState
{
    SCENESTATE_NULL = 0,
    SCENESTATE_GUISTART,
    SCENESTATE_LOGINSCREEN,
    SCENESTATE_REALMSELECT, // actually both realm and char select will call the same scene, but with slightly different data
    SCENESTATE_CHARSELECT,
    SCENESTATE_LOADING,
    SCENESTATE_WORLD,
    SCENESTATE_NOSCENE = 0xFFFFFFFF
};

enum DriverIDs
{
    NULLDEVICE = 0,
    SOFTWARE = 1,
    BURNINGSVIDEO = 2,
    OPENGL = 3,
    DIRECTX8 = 4,
    DIRECTX9 = 5,
};

#define MOUSE_SENSIVITY 0.5f
#define ANGLE_STEP (M_PI/180.0f)
#define DEG_TO_RAD(x) ((x)*ANGLE_STEP)
#define RAD_TO_DEG(x) ((x)/ANGLE_STEP)
#define RAD_FIX(x) ( (x)>(2*M_PI) ? ((x)-(2*M_PI)) : ( ((x)<0) ? ((x)+(2*M_PI)) : (x) ) )
#define DEG_FIX(x) ( (x)>360 ? ((x)-360) : ( ((x)<0) ? ((x)+360) : (x) ) )
#define IRR_TO_O(x) RAD_FIX(M_PI-DEG_TO_RAD(x)) // convert World orientation (rad) into irrlicht rotation (deg)
#define O_TO_IRR(x) DEG_FIX(180-RAD_TO_DEG(x)) // ... and the other way around

inline irr::core::vector3df WPToIrr(WorldPosition wp)
{
    return irr::core::vector3df(-wp.x, wp.z, -wp.y);
}

inline WorldPosition IrrToWP(irr::core::vector3df v, float o_rad)
{
    return WorldPosition(-v.X, -v.Z, v.Y, IRR_TO_O(o_rad)); // rotate by 90° and fix value
}


class PseuGUIRunnable : public ZThread::Runnable
{
public:
    PseuGUIRunnable();
    ~PseuGUIRunnable();
    void run(void);
    PseuGUI *GetGUI(void);
private:
    PseuGUI* _gui;
};


class PseuGUI
{
    //  too bad friends are not inherited...
    friend class Scene;
    friend class SceneWorld;
    friend class SceneGuiStart;
    friend class SceneLogin;
    friend class SceneCharSelection;
    // ...

public:
    PseuGUI();
    ~PseuGUI();

    void SetInstance(PseuInstance*);
    inline PseuInstance *GetInstance(void) { return _instance; }
    void Run(void);
    void SetDriver(uint8);
    void SetResolution(uint16 x, uint16 y, uint16 depth=32);
    void SetWindowed(bool);
    void SetVSync(bool);
    void UseShadows(bool);
    void Cancel(void);
    void Shutdown(void);
    inline bool IsInitialized(void) { return _initialized && _device; }
    inline void SetUseSound(bool b) { _usesound = b; }

    inline bool MustDie(void) { return _mustdie; }

    // interfaces to tell the gui what to draw
    void NotifyObjectDeletion(uint64 guid);
    void NotifyObjectCreation(Object *o);
    void NotifyAllObjectsDeletion(void);

    // scenes
    void SetSceneState(SceneState);
    bool SetSceneData(uint32, uint32);
    uint32 GetSceneState(void);
    inline void UpdateScene(void) { _updateScene = true; }

    // helpers
    WorldPosition GetWorldPosition(void);

private:
    void _Init(void);
    void _UpdateSceneState(void);
    uint16 _xres,_yres,_colordepth;
    bool _windowed,_vsync,_shadows;
    bool _initialized,_mustdie;
    bool _usesound;
    irr::IrrlichtDevice *_device;
    irr::video::IVideoDriver* _driver;
    irr::scene::ISceneManager* _smgr;
    irr::gui::IGUIEnvironment* _guienv;
    irr::video::E_DRIVER_TYPE _driverType;
    irrklang::ISoundEngine *_soundengine;
    DrawObjMgr domgr;
    PseuInstance *_instance;
    SceneState _scenestate, _scenestate_new;
    Scene *_scene;
    irr::ITimer *_timer;
    uint32 _passtime, _lastpasstime, _passtimediff;
    irr::core::dimension2d<irr::s32> _screendimension;
    uint32 _throttle;//used for frameratelimiting
    bool _updateScene; // manually update scene?

};



#endif

