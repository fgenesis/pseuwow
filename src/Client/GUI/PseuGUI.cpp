#include "common.h"
#include "irrlicht/irrlicht.h"
#include "Object.h"
#include "DrawObject.h"
#include "PseuWoW.h"
#include "PseuGUI.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

enum DriverIDs
{
    NULLDEVICE = 0,
    SOFTWARE = 1,
    BURNINGSVIDEO = 2,
    OPENGL = 3,
    DIRECTX8 = 4,
    DIRECTX9 = 5,
};

PseuGUIRunnable::PseuGUIRunnable()
{
    _gui = new PseuGUI();
}

void PseuGUIRunnable::run(void)
{
    _gui->Run();
}

PseuGUI *PseuGUIRunnable::GetGUI(void)
{
    return _gui;
}

PseuGUIRunnable::~PseuGUIRunnable()
{
    delete _gui;
}

PseuGUI::PseuGUI()
{
    _xres = 640;
    _yres = 480;
    _colordepth = 32;
    _shadows = false;
    _windowed = true;
    _vsync = false;
    _initialized = false;
    _mustdie = false;
    _driverType = video::EDT_BURNINGSVIDEO; // nulldevice makes not really a sense to display stuff
}

PseuGUI::~PseuGUI()
{
    this->Cancel();
    _instance->DeleteGUI(); // this makes the instance set its gui ptr to NULL
}

void PseuGUI::SetDriver(uint8 driverId)
{
    switch(driverId)
    {
    case DIRECTX9: _driverType = video::EDT_DIRECT3D9;break;
    case DIRECTX8: _driverType = video::EDT_DIRECT3D8;break;
    case OPENGL: _driverType = video::EDT_OPENGL;   break;
    case SOFTWARE:      _driverType = video::EDT_SOFTWARE; break;
    case BURNINGSVIDEO: _driverType = video::EDT_BURNINGSVIDEO;break;
    case NULLDEVICE:    _driverType = video::EDT_NULL;     break;
    default: _driverType = video::EDT_BURNINGSVIDEO; // if no valid driver detected, use software
    }

    // TODO: add support for changing driver during runtime?
}

void PseuGUI::SetResolution(uint16 x, uint16 y, uint16 depth)
{
    _xres = x;
    _yres = y;
    _colordepth = depth;
}

void PseuGUI::SetWindowed(bool b)
{
    _windowed = b;
    // see PseuGUI::Init(): !_windowed == fullscreen
}

void PseuGUI::SetVSync(bool b)
{
    _vsync = b;
}

void PseuGUI::UseShadows(bool b)
{
    _shadows = b;
}

// if this fuction is called from another thread the device will not work correctly. o_O
void PseuGUI::_Init(void)
{
    _device = createDevice(_driverType,dimension2d<s32>(_xres,_yres),_colordepth,!_windowed,_shadows,_vsync);
    DEBUG(logdebug("PseuGUI::Init() _device=%X",_device));
    _device->setWindowCaption(L"PseuWoW - Initializing");
    _driver = _device->getVideoDriver();
    _smgr = _device->getSceneManager();
    //...
    _initialized = true;
}

void PseuGUI::Cancel(void)
{
    DEBUG(logdebug("PseuGUI::Cancel()"));
    _mustdie = true;
    if(_device)
    {
        _device->drop();
        _device = NULL;
    }
}

void PseuGUI::Shutdown(void)
{
     DEBUG(logdebug("PseuGUI::Shutdown()"));
    _mustdie = true;
}

void PseuGUI::Run(void)
{
    if(!_initialized)
        this->_Init();

    DEBUG(logdebug("PseuGUI::Run() _device=%X",_device));

    int lastFPS = -1, fps = -1;

    while(_device && _device->run() && !_mustdie)
    {
        if (!_device->isWindowActive())
        {
            _device->sleep(10); // save cpu & gpu power if not focused
        }

        try
        {
            _driver->beginScene(true, true, 0);

            domgr.Update(); // iterate over DrawObjects, draw them and clean up

            _smgr->drawAll();

            _driver->endScene();
        }
        catch(...)
        {
            logerror("Unhandled exception in PseuGUI::Run() device=%X smgr=%X objects:%u", _device, _smgr, domgr.StorageSize());
        }

        fps = _driver->getFPS();

        if (lastFPS != fps)
        {
            core::stringw str = L"PseuWoW [";
            str += _driver->getName();
            str += "] FPS:";
            str += fps;

            _device->setWindowCaption(str.c_str());

            lastFPS = fps;
            DEBUG(logdebug("PseuGUI: Current FPS: %u",fps));
        }

    }
    DEBUG(logdebug("PseuGUI::Run() finished"));
    Cancel(); // already got shut down somehow, we can now safely cancel and drop the device
}

// called from ObjMgr::Remove(guid)
void PseuGUI::NotifyObjectDeletion(uint64 guid)
{
    domgr.Delete(guid);
}

// called from ObjMgr::Add(Object*)
void PseuGUI::NotifyObjectCreation(Object *o)
{
    DrawObject *d = new DrawObject(_smgr,o);
    domgr.Add(o->GetGUID(),d);
}

void PseuGUI::SetInstance(PseuInstance* in)
{
    _instance = in;
}


