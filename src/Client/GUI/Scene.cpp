#include "common.h"
#include "PseuGUI.h"
#include "PseuWoW.h"
#include "CCursorController.h"
#include "Scene.h"

Scene::Scene(PseuGUI *g)
{
    memset(scenedata, 0, sizeof(uint32) * SCENEDATA_SIZE);
    textdb = NULL;
    gui = g;
    instance = gui->GetInstance();
    device = gui->_device;
    driver = gui->_driver;
    smgr = gui->_smgr;
    guienv = gui->_guienv;
    rootgui = guienv->getRootGUIElement();
    cursor = new CCursorController(device->getCursorControl(), driver);
    cursor->setOSCursorVisible(true);
    cursor->setVisible(false);
    cursor->render(); // apply above settings

    soundengine = gui->_soundengine;
}

void Scene::OnDraw(void)
{
    device->yield();
}

void Scene::OnDelete(void)
{
}

void Scene::OnUpdate(s32)
{
}

void Scene::OnManualUpdate(void)
{
    OnResize();
}

void Scene::OnResize(void)
{
}

void Scene::OnDrawBegin(void)
{
}

video::SColor Scene::GetBackgroundColor(void)
{
    return 0;
}

Scene::~Scene()
{
    delete cursor;
    DEBUG(logdebug("Scene::~Scene()"));
}

core::stringw Scene::GetStringFromDB(u32 index, u32 entry, SCPDatabase *other_db /* = NULL */)
{
    core::stringw r = "";
    SCPDatabase *db = other_db ? other_db : textdb;
    if(!db)
    {
        r += L"<string ";
        r += index;
        r += L"/";
        r += entry;
        r += L" not found>";
        return r;
    }
    char buf[20];
    sprintf(buf,"%u",entry);
    r += db->GetString(index, buf);
    return r;
}
