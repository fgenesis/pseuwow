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

void Scene::Draw(void)
{
    device->yield();
}
