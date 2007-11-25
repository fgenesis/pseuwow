#include "common.h"
#include "PseuGUI.h"
#include "PseuWoW.h"
#include "Scene.h"
#include "MapTile.h"
#include "MapMgr.h"
#include "ShTlTerrainSceneNode.h"
#include "MCamera.h"
#include "MInput.h"
#include "WorldSession.h"
#include "World.h"
#include <sstream>


SceneWorld::SceneWorld(PseuGUI *g) : Scene(g)
{
    DEBUG(logdebug("SceneWorld: Initializing..."));

    s32 mapsize = 9 * 16 * 3; // 9 height floats in 16 chunks per tile per axis in 3 MapTiles
    s32 tilesize = UNITSIZE;
    s32 meshsize = CHUNKSIZE*3;
    vector3df terrainPos(0.0f, 0.0f, 0.0f); // TODO: use PseuWoW's world coords here?

    eventrecv = new MyEventReceiver();
    device->setEventReceiver(eventrecv);

    camera = new MCameraFPS(smgr);
    camera->setNearValue(0.1f);
    camera->setFarValue(tilesize*meshsize/2);
    camera->setPosition(core::vector3df(mapsize*tilesize/2, 0, mapsize*tilesize/2) + terrainPos);
    
    terrain = new ShTlTerrainSceneNode(smgr,mapsize,mapsize,tilesize,meshsize);
    terrain->drop();
    terrain->follow(camera->getNode());
    terrain->setMaterialTexture(0, driver->getTexture("data/misc/dirt_test.jpg"));
    terrain->setDebugDataVisible(scene::EDS_FULL);
    terrain->setMaterialFlag(video::EMF_LIGHTING, true);
    terrain->setMaterialFlag(video::EMF_FOG_ENABLE, false);
    terrain->setPosition(terrainPos);

    // randomize base color
    for(s32 j=0; j<terrain->getSize().Height+1; j++)
        for(s32 i=0; i<terrain->getSize().Width+1; i++)
        {
            u32 g = (rand() % 150) + 80;
            u32 r = (rand() % 50);
            u32 b = (rand() % 50);

            terrain->setColor(i,j, video::SColor(255,r,g,b));
        }

    MapMgr *mapmgr = g->GetInstance()->GetWSession()->GetWorld()->GetMapMgr();

    // TODO: better to do this with some ZThread Condition or FastMutex, but dont know how to. help plz! [FG]
    if(!mapmgr->Loaded())
    {
        logdebug("SceneWorld: Waiting until maps are loaded...");
        while(!mapmgr->Loaded())
            device->sleep(50);
    }

    // something is not good here. we have terrain, but the chunks are read incorrectly.
    // need to find out where which formula is wrong
    // the current terrain renderer code is just a test to see if ADT files are read correctly.
    // EDIT: it seems to display fine now, but i am still not sure if the way it is done is correct...
    mutex.acquire(); // prevent other threads deleting the maptile
    for(s32 tiley = 0; tiley < 3; tiley++)
    {
        for(s32 tilex = 0; tilex < 3; tilex++)
        {
            MapTile *maptile = mapmgr->GetNearTile(tilex - 1, tiley - 1);
            if(maptile)
            {
                // apply map height data
                for(uint32 chy = 0; chy < 16; chy++)
                    for(uint32 chx = 0; chx < 16; chx++)
                    {
                        MapChunk *chunk = maptile->GetChunk(chx, chy);
                        std::stringstream ss;
                        DEBUG(logdebug("Apply MapChunk (%u, %u)",chx,chy));
                        for(uint32 hy = 0; hy < 9; hy++)
                        {
                            for(uint32 hx = 0; hx < 9; hx++)
                            {
                                f32 h = chunk->hmap_rough[hx * 9 + hy] + chunk->baseheight; // not sure if hx and hy are used correctly here
                                h *= -1; // as suggested by bLuma
                                ss.precision(3);
                                ss << h << '\t';
                                terrain->setHeight((144 * tiley) + (9 * chx) + hx, (144  * tilex) + (9 * chy) + hy, h);
                            }
                            ss << "\n";
                        }
                        //DEBUG(logdebug("\n%s\n",ss.str().c_str()));
                    }
            }
            else
            {
                logerror("SceneWorld: MapTile not loaded, can't apply heightmap!");
            }
        }
    }
    mutex.release();

    terrain->smoothNormals();

    ILightSceneNode* light = smgr->addLightSceneNode(0, core::vector3df(0,0,0), 
    SColorf(255, 255, 255, 255), 1000.0f);
    SLight ldata = light->getLightData();
    ldata.AmbientColor = video::SColorf(0.2f,0.2f,0.2f);
    ldata.DiffuseColor = video::SColorf(1.0f,1.0f,1.0f);
    ldata.Type = video::ELT_DIRECTIONAL;
    ldata.Position = core::vector3df(-10,5,-5);
    light->setLightData(ldata);

    driver->setFog(video::SColor(255,100,101,140), true, tilesize*meshsize/4, tilesize*(meshsize-4)/2, 0.05f);

    DEBUG(logdebug("SceneWorld: Init done!"));
}

void SceneWorld::OnUpdate(f32 timediff)
{
    if(eventrecv->key.pressed(KEY_KEY_W)) camera->moveForward(50 * timediff);
    if(eventrecv->key.pressed(KEY_KEY_S)) camera->moveBack(50 * timediff);
    if(eventrecv->key.pressed(KEY_KEY_D)) camera->moveRight(50 * timediff);
    if(eventrecv->key.pressed(KEY_KEY_A)) camera->moveLeft(50 * timediff);

    // mouse directional control
    camera->turnRight(5 * (device->getCursorControl()->getRelativePosition().X - 0.5f));
    camera->turnUp(5 * (device->getCursorControl()->getRelativePosition().Y - 0.5f));
    //device->getCursorControl()->setPosition(0.5f, 0.5f);

    // camera height control
    if (eventrecv->mouse.wheel < 10) eventrecv->mouse.wheel = 10;
    camera->setHeight(  eventrecv->mouse.wheel + terrain->getHeight(camera->getPosition())  );
}

void SceneWorld::OnDraw(void)
{
    // draw all objects
    gui->domgr.Update(); // iterate over DrawObjects, draw them and clean up
}

void SceneWorld::OnDelete(void)
{
    DEBUG(logdebug("~SceneWorld()"));
}
