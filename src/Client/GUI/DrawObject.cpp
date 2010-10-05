#include "common.h"
#include "PseuGUI.h"
#include "DrawObject.h"
#include "PseuWoW.h"
#include "Object.h"
#include "Player.h"
#include "GameObject.h"
#include "WorldSession.h"
#include "MemoryInterface.h"
#include "MemoryDataHolder.h"

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
    if(node)
    {
        text->remove();
        node->remove();
    }
}

void DrawObject::Unlink(void)
{
    node = NULL;
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

    if(!node && _obj->IsWorldObject()) // only world objects have coords and can be drawn
    {
        std::string modelfilename, texturename = "";
        uint32 opacity = 255;
        if (_obj->IsUnit())
        {
            uint32 displayid = _obj->GetUInt32Value(UNIT_FIELD_DISPLAYID);
            SCPDatabase *cdi = _instance->dbmgr.GetDB("creaturedisplayinfo");
            SCPDatabase *cmd = _instance->dbmgr.GetDB("creaturemodeldata");
            uint32 modelid = cdi && displayid ? cdi->GetUint32(displayid,"model") : 0;
            logdebug("modelid = %u, displayid = %u",modelid,displayid);
            modelfilename = (cmd ? cmd->GetString(modelid,"file") : "");
            char buf[1000];
            MemoryDataHolder::MakeModelFilename(buf, modelfilename);
            modelfilename = buf;
            logdebug("Unit %s", modelfilename);
//             if (cdi && strcmp(cdi->GetString(displayid,"name1"), "") != 0) 
//                 texturename = std::string("data/texture/") + cdi->GetString(displayid,"name1");
            opacity = cdi && displayid ? cdi->GetUint32(displayid,"opacity") : 255;
        } 
        else if (_obj->IsCorpse())
        {
            uint8 race = (_obj->GetUInt32Value(CORPSE_FIELD_BYTES_1) >> 8)&0xFF;
            uint8 gender = (_obj->GetUInt32Value(CORPSE_FIELD_BYTES_1) >> 16)&0xFF;
            std::string racename = "", gendername = "";

            SCPDatabase *scprace = _instance->dbmgr.GetDB("race");
            SCPDatabase *scpgender = _instance->dbmgr.GetDB("gender");
            if (scprace)
                racename = scprace->GetString(race, "name_general");
            if (scpgender)
                gendername = scpgender->GetString(gender, "name");

            // TODO: fix this for non-MPQ mode
            modelfilename = std::string("World\\Generic\\PassiveDoodads\\DeathSkeletons\\") + racename + gendername + "DeathSkeleton.m2";
            char buf[1000];
            MemoryDataHolder::MakeModelFilename(buf,modelfilename);
            modelfilename = buf;
            logdebug("Corpse %s",buf);

        
        }
        else if (_obj->IsGameObject())
        {
            GameobjectTemplate* gotempl = _instance->GetWSession()->objmgr.GetGOTemplate(_obj->GetEntry());
            while (!gotempl) 
            {
                ZThread::Thread::sleep(10);
                gotempl = _instance->GetWSession()->objmgr.GetGOTemplate(_obj->GetEntry());
            }
            if (gotempl)
            {
                // GAMEOBJECT_TYPE_TRAP
                if (gotempl->type == 6) // damage source on fires, skip for now
                {
                    _initialized = true;
                    return;
                }

                uint32 displayid = gotempl->displayId;
                SCPDatabase *gdi = _instance->dbmgr.GetDB("gameobjectdisplayinfo");
                if (gdi && displayid)
                {
                    char buf[1000];
                    MemoryDataHolder::MakeModelFilename(buf,gdi->GetString(displayid,"model"));
                    modelfilename = buf;
                    logdebug("Gameobject %s",buf);

                    if (strcmp(gdi->GetString(displayid,"texture"), "") != 0)
                    {
                        char buf[1000];
                        MemoryDataHolder::MakeTextureFilename(buf,gdi->GetString(displayid,"texture"));
                        texturename = buf;
                    }
                }
                
                DEBUG(logdebug("GAMEOBJECT: %u - %u", _obj->GetEntry(), displayid));
            } else {
                DEBUG(logdebug("GAMEOBJECT UNKNOWN: %u", _obj->GetEntry()));
            }
        }

        io::IReadFile* modelfile = io::IrrCreateIReadFileBasic(_device, modelfilename.c_str());
        if (!modelfile)
            {
                logerror("DrawObject: model file not found: %s", modelfilename.c_str());
            }
        scene::IAnimatedMesh *mesh = _smgr->getMesh(modelfile);


        if(mesh)
        {
            node = _smgr->addAnimatedMeshSceneNode(mesh);
            //video::ITexture *tex = _device->getVideoDriver()->getTexture("data/misc/square.jpg");
            //node->setMaterialTexture(0, tex);
        }
        else
        {
            node = _smgr->addCubeSceneNode(1);
        }
        if (!texturename.empty())
        {
            logdebug("%s",texturename.c_str());
            io::IReadFile* texturefile = io::IrrCreateIReadFileBasic(_device, texturename.c_str());
            if (!texturefile)
                {
                    logerror("DrawObject: texture file not found: %s", texturename.c_str());
                }   

            node->setMaterialTexture(0, _device->getVideoDriver()->getTexture(texturefile));
        }
        //node->getMaterial(0).DiffuseColor.setAlpha(opacity);
        node->setName("OBJECT");
        if (node->getMaterialCount())
        {
            node->getMaterial(0).setFlag(video::EMF_LIGHTING, true);
            node->getMaterial(0).setFlag(video::EMF_FOG_ENABLE, true);
        }

        text=_smgr->addTextSceneNode(_guienv->getBuiltInFont(), L"TestText" , irr::video::SColor(255,255,255,255),node, irr::core::vector3df(0,5,0));
        if(_obj->IsPlayer())
        {
            text->setTextColor(irr::video::SColor(255,255,0,0));
        }
        else if(_obj->IsCreature())
        {
            text->setTextColor(irr::video::SColor(255,0,0,255));
        }

    }
    logdebug("initialize DrawObject 0x%X obj: 0x%X "I64FMT,this,_obj,_obj->GetGUID());

    _initialized = true;
}

void DrawObject::Draw(void)
{
    if(!_initialized)
        _Init();

    //printf("DRAW() for pObj 0x%X name '%s' guid "I64FMT"\n", _obj, _obj->GetName().c_str(), _obj->GetGUID());
    if(node)
    {
        WorldPosition pos = ((WorldObject*)_obj)->GetPosition();
        node->setPosition(WPToIrr(pos));
        rotation.Y = O_TO_IRR(pos.o);

        float s = _obj->GetFloatValue(OBJECT_FIELD_SCALE_X);
        if(s <= 0)
            s = 1;
        node->setScale(irr::core::vector3df(s,s,s));
        node->setRotation(rotation);

        //node->setRotation(irr::core::vector3df(0,RAD_TO_DEG(((WorldObject*)_obj)->GetO()),0));
        irr::core::stringw tmp = L"";
        if(_obj->GetName().empty() && !_obj->IsCorpse())
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

