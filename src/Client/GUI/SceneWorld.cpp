#include <sstream>
#include <math.h>
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
#include "CCursorController.h"
#include "MovementMgr.h"
#include "DrawObject.h"
#include "irrKlangSceneNode.h"

// TODO: replace this by conf value
#define MAX_CAM_DISTANCE 70

SceneWorld::SceneWorld(PseuGUI *g) : Scene(g)
{
    DEBUG(logdebug("SceneWorld: Initializing..."));
    debugmode = false;
    _freeCameraMove = true;

    // store some pointers right now to prevent repeated ptr dereferencing later (speeds up code)
    gui = g;
    wsession = gui->GetInstance()->GetWSession();
    world = wsession->GetWorld();
    mapmgr = world->GetMapMgr();
    movemgr = world->GetMoveMgr();
    mychar = wsession->GetMyChar();
    ASSERT(mychar);
    _CalcXYMoveVect(mychar->GetO());
    old_char_o = mychar->GetO();

    if(soundengine)
    {
        soundengine->stopAllSounds();
    }

    ILightSceneNode* light = smgr->addLightSceneNode(0, core::vector3df(0,0,0), SColorf(255, 255, 255, 255), 1000.0f);
    SLight ldata = light->getLightData();
    ldata.AmbientColor = video::SColorf(0.2f,0.2f,0.2f);
    ldata.DiffuseColor = video::SColorf(1.0f,1.0f,1.0f);
    ldata.Type = video::ELT_DIRECTIONAL;
    ldata.Position = core::vector3df(-0.22f,-1,0);
    light->setLightData(ldata);

    eventrecv = new MyEventReceiver();
    device->setEventReceiver(eventrecv);
    eventrecv->mouse.wheel = MAX_CAM_DISTANCE;

    camera = new MCameraFPS(smgr);
    camera->setNearValue(0.1f);

    f32 farclip = instance->GetConf()->farclip;
    if(farclip < 50)
        farclip = TILESIZE;

    f32 fov = instance->GetConf()->fov;
    if(!iszero(fov))
    {
        logdetail("Camera: Field of view (FOV) = %.3f",fov);
        camera->setFOV(fov);
    }

    camera->setFarValue(farclip);

    debugText = guienv->addStaticText(L"< debug text >",rect<s32>(0,0,driver->getScreenSize().Width,30),true,true,0,-1,true);

    envBasicColor = video::SColor(0,100,101,190);

    smgr->setShadowColor(); // set shadow to default color

    sky = NULL;
    selectedNode = oldSelectedNode = NULL;
    //sky = smgr->addSkyDomeSceneNode(driver->getTexture("data/misc/sky.jpg"),64,64,1.0f,2.0f);
    /* // TODO: for now let irrlicht draw the skybox
    sky->grab(); // if the camera clip is set too short, the sky will not be rendered properly.
    sky->remove(); // thus we grab the sky node while removing it from rendering.
    */

    f32 fogfar = instance->GetConf()->fogfar;
    if(fogfar < 30)
        fogfar = farclip * 0.7f;

    f32 fognear = instance->GetConf()->fognear;
    if(fognear < 10)
        fognear = fogfar * 0.75f;

    logdetail("GUI: Using farclip=%.2f fogfar=%.2f fognear=%.2f", farclip, fogfar, fognear);

    driver->setFog(envBasicColor, true, fognear, fogfar, 0.02f);

    // setup cursor
    cursor->setOSCursorVisible(false);
    cursor->addMouseCursorTexture("data/misc/cursor.png", true);
    cursor->setVisible(true);

    InitTerrain();
    UpdateTerrain();
    RelocateCameraBehindChar();

    DEBUG(logdebug("SceneWorld: Init done!"));
}

void SceneWorld::OnUpdate(s32 timediff)
{
    static position2d<s32> mouse_pos;

    UpdateTerrain();

    mouse_pressed_left = eventrecv->mouse.left_pressed();
    mouse_pressed_right = eventrecv->mouse.right_pressed();
    float timediff_f = timediff / 1000.0f;

    if( (mouse_pressed_right || mouse_pressed_left) && cursor->isVisible())
    {
        // TODO: if mouse is hovering over a gui element, do not hide
        cursor->setVisible(false);
    }
    else if( !(mouse_pressed_right || mouse_pressed_left) && !cursor->isVisible())
    {
        cursor->setVisible(true);
    }

    // object focused - only check if mouse moved, saves CPU
    // TODO: check if camera moved, also (maybe from external source)
    /*if(mouse_pos != cursor->getMousePos())
    {
        focusedNode = smgr->getSceneCollisionManager()->getSceneNodeFromScreenCoordinatesBB(cursor->getMousePos());
        if(focusedNode && mouse_pressed_left)
            selectedNode = focusedNode;
    }*/ // i'll continue working on this - [FG]


    // maybe it is better to replace the sin() and cos() with some irr::core::matrix4 calcualtions... not sure what is more efficient

    if(eventrecv->key.pressed(KEY_KEY_W) || (mouse_pressed_left && mouse_pressed_right))
    {
        if(_freeCameraMove)
            camera->moveForward(50 * timediff_f);
        else
        {
            f32 speedfactor = timediff_f * mychar->GetSpeed(MOVE_RUN);
            movemgr->SetMoveMode(MOVEMODE_MANUAL);
            movemgr->MoveStartForward();
            WorldPosition wp = mychar->GetPosition();
            _CalcXYMoveVect(wp.o);
            wp.x += (xyCharMovement.X * speedfactor);
            wp.y += (xyCharMovement.Y * speedfactor);
            wp.z = terrain->getHeight(WPToIrr(wp));
            mychar->SetPosition(wp);
        }
    }

    if(eventrecv->key.pressed(KEY_KEY_S))
    {
        if(_freeCameraMove)
            camera->moveBack(50 * timediff_f);
        else
        {
            f32 speedfactor = timediff_f * mychar->GetSpeed(MOVE_WALKBACK);
            movemgr->SetMoveMode(MOVEMODE_MANUAL);
            movemgr->MoveStartBackward();
            WorldPosition wp = mychar->GetPosition();
            _CalcXYMoveVect(wp.o);
            wp.x -= (xyCharMovement.X * speedfactor);
            wp.y -= (xyCharMovement.Y * speedfactor);
            wp.z = terrain->getHeight(WPToIrr(wp));
            mychar->SetPosition(wp);
        }
    }

    // if right mouse button pressed, move in axis, if not, turn camera
    if(eventrecv->key.pressed(KEY_KEY_D))
    {
        if(mouse_pressed_right)
        {
            if(_freeCameraMove)
                camera->moveRight(50 * timediff_f);
            else
            {
                // TODO: strafe case
            }
        }
        else
        {
            if(_freeCameraMove)
                camera->turnRight(timediff_f * M_PI * 25);
            else
            {
                movemgr->SetMoveMode(MOVEMODE_MANUAL);
                movemgr->MoveStartTurnRight();
                WorldPosition wp = mychar->GetPosition();
                wp.z = terrain->getHeight(WPToIrr(wp));
                wp.o -= (timediff_f * mychar->GetSpeed(MOVE_TURN));
                wp.o = RAD_FIX(wp.o);
                mychar->SetPosition(wp);
                _CalcXYMoveVect(wp.o);
            }
        }
    }

    if(eventrecv->key.pressed(KEY_KEY_A))
    {
        if(mouse_pressed_right)
        {
            if(_freeCameraMove)
                camera->moveLeft(50 * timediff_f);
            else
            {
                // TODO: strafe case
            }
        }
        else
        {
            if(_freeCameraMove)
                camera->turnLeft(timediff_f * M_PI * 25);
            else
            {
                movemgr->SetMoveMode(MOVEMODE_MANUAL);
                movemgr->MoveStartTurnLeft();
                WorldPosition wp = mychar->GetPosition();
                wp.z = terrain->getHeight(WPToIrr(wp));
                wp.o += (timediff_f * mychar->GetSpeed(MOVE_TURN));
                wp.o = RAD_FIX(wp.o);
                mychar->SetPosition(wp);
                _CalcXYMoveVect(wp.o);
            }
        }
    }

    if(eventrecv->key.pressed(KEY_KEY_E))
    {
        if(_freeCameraMove)
            camera->moveRight(50 * timediff_f);
        else
        {/*
            f32 speedfactor = timediff_f * mychar->GetSpeed(MOVE_RUN);
            movemgr->SetMoveMode(MOVEMODE_MANUAL);
            movemgr->MoveStartStrafeRight();
            WorldPosition wp = mychar->GetPosition();
            _CalcXYMoveVect(wp.o);
            wp.x -= (xyCharMovement.X * speedfactor);
            wp.y -= (xyCharMovement.Y * speedfactor);
            wp.z = terrain->getHeight(WPToIrr(wp));
            mychar->SetPosition(wp);
        */}
    }

    if(eventrecv->key.pressed(KEY_KEY_Q))
    {
        if(_freeCameraMove)
            camera->moveLeft(50 * timediff_f);
        else
        {/*
            f32 speedfactor = timediff_f * mychar->GetSpeed(MOVE_RUN);
            movemgr->SetMoveMode(MOVEMODE_MANUAL);
            movemgr->MoveStartStrafeLeft();
            WorldPosition wp = mychar->GetPosition();
            _CalcXYMoveVect(wp.o);
            wp.x -= (xyCharMovement.X * speedfactor);
            wp.y -= (xyCharMovement.Y * speedfactor);
            wp.z = terrain->getHeight(WPToIrr(wp));
            mychar->SetPosition(wp);
        */}
    }

    /*if(eventrecv->key.pressed(KEY_SPACE))
    {
        movemgr->SetMoveMode(MOVEMODE_MANUAL);
        movemgr->MoveJump();
    }*/

    // listen to *not* pressed keys only if manually moving
    if(movemgr->GetMoveMode() == MOVEMODE_MANUAL)
    {
        if (!eventrecv->key.pressed(KEY_KEY_D) && !eventrecv->key.pressed(KEY_KEY_A))
        {
            movemgr->MoveStopTurn();
        }
        if (!eventrecv->key.pressed(KEY_KEY_W) && !eventrecv->key.pressed(KEY_KEY_S) && !(mouse_pressed_left && mouse_pressed_right))
        {
            movemgr->MoveStop();
        }
        // TODO: add strafe case
    }

    // if we moved, relocate MyCharacter
    /*if(movemgr->IsMoved())
    {
        MyCharacter *my = wsession->GetMyChar();
        if(my)
        {
            WorldPosition newpos = GetWorldPosition();
            my->SetPosition(newpos);
        }
        else
        {
            logerror("SceneWorld: Can't move MyCharacter, not found!");
        }
    }*/

    if(eventrecv->key.pressed_once(KEY_HOME))
    {
        _freeCameraMove = !_freeCameraMove;

        // TODO: uncomment this as soon as the camera isn't adjusted anymore every single frame
        //if(!_freeCameraMove)
        //    RelocateCameraBehindChar();

        //TODO: this will not be needed anymore with the above code uncommented
        // make player always visble when switching to freefly mode
        scene::ISceneNode *charnode = GetMyCharacterSceneNode(); // NOTE: this call is absolutely not optimized!
        if(charnode)
            charnode->setVisible(true);
    }

    if(eventrecv->key.pressed_once(KEY_BACK))
    {
        debugmode = !debugmode;

        // -- rendering with all debug flags uses WAY too much CPU to leave it turned on
        E_DEBUG_SCENE_TYPE dflags = E_DEBUG_SCENE_TYPE(debugmode ? (EDS_BBOX | EDS_BBOX_BUFFERS | EDS_SKELETON) : EDS_OFF);

        const core::list<scene::ISceneNode*>& nodelist = smgr->getRootSceneNode()->getChildren();
        for(core::list<scene::ISceneNode*>::ConstIterator it = nodelist.begin(); it != nodelist.end(); it++)
        {
            (*it)->setDebugDataVisible(dflags);
        }
        // only terrain is especially useful with all debug flags enabled
        terrain->setDebugDataVisible(debugmode ? EDS_FULL : EDS_OFF);
    }

    if(eventrecv->key.pressed_once(KEY_INSERT))
    {
        IImage *scrnshot = driver->createScreenShot();
        if(scrnshot)
        {
            CreateDir("screenshots");
            std::string date = getDateString();
            for(uint32 i = 0; i < date.length(); i++)
            {
                if(date[i] == ':')
                    date[i] = '_';
            }
            if(date[date.length()-1] == ' ')
            {
                date = date.substr(0,date.length()-2);
            }
            driver->writeImageToFile(scrnshot, ("screenshots/PseuWoW " + date + ".jpg").c_str(), device->getTimer()->getRealTime());
            scrnshot->drop();
        }
    }

    // this fixes the camera getting screwed up by noob user; resets it back to a usable position if someone managed to flip it over
    if(camera->getPitch() < 270 && camera->getPitch() > 90)
        camera->turnUp(90);

    // camera distance control
    if (eventrecv->mouse.wheel < 0)
        eventrecv->mouse.wheel = 0;
    if(eventrecv->mouse.wheel > MAX_CAM_DISTANCE)
        eventrecv->mouse.wheel = MAX_CAM_DISTANCE;

    if(mouse_pressed_left || mouse_pressed_right)
    {
        if(mouse_pos != cursor->getMousePos())
        {
            camera->turnRight(MOUSE_SENSIVITY * (device->getCursorControl()->getPosition().X - mouse_pos.X));
            // check if new camera pitch would cause camera to flip over; if thats the case keep current pitch
            f32 upval = MOUSE_SENSIVITY * (device->getCursorControl()->getPosition().Y - mouse_pos.Y);
            f32 newval = camera->getPitch() + upval;
            if( newval > 270.1f || newval < 89.9f)
            {
                camera->turnUp(upval);
            }
            device->getCursorControl()->setPosition(mouse_pos);

            // rotate character if right mouse button pressed.
            if(mouse_pressed_right && !_freeCameraMove)
            {
                mychar->GetPositionPtr()->o = PI*3/2 - DEG_TO_RAD(camera->getHeading());
                // send update to server only if we turned by some amount and not always when we turn
                if(!equals(old_char_o, mychar->GetO(), MOVE_TURN_UPDATE_DIFF))
                {
                    old_char_o = mychar->GetO();
                    movemgr->MoveSetFacing();
                }
            }
        }
    }
    else
    {
        mouse_pos = device->getCursorControl()->getPosition();
    }

    // TODO: check if the cam really has to be relocated; might save some CPU but not sure...
    if(_freeCameraMove)
    {
        camera->setHeight( terrain->getHeight(camera->getPosition()) + 4 );
    }
    else
    {
        RelocateCameraBehindChar();
    }

    core::stringw str = L"";

    DEBUG(
    WorldPosition wp = GetWorldPosition();
    str += L" Camera: pitch:";
    str += camera->getPitch();
    str += L"  c pos:";
    str += camera->getPosition().X;
    str += L",";
    str += camera->getPosition().Y;
    str += L",";
    str += camera->getPosition().Z;
    str += L"\n";
    str += " ## HEAD: ";
    str += IRR_TO_O(camera->getHeading());
    str += L"  Pos: ";
    str += wp.x;
    str += L" | ";
    str += wp.y;
    str += L" | ";
    str += wp.z;
    str += L" | OR:";
    str += wp.o;
//    str += ((((((str + wp.x) + L" | ") + wp.y) + L" | ") + wp.z) + L" | OR:") + wp.o;// WTF?
    str += L"  -- Terrain: Sectors: ";
    str += (int)terrain->getSectorsRendered();
    str += L" / ";
    str += (int)terrain->getSectorCount();
    str += L" (";
    str += (u32)(((f32)terrain->getSectorsRendered()/(f32)terrain->getSectorCount())*100.0f);
    str += L"%)";
    str += L" mwheel=";
    str += eventrecv->mouse.wheel;

    str += L"\n";


    const core::list<scene::ISceneNode*>& nodelist = smgr->getRootSceneNode()->getChildren();
    str += L"Scene nodes: total: ";
    str += nodelist.getSize();
    str += L" visible: ";
    u32 vis = 0;
    for(core::list<scene::ISceneNode*>::ConstIterator it = nodelist.begin(); it != nodelist.end(); it++)
        if((*it)->isVisible())
            vis++;
    str += vis;
    str += L"\n";
    ); // END DEBUG;

    str += driver->getFPS();
    str += L" FPS";

    debugText->setText(str.c_str());



    gui->domgr.Update(); // iterate over DrawObjects, draw them and clean up

}

void SceneWorld::OnDrawBegin(void)
{
    // TODO: this does work, but looks like the skybox is moving unsynced with the rest of the scene
    /*
    // this is the custom skybox/skydome render pass, done before anything else is drawn
    f32 clip = camera->getFarValue(); // store old farclip
    camera->setFarValue(50000); // set it so some incredible value, the sky is withoin these bounds for sure
    camera->getNode()->render(); // set the farclip in the driver
    //sky->OnRegisterSceneNode(); // process sky rendering
    sky->OnAnimate(0);
    sky->render();
    camera->setFarValue(clip); // restore old farclip
    */
}

void SceneWorld::OnDraw(void)
{
    cursor->render();
}

void SceneWorld::OnDelete(void)
{
    DEBUG(logdebug("~SceneWorld()"));
    _doodads.clear();
    _sound_emitters.clear();
    gui->domgr.Clear();
    delete camera;
    delete eventrecv;
    //sky->drop();
}

void SceneWorld::InitTerrain(void)
{
    if(!mapmgr)
    {
        logerror("SceneWorld: MapMgr not present, cant create World GUI. Switching back GUI to idle.");
        gui->SetSceneState(SCENESTATE_GUISTART);
        return;
    }
    s32 mapsize = (8 * 16 * 3) - 1; // 9-1 height floats in 16 chunks per tile per axis in 3 MapTiles

    // terrain rendering settings
    u32 rendersize = instance->GetConf()->terrainrendersize;
    if(!rendersize)
        rendersize = camera->getFarValue() / 3.0f;

    u32 sectors = instance->GetConf()->terrainsectors;
    if(!sectors)
        sectors = 5;

    u32 step = instance->GetConf()->terrainupdatestep;
    if(!step || step > 50)
        step = 1;

    logdetail("Terrain: Using %ux%u sectors, rendersize=%u, updatestep=%u",sectors,sectors,rendersize,step);

    terrain = new ShTlTerrainSceneNode(smgr,mapsize,mapsize,UNITSIZE,rendersize,sectors);
    terrain->drop();
    terrain->setStep(step);
    terrain->follow(camera->getNode());
    terrain->getMaterial(0).setTexture(1,driver->getTexture("data/misc/dirt_test.jpg"));
    terrain->getMaterial(0).setFlag(video::EMF_LIGHTING, true);
    terrain->getMaterial(0).setFlag(video::EMF_FOG_ENABLE, true);

}


void SceneWorld::UpdateTerrain(void)
{
    // check if we changed the maptile
    if(map_gridX == mapmgr->GetGridX() && map_gridY == mapmgr->GetGridY())
        return; // grid not changed, not necessary to update tile data

    // ... if changed, do necessary stuff...
    map_gridX = mapmgr->GetGridX();
    map_gridY = mapmgr->GetGridY();

    // TODO: better to do this with some ZThread Condition or FastMutex, but dont know how to. help plz! [FG]
    if(!mapmgr->Loaded())
    {
        logdebug("SceneWorld: Waiting until maps are loaded...");
        while(!mapmgr->Loaded())
            device->sleep(1);
        logdebug("SceneWorld: ... maps done loading");
    }

    // TODO: as soon as WMO-only worlds are implemented, remove this!!
    if(!mapmgr->GetLoadedMapsCount())
    {
        logerror("SceneWorld: Error: No maps loaded, not able to draw any terrain. Switching back GUI to idle.");
        logerror("SceneWorld: Hint: Be sure you are not in an WMO-only world (e.g. human capital city or most instances)!");
        gui->SetSceneState(SCENESTATE_GUISTART);
        return;
    }

    UpdateMapSceneNodes(_doodads); // drop doodads on maps not loaded anymore. no maptile pointers are dereferenced here, so it can be done before acquiring the mutex
    UpdateMapSceneNodes(_sound_emitters); // same with sound emitters

    mutex.acquire(); // prevent other threads from deleting maptiles

    // to set the correct position of the terrain, we have to use the top-left tile's coords as terrain base pos
    MapTile *maptile = mapmgr->GetNearTile(-1, -1);
    vector3df tpos(0,0,0); // height already managed when building up terrain (-> Y = always 0)
    if(maptile)
    {
        tpos.X = -maptile->GetBaseX();
        tpos.Z = -maptile->GetBaseY();
    }
    else if(maptile = mapmgr->GetCurrentTile()) // this is tile (0, 0) in relative coords
    {
        logdebug("SceneWorld: Using alternative coords due to missing MapTile");
        tpos.X = -(maptile->GetBaseX() + TILESIZE);
        tpos.Z = -(maptile->GetBaseY() + TILESIZE);
    }
    logdebug("SceneWorld: Setting position of terrain (x:%.2f y:%.2f z:%.2f)", tpos.X, tpos.Y, tpos.Z);
    terrain->setPosition(tpos);

    logdebug("SceneWorld: Displaying MapTiles near grids x:%u y:%u",mapmgr->GetGridX(),mapmgr->GetGridY());
    logdebug("Loaded maps: %u: %s",mapmgr->GetLoadedMapsCount(), mapmgr->GetLoadedTilesString().c_str());
    for(s32 tiley = 0; tiley < 3; tiley++)
    {
        for(s32 tilex = 0; tilex < 3; tilex++)
        {
            MapTile *maptile = mapmgr->GetNearTile(tilex - 1, tiley - 1);
            uint32 tile_real_x =  mapmgr->GetGridX() + tilex - 1;
            uint32 tile_real_y =  mapmgr->GetGridY() + tiley - 1;
            if(maptile)
            {
                // apply map height data
                logdebug("Applying height data for tile (%u, %u)", tile_real_x, tile_real_y);
                for(uint32 chy = 0; chy < 16; chy++)
                {
                    for(uint32 chx = 0; chx < 16; chx++)
                    {
                        MapChunk *chunk = maptile->GetChunk(chx, chy);
                        for(uint32 hy = 0; hy < 8; hy++)
                        {
                            for(uint32 hx = 0; hx < 8; hx++)
                            {
                                f32 h = chunk->hmap_rough[hy * 9 + hx] + chunk->baseheight; // not sure if hx and hy are used correctly here
                                u32 terrainx = (128 * tilex) + (8 * chx) + hx;
                                u32 terrainy = (128 * tiley) + (8 * chy) + hy;
                                terrain->setHeight(terrainy, terrainx, h);
                            }
                        }
                    }
                }
                // create doodads
                logdebug("Loading %u doodads for tile (%u, %u)", maptile->GetDoodadCount(), tile_real_x, tile_real_y);
                for(uint32 i = 0; i < maptile->GetDoodadCount(); i++)
                {
                    Doodad *d = maptile->GetDoodad(i);
                    if(_doodads.find(d->uniqueid) == _doodads.end()) // only add doodads that dont exist yet
                    {
                        scene::IAnimatedMesh *mesh = smgr->getMesh(d->model.c_str());
                        if(mesh)
                        {
                            scene::IAnimatedMeshSceneNode *doodad = smgr->addAnimatedMeshSceneNode(mesh);
                            if(doodad)
                            {
                                for(u32 m = 0; m < doodad->getMaterialCount(); m++)
                                {
                                    doodad->getMaterial(m).setFlag(EMF_FOG_ENABLE, true);
                                }
                                doodad->setAutomaticCulling(EAC_BOX);
                                // this is causing the framerate to drop to ~1. better leave it disabled for now :/
                                //doodad->addShadowVolumeSceneNode();
                                doodad->setPosition(core::vector3df(-d->x, d->z, -d->y));

                                // Rotation problems
                                // MapTile.cpp - changed to
                                // d.ox = mddf.c; d.oy = mddf.b; d.oz = mddf.a;
                                // its nonsense to do d.oy = mddf.b-90; and rotation with -d->oy-90 = -(mddf.b-90)-90 = -mddf.b
                                // here:
                                // doodad->setRotation(core::vector3df(-d->ox,0,-d->oz)); // rotated axes looks good
                                // doodad->setRotation(core::vector3df(0,-d->oy,0));      // same here
                                doodad->setRotation(core::vector3df(-d->ox,-d->oy,-d->oz)); // very ugly with some rotations, |ang|>360? 
                                
                                doodad->setScale(core::vector3df(d->scale, d->scale, d->scale));
                                
                                // smgr->addTextSceneNode(this->device->getGUIEnvironment()->getBuiltInFont(), (irr::core::stringw(L"")+(float)d->uniqueid).c_str() , irr::video::SColor(255,255,255,255),doodad, irr::core::vector3df(0,5,0));
                                SceneNodeWithGridPos gp;
                                gp.gx = mapmgr->GetGridX() + tilex - 1;
                                gp.gy = mapmgr->GetGridY() + tiley - 1;
                                gp.scenenode = doodad;
                                _doodads[d->uniqueid] = gp;
                            }
                        }
                    }
                }
                // create sound emitters
                logdebug("Loading %u sound emitters for tile (%u, %u)", maptile->GetSoundEmitterCount(), tile_real_x, tile_real_y);
                uint32 fieldId[10]; // SCP: file1 - file10 (index 0 not used)
                char fieldname_t[10];
                SCPDatabase *sounddb = gui->GetInstance()->dbmgr.GetDB("sound");
                if(sounddb)
                {
                    for(uint32 i = 0; i < 10; i++)
                    {
                        sprintf(fieldname_t,"file%lu",i + 1); // starts with "file1"
                        fieldId[i] = sounddb->GetFieldId(fieldname_t);
                    }

                    for(uint32 i = 0; i < maptile->GetSoundEmitterCount(); i++)
                    {
                        MCSE_chunk *snd = maptile->GetSoundEmitter(i);
                        if(_sound_emitters.find(snd->soundPointID) == _sound_emitters.end())
                        {
                            CIrrKlangSceneNode *snode = new CIrrKlangSceneNode(soundengine, smgr->getRootSceneNode(), smgr, snd->soundPointID);
                            snode->drop();
                            snode->setPosition(core::vector3df(-snd->x, snd->z, -snd->y));
                            snode->getDebugCube()->setPosition(snode->getPosition());
                            snode->setMinMaxSoundDistance(snd->minDistance,snd->maxDistance);
                            bool exists = sounddb->GetRowByIndex(snd->soundNameID);
                            if(exists)
                            {
                                for(uint32 s = 0; s < 10; s++)
                                {
                                    u32 offs = sounddb->GetInt(snd->soundNameID, fieldId[s]);
                                    if(fieldId[s] != SCP_INVALID_INT && offs && offs != SCP_INVALID_INT)
                                    {
                                        std::string fn = "data/sound/";
                                        fn += sounddb->GetString(snd->soundNameID, fieldId[s]);
                                        snode->addSoundFileName(fn.c_str());
                                    }
                                }
                                snode->setLoopingStreamMode();
                            }

                            core::stringw txt;
                            txt += (exists ? sounddb->GetString(snd->soundNameID, "name") : "[NA SoundEmitter]");
                            txt += L" (";
                            txt += u32(snd->soundNameID);
                            txt += L")";
                            snode->getDebugText()->setPosition(snode->getPosition());
                            snode->getDebugText()->setText(txt.c_str());

                            SceneNodeWithGridPos gp;
                            gp.gx = mapmgr->GetGridX() + tilex - 1;
                            gp.gy = mapmgr->GetGridY() + tiley - 1;
                            gp.scenenode = snode;
                            _sound_emitters[snd->soundPointID] = gp;
                        }

                    }
                }

            }
            else
            {
                logerror("SceneWorld: MapTile (%u, %u) not loaded!", tile_real_x, tile_real_y);
            }
        }
    }
    mutex.release();

    // find out highest/lowest spot
    f32 highest = terrain->getHeight(0,0);
    f32 lowest = terrain->getHeight(0,0);
    f32 curheight;
    for(s32 j=0; j<terrain->getSize().Height+1; j++)
        for(s32 i=0; i<terrain->getSize().Width+1; i++)
        {
            curheight = terrain->getHeight(i,j);
            highest = MAX(highest,curheight);
            lowest = MIN(lowest,curheight);
        }
//    f32 heightdiff = highest - lowest;

    // randomize terrain color depending on height
    for(s32 j=0; j<terrain->getSize().Height+1; j++)
        for(s32 i=0; i<terrain->getSize().Width+1; i++)
        {
            curheight = terrain->getHeight(i,j);
            u32 g = (u32)(curheight / highest * 120) + 125;
            u32 r = (u32)(curheight / highest * 120) + 60;
            u32 b = (u32)(curheight / highest * 120) + 60;

            terrain->setColor(i,j, video::SColor(255,r,g,b));
        }

    logdebug("SceneWorld: Smoothing terrain normals...");
    terrain->smoothNormals();

    // TODO: check if camera should really be relocated -> in case we got teleported
    // do NOT relocate camera if we moved around and triggered the map loading code by ourself!
    RelocateCameraBehindChar();
}

// drop unneeded map SceneNodes from the map
void SceneWorld::UpdateMapSceneNodes(std::map<uint32,SceneNodeWithGridPos>& node_map)
{
    uint32 s = node_map.size();
    std::set<uint32> tmp; // temporary storage for all doodad unique ids
    // too bad erasing from a map causes pointer invalidation, so first store all unique ids, and then erase
    for(std::map<uint32,SceneNodeWithGridPos>::iterator it = node_map.begin(); it != node_map.end(); it++ )
        if(!mapmgr->GetTile(it->second.gx, it->second.gy))
            tmp.insert(it->first);
    for(std::set<uint32>::iterator it = tmp.begin(); it != tmp.end(); it++)
    {
        node_map[*it].scenenode->remove();
        node_map.erase(*it);
    }
    logdebug("SceneWorld: MapSceneNodes cleaned up, before: %u, after: %u, dropped: %u", s, node_map.size(), s - node_map.size());
}


void SceneWorld::RelocateCamera(void)
{

    MyCharacter *my = wsession->GetMyChar();
    if(my)
    {
        //logdebug("SceneWorld: Relocating camera to MyCharacter");
        camera->setPosition(vector3df(-my->GetX(),my->GetZ(),-my->GetY()));
        camera->turnLeft(camera->getHeading() - RAD_TO_DEG(PI*3/2 - my->GetO()));
    }
    else
    {
        logerror("SceneWorld: Relocating camera to MyCharacter - not found!");
    }
}

// TODO: call this func only when really needed, and not in every loop?
void SceneWorld::RelocateCameraBehindChar(void)
{
    if(mychar)
    {
        float distance = (MAX_CAM_DISTANCE / 5.0f) - (eventrecv->mouse.wheel / 5.0f);
        //DEBUG(logdebug("SceneWorld: Relocating camera behind MyCharacter, dist %.2f",distance));

        // TODO: partial transparency for near character zoom (TEST) [fg]
        // didnt work at all, so if somebody knows how to set a model transparent please fix this!
        /*
        if(distance <= 4)
        {
            scene::ISceneNode *charnode = GetMyCharacterSceneNode(); // NOTE: this call is absolutely not optimized!
            if(charnode)
            {
                charnode->getMaterial(0).MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
                charnode->getMaterial(0).AmbientColor.setAlpha(distance * 50);
                charnode->getMaterial(0).DiffuseColor.setAlpha(distance * 50);
            }
        }
        */
        // WORKAROUND: if camera is too near, just make our player invisible
        scene::ISceneNode *charnode = GetMyCharacterSceneNode(); // NOTE: this call is absolutely not optimized!
        if(charnode)
        {
            if(distance <= 0.25f)
                charnode->setVisible(false);
            else
                charnode->setVisible(true);
        }

        if(mouse_pressed_left)
        {
            camera->setPosition(vector3df(-mychar->GetX(), mychar->GetZ() + distance + 1.5f, -mychar->GetY()));
            camera->moveBack(distance);
        }
        else
        {
            camera->setPosition(vector3df(-mychar->GetX(), mychar->GetZ() + distance + 1.5f, -mychar->GetY()));
            camera->turnLeft(camera->getHeading() - RAD_TO_DEG(PI*3/2 - mychar->GetO()));
            camera->moveBack(distance);
        }
    }
    else
    {
        logerror("SceneWorld: Relocating camera behind MyCharacter - not found!");
    }
}

WorldPosition SceneWorld::GetWorldPosition(void)
{
    // TODO: later do not use CAMERA, but CHARACTER position, as soon as camera is changed from 1st to 3rd person view
    // and floating around character in the middle
    return IrrToWP(camera->getPosition(), DEG_TO_RAD(camera->getHeading()));
}

video::SColor SceneWorld::GetBackgroundColor(void)
{
    return envBasicColor;
}

void SceneWorld::_CalcXYMoveVect(float o)
{
    xyCharMovement.X = cos(o);
    xyCharMovement.Y = sin(o);
}

scene::ISceneNode *SceneWorld::GetMyCharacterSceneNode(void)
{
    DrawObject *d = gui->domgr.Get(mychar->GetGUID());
    return d ? d->GetSceneNode() : NULL;
}


