#include "common.h"
#include "irrlicht/irrlicht.h"
#include "CM2MeshFileLoader.h"
#include "CImageLoaderBLP.h"
#include "Object.h"
#include "DrawObject.h"
#include "PseuWoW.h"
#include "Scene.h"
#include "PseuGUI.h"

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
    _scenestate = _scenestate_new = SCENESTATE_NULL;
    _smgr = NULL;
    _device = NULL;
    _guienv = NULL;
    _scene = NULL;
    _passtime = _lastpasstime = _passtimediff = 0;
    }

PseuGUI::~PseuGUI()
{
    domgr.Clear();
    this->Cancel();
    _instance->DeleteGUI(); // this makes the instance set its gui ptr to NULL
    logdebug("PseuGUI::~PseuGUI()");
}

void PseuGUI::SetDriver(uint8 driverId)
{
    switch(driverId)
    {
    case DIRECTX9:      _driverType = video::EDT_DIRECT3D9;     break;
    case DIRECTX8:      _driverType = video::EDT_DIRECT3D8;     break;
    case OPENGL:        _driverType = video::EDT_OPENGL;        break;
    case SOFTWARE:      _driverType = video::EDT_SOFTWARE;      break;
    case BURNINGSVIDEO: _driverType = video::EDT_BURNINGSVIDEO; break;
    case NULLDEVICE:    _driverType = video::EDT_NULL;          break;
    default:            _driverType = video::EDT_BURNINGSVIDEO; // if no valid driver detected, use software
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
    if(!_device)
    {
        logerror("PseuGUI: Can't use specified video driver, trying software mode...");
        _device = createDevice(video::EDT_SOFTWARE,dimension2d<s32>(_xres,_yres),_colordepth,!_windowed,false,false);
        if(!_device)
        {
            logerror("ERROR: PseuGUI::_Init() failed, no video driver available!");
            return;
        }
        else
        {
            logerror("PseuGUI: Software mode OK!");
        }
    }
    DEBUG(logdebug("PseuGUI::Init() _device=%X",_device));
    _device->setWindowCaption(L"PseuWoW - Initializing");
    _device->setResizeAble(true);
    _driver = _device->getVideoDriver();
    _smgr = _device->getSceneManager();
    _guienv = _device->getGUIEnvironment();
    _timer = _device->getTimer();
    //...

    // register external loaders for not supported filetypes
    video::CImageLoaderBLP* BLPloader = new video::CImageLoaderBLP();
	_driver->addExternalImageLoader(BLPloader);
    scene::CM2MeshFileLoader* m2loader = new scene::CM2MeshFileLoader(_device, "./data/texture");
    _smgr->addExternalMeshLoader(m2loader);
    _throttle=0;
    _initialized = true;
}

void PseuGUI::Cancel(void)
{
    DEBUG(logdebug("PseuGUI::Cancel()"));

    if(_scene)
    {
        _scene->OnDelete();
        delete _scene;
        _scene = NULL;
    }
    if(_device)
    {
        _device->drop();
        _device = NULL;

    }
    _mustdie = true;
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
    if(!_initialized) // recheck
    {
        logerror("PseuGUI: not initialized, using non-GUI mode");
        Cancel();
        return;
    }

    DEBUG(logdebug("PseuGUI::Run() _device=%X",_device));

    int lastFPS = -1, fps = -1;

    while(_device && _device->run() && !_mustdie)
    {
        _lastpasstime = _passtime;
        _passtime = _timer->getTime();
        _passtimediff = _passtime - _lastpasstime;
        // _HandleWindowResize(); // not yet used; doesnt work

        if (!_device->isWindowActive())
        {
            _device->sleep(10); // save cpu & gpu power if not focused
        }

        try
        {
            _UpdateSceneState();

            if(!_scene)
            {
                _device->sleep(10);
                continue;
            }

            _scene->OnUpdate(_passtimediff); // custom: process input, set camera, etc
            _driver->beginScene(true, true, _scene->GetBackgroundColor()); // irr: call driver to start drawing
            _scene->OnDrawBegin(); // custom: draw everything before irrlicht draws everything by itself
            _smgr->drawAll(); // irr: draw all scene nodes
            _guienv->drawAll(); // irr: draw gui elements
            _scene->OnDraw(); // custom: draw everything that has to be draw late (post-processing also belongs here)
            _driver->endScene(); // irr: drawing done
            if(_driver->getFPS()>100 && _throttle < 10)//Primitive FPS-Limiter - upper cap hardcoded 100 FPS.
                _throttle++;                           //lowercap 60 (if it drops below, limiting is eased).
            if(_driver->getFPS()<60 && _throttle>0)    //but honestly, a 10 msec delay is not worth this amount of code.
                _throttle--;                           //If the FPS is down, it will never be because of this
            if(_throttle>0)                            //Thus i opt for dropping the charade and using a fixed conf value of max 10.
                _device->sleep(_throttle);             //sleeps max 10 msec (=_throttle) here.

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
        }

    }
    domgr.UnlinkAll(); // At this point the irr::device is probably closed and deleted already, which means it deleted
                       // all SceneNodes and everything. the ptrs are still stored in the DrawObjects, means they need to be unlinked now not to cause a crash.
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
    DrawObject *d = new DrawObject(_device,o,_instance);
    domgr.Add(o->GetGUID(),d);
}

void PseuGUI::SetInstance(PseuInstance* in)
{
    _instance = in;
}

void PseuGUI::SetSceneState(SceneState state)
{
    _scenestate_new = state; // will be applied at next cycle
}

void PseuGUI::_UpdateSceneState(void)
{
    if(_scenestate != _scenestate_new && _smgr)
    {
        if(_scene)
        {
            _scene->OnDelete();
            delete _scene;
        }
        _smgr->clear();
        _guienv->clear();

        _scenestate = _scenestate_new;

        logdebug("PseuGUI: switched to SceneState %u", _scenestate);

        switch (_scenestate)
        {
            case SCENESTATE_GUISTART: _scene = new SceneGuiStart(this); break;
            case SCENESTATE_LOGINSCREEN: _scene = new SceneLogin(this); break;
            case SCENESTATE_WORLD: _scene = new SceneWorld(this); break;
            default: _scene = new Scene(this); // will draw nothing, just yield the gui
        }
        _scene->SetState(_scenestate);

        logdebug("PseuGUI: scene created.");
    }
}

bool PseuGUI::SetSceneData(uint32 index, uint32 value)
{
    if(!_scene)
        return false;
    _scene->SetData(index, value);
    return true;
}

uint32 PseuGUI::GetSceneState(void)
{
    if(!_scene)
        return SCENESTATE_NOSCENE;
    return _scene->GetState();
}


// used to get our current WorldPosition
WorldPosition PseuGUI::GetWorldPosition(void)
{
    if(_scene && _scene->GetState() == SCENESTATE_WORLD)
    {
        return ((SceneWorld*)_scene)->GetWorldPosition();
    }
    return WorldPosition();
}

void PseuGUI::_HandleWindowResize(void)
{
    dimension2d<s32> scrn = _driver->getScreenSize();
    if(_screendimension.Width != scrn.Width)
    {
        scrn.Height = s32(scrn.Width * 0.8f); // for now use aspect ratio 5:4
        _screendimension = scrn;
        _driver->OnResize(scrn);
        DEBUG(logdebug("DEBUG: Width resize handled, Height adjusted"));

    }
    else if(_screendimension.Height != scrn.Height)
    {
        scrn.Width = s32(scrn.Height * 1.25); // 5:4 here too
        _screendimension = scrn;
        _driver->OnResize(scrn);
        DEBUG(logdebug("DEBUG: Height resize handled, Width adjusted"));

    }
    // TODO: how to set irrlicht window size ?!

}
