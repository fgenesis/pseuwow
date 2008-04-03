#include "common.h"
#include "PseuGUI.h"
#include "DrawObject.h"
#include "PseuWoW.h"
#include "Object.h"

using namespace irr;

DrawObject::DrawObject(irr::IrrlichtDevice *device, Object *obj, PseuInstance *ins)
{
    _initialized = false;
    Unlink();
    _device = device;
    _smgr = device->getSceneManager();
    _guienv = device->getGUIEnvironment();
    _obj = obj;
    _instance = ins;
    DEBUG( logdebug("create DrawObject() this=%X obj=%X name='%s' smgr=%X",this,_obj,_obj->GetName().c_str(),_smgr) );
}

DrawObject::~DrawObject()
{
    DEBUG( logdebug("~DrawObject() this=0x%X obj=0x%X smgr=%X",this,_obj,_smgr) );
    if(cube)
    {
        text->remove();
        cube->remove();
    }
}

void DrawObject::Unlink(void)
{
    cube = NULL;
    text = NULL;
}

void DrawObject::_Init(void)
{
    if(!cube && _obj->IsWorldObject()) // only world objects have coords and can be drawn
    {
        uint32 displayid = _obj->IsUnit() ? _obj->GetUInt32Value(UNIT_FIELD_DISPLAYID) : 0; // TODO: in case its GO get it from proto data
        uint32 modelid = _instance->dbmgr.GetDB("creaturedisplayinfo").GetField(displayid).GetInteger("model");
        std::string modelfile = std::string("data/model/") + _instance->dbmgr.GetDB("creaturemodeldata").GetField(modelid).GetString("file");
        //scene::IAnimatedMesh *mesh = _smgr->getMesh(modelfile.c_str());
        // ok we use a model that works for sure until crashs with some M2 files, especially character models, are fixed
        /*scene::IAnimatedMesh *mesh = _smgr->getMesh("data/model/gyrocopter.m2");
        if(mesh)
        {
            cube = _smgr->addAnimatedMeshSceneNode(mesh);
            //video::ITexture *tex = _device->getVideoDriver()->getTexture("data/misc/square.jpg");
            //cube->setMaterialTexture(0, tex);
            //tex->drop();
        }
        else
        {*/
            cube = _smgr->addCubeSceneNode(2);
        //}
        cube->setName("OBJECT");
        //cube->setPosition(irr::core::vector3di(100,100,100));
        cube->setRotation(core::vector3df(0,0,0));

        if(_obj->IsPlayer())
        {
            cube->getMaterial(0).DiffuseColor.set(255,255,0,0);
        }
        else if(_obj->IsCreature())
        {
            cube->getMaterial(0).DiffuseColor.set(255,0,255,0);
        }
        text=_smgr->addTextSceneNode(_guienv->getBuiltInFont(), L"TestText" , irr::video::SColor(255,255,255,255),cube, irr::core::vector3df(0,5,0));
    }
    DEBUG(logdebug("initialize DrawObject 0x%X obj: 0x%X "I64FMT,this,_obj,_obj->GetGUID()))

    _initialized = true;
}

void DrawObject::Draw(void)
{
    if(!_initialized)
        _Init();

    //printf("DRAW() for pObj 0x%X name '%s' guid "I64FMT"\n", _obj, _obj->GetName().c_str(), _obj->GetGUID());
    if(cube)
    {
        WorldPosition pos = ((WorldObject*)_obj)->GetPosition();
        cube->setPosition(irr::core::vector3df(-pos.x,pos.z,-pos.y));
        if(_obj->IsPlayer())
        {
            cube->getMaterial(0).DiffuseColor.set(255,255,0,0);
            text->setTextColor(irr::video::SColor(255,255,0,0));
        }
        else if(_obj->IsCreature())
        {
            cube->getMaterial(0).DiffuseColor.set(255,0,0,255);
            text->setTextColor(irr::video::SColor(255,0,0,255));
        }

        float s = _obj->GetFloatValue(OBJECT_FIELD_SCALE_X);
        if(s <= 0)
            s = 1;
        cube->setScale(irr::core::vector3df(s,s,s));

        //cube->setRotation(irr::core::vector3df(0,RAD_TO_DEG(((WorldObject*)_obj)->GetO()),0));
        irr::core::stringw tmp = L"";
        if(_obj->GetName().empty())
        {
            tmp += L"unk<";
            tmp += _obj->GetTypeId();
            tmp += L">";
        }
        else
        {
            tmp += _obj->GetName().c_str();
        }
        text->setText(tmp.c_str());
    }
}

