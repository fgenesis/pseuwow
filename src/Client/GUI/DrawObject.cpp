#include "common.h"
#include "PseuGui.h"
#include "DrawObject.h"
#include "PseuWoW.h"

DrawObject::DrawObject(irr::scene::ISceneManager *smgr, Object *obj)
{
    _smgr = smgr;
    _obj = obj;
    DEBUG( logdebug("create DrawObject() this=%X obj=%X smgr=%X",this,_obj,_smgr) );
}

DrawObject::~DrawObject()
{
    DEBUG( logdebug("~DrawObject() this=%X obj=%X smgr=%X",this,_obj,_smgr) );
}

void DrawObject::Draw(void)
{

}
