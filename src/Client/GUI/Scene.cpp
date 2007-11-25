#include "common.h"
#include "PseuGUI.h"
#include "PseuWoW.h"
#include "Scene.h"

Scene::Scene(PseuGUI *g)
{
    gui = g;
    device = gui->_device;
    driver = gui->_driver;
    smgr = gui->_smgr;
    guienv = gui->_guienv;
}

void Scene::OnDraw(void)
{
    device->yield();
}

void Scene::OnDelete(void)
{
}

void Scene::OnUpdate(f32)
{
}

Scene::~Scene()
{
    DEBUG(logdebug("Scene::~Scene()"));
}
