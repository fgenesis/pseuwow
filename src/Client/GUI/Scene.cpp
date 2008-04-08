#include "common.h"
#include "PseuGUI.h"
#include "PseuWoW.h"
#include "CCursorController.h"
#include "Scene.h"

Scene::Scene(PseuGUI *g)
{
    memset(scenedata, 0, sizeof(uint32) * SCENEDATA_SIZE);
    gui = g;
    instance = gui->GetInstance();
    device = gui->_device;
    driver = gui->_driver;
    smgr = gui->_smgr;
    guienv = gui->_guienv;
    cursor = new CCursorController(device->getCursorControl(), driver);
    cursor->setOSCursorVisible(true);
    cursor->setVisible(false);
    cursor->render(); // apply above settings
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

void Scene::OnDrawBegin(void)
{
}

video::SColor Scene::GetBackgroundColor(void)
{
    return 0;
}

Scene::~Scene()
{
    DEBUG(logdebug("Scene::~Scene()"));
}
