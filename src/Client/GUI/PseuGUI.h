#ifndef PSEUGUI_H
#define PSEUGUI_H

#include "irrlicht/irrlicht.h"
#include "DrawObjMgr.h"

class PseuGUI;
class Object;
class PseuInstance;

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
public:
    PseuGUI();
    ~PseuGUI();

    void SetInstance(PseuInstance*);
    void Run(void);
    void SetDriver(uint8);
    void SetResolution(uint16 x, uint16 y, uint16 depth=32);
    void SetWindowed(bool);
    void SetVSync(bool);
    void UseShadows(bool);
    void Cancel(void);
    void Shutdown(void);
    inline bool MustDie(void) { return _mustdie; }

    // interfaces to tell the gui what to draw
    void NotifyObjectDeletion(uint64 guid);
    void NotifyObjectCreation(Object *o);

private:
    void _Init(void);
    uint16 _xres,_yres,_colordepth;
    bool _windowed,_vsync,_shadows;
    bool _initialized,_mustdie;
    irr::IrrlichtDevice *_device;
    irr::video::IVideoDriver* _driver;
    irr::scene::ISceneManager* _smgr;
    irr::gui::IGUIEnvironment* _guienv;
    irr::video::E_DRIVER_TYPE _driverType;
    DrawObjMgr domgr;
    PseuInstance *_instance;

};



#endif

