#include "common.h"
#include "irrlicht/irrlicht.h"
#include "CM2MeshFileLoader.h"
#include "CWMOMeshFileLoader.h"
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
    _soundengine = NULL;
    _usesound = false;
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
    _screendimension = _driver->getScreenSize();
    //...

    // disable crappy irrlicht logging
    _device->getLogger()->setLogLevel(ELL_NONE);

    // register external loaders for not supported filetypes
    video::CImageLoaderBLP* BLPloader = new video::CImageLoaderBLP();
	_driver->addExternalImageLoader(BLPloader);
    scene::CM2MeshFileLoader* m2loader = new scene::CM2MeshFileLoader(_device, "./data/texture");
    _smgr->addExternalMeshLoader(m2loader);
    scene::CWMOMeshFileLoader* wmoloader = new scene::CWMOMeshFileLoader(_device, "./data/texture");
    _smgr->addExternalMeshLoader(wmoloader);
    _throttle=0;
    _initialized = true;

    // initialize the sound engine
    if(_usesound)
    {
        _soundengine = createIrrKlangDevice();
        if(_soundengine)
        {
            logdetail("PseuGUI: Sound Driver: %s",_soundengine->getDriverName());
            _soundengine->setSoundVolume(GetInstance()->GetConf()->masterSoundVolume);
            // accept only values between 0 and 1
            if(_soundengine->getSoundVolume() < 0.0f || _soundengine->getSoundVolume() >= 1.0f)
                _soundengine->setSoundVolume(1.0f);
            logdetail("PseuGUI: Master Sound Volume: %.3f",_soundengine->getSoundVolume());
        }
        else
            logerror("PseuGUI: Failed to initialize sound engine!");
    }
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
    if(_soundengine)
    {
        _soundengine->drop();
        _soundengine = NULL;
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

        if (!_device->isWindowActive())
        {
            _device->sleep(10); // save cpu & gpu power if not focused
        }

        _UpdateSceneState();

        if(!_scene)
        {
            _device->sleep(10);
            continue;
        }

        if(_screendimension != _driver->getScreenSize())
        {
            _scene->OnResize();
            _screendimension = _driver->getScreenSize();
        }

        if(_updateScene)
        {
            _updateScene = false;
            _scene->OnManualUpdate();
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

void PseuGUI::NotifyAllObjectsDeletion(void)
{
    domgr.Clear();
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
            _scene = NULL;
        }
        _smgr->clear();
        _guienv->clear();



        logdebug("PseuGUI: switching to SceneState %u", _scenestate_new);

        switch (_scenestate_new)
        {
            case SCENESTATE_GUISTART: _scene = new SceneGuiStart(this); break;
            case SCENESTATE_LOGINSCREEN: _scene = new SceneLogin(this); break;
            case SCENESTATE_WORLD: _scene = new SceneWorld(this); break;
            case SCENESTATE_REALMSELECT:
                _scene = new SceneCharSelection(this);
                _scene->SetData(ISCENE_CHARSEL_REALMFIRST, 1);
                _scenestate_new = SCENESTATE_CHARSELECT;
                break;
            case SCENESTATE_CHARSELECT:
                _scene = new SceneCharSelection(this);
                _scene->SetData(ISCENE_CHARSEL_REALMFIRST, 0);
                break;
            default: _scene = new Scene(this); // will draw nothing, just yield the gui
        }
        _scene->SetState(_scenestate_new);
        // current scenestate can be set safely after scene is created and ready
        _scenestate = _scenestate_new;


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
    /* // not good, not threadsafe! (can crash)
    if(!_scene)
        return SCENESTATE_NOSCENE;
    return _scene->GetState();*/
    return _scenestate;
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

