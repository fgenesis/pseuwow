#include "common.h"
#include "PseuGUI.h"
#include "PseuWoW.h"
#include "Scene.h"

SceneWorld::SceneWorld(PseuGUI *g) : Scene(g)
{
}

void SceneWorld::Draw(void)
{
    // draw maps here

    // draw all objects
    gui->domgr.Update(); // iterate over DrawObjects, draw them and clean up

    // draw interface here

}

SceneWorld::~SceneWorld()
{
}