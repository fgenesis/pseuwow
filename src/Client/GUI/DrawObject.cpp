#include "common.h"
#include "PseuGUI.h"
#include "DrawObject.h"
#include "PseuWoW.h"
#include "Object.h"
#include "Player.h"

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
    if(_obj->IsPlayer())
    {
        Player *p = (Player*)_obj;
        DEBUG(logdebug("Player: race=%u gender=%u face=%u skin=%u traits=%u hair=%u haircol=%u",
            p->GetRace(),p->GetGender(),p->GetFaceId(),p->GetSkinId(),p->GetFaceTraitsId(),p->GetHairStyleId(),p->GetHairColorId()));
    }

    if(!cube && _obj->IsWorldObject()) // only world objects have coords and can be drawn
    {
        uint32 displayid = _obj->IsUnit() ? _obj->GetUInt32Value(UNIT_FIELD_DISPLAYID) : 0; // TODO: in case its GO get it from proto data
        SCPDatabase& cdi = _instance->dbmgr.GetDB("creaturedisplayinfo");
        SCPField& crdata = cdi.GetField(displayid);
        uint32 modelid = crdata.GetInteger("model");
        std::string modelfile = std::string("data/model/") + _instance->dbmgr.GetDB("creaturemodeldata").GetField(modelid).GetString("file");
        uint32 opacity = crdata.GetInteger("opacity");
        scene::IAnimatedMesh *mesh = _smgr->getMesh(modelfile.c_str());
        if(mesh)
        {
            rotation.X = 270.0f; // M2 models are stored "lying on the side" - this puts them standing
                                 // ok, this f*cks up the text scene node, but shouldnt be such a problem right now,
                                 // until the M2-loader has been corrected so far that this line can be removed
            cube = _smgr->addAnimatedMeshSceneNode(mesh);
            //video::ITexture *tex = _device->getVideoDriver()->getTexture("data/misc/square.jpg");
            //cube->setMaterialTexture(0, tex);
        }
        else
        {
            cube = _smgr->addCubeSceneNode(2);
        }
        //cube->getMaterial(0).DiffuseColor.setAlpha(opacity);
        cube->setName("OBJECT");
        //cube->getMaterial(0).setFlag(video::EMF_LIGHTING, true);
        //cube->getMaterial(0).setFlag(video::EMF_FOG_ENABLE, true);

        text=_smgr->addTextSceneNode(_guienv->getBuiltInFont(), L"TestText" , irr::video::SColor(255,255,255,255),cube, irr::core::vector3df(0,5,0));
        if(_obj->IsPlayer())
        {
            text->setTextColor(irr::video::SColor(255,255,0,0));
        }
        else if(_obj->IsCreature())
        {
            text->setTextColor(irr::video::SColor(255,0,0,255));
        }

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
        cube->setPosition(WPToIrr(pos));
        rotation.Y = O_TO_IRR(pos.o);

        float s = _obj->GetFloatValue(OBJECT_FIELD_SCALE_X);
        if(s <= 0)
            s = 1;
        cube->setScale(irr::core::vector3df(s,s,s));
        cube->setRotation(rotation);

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

