#include "GameObject.h"

GameObject::GameObject() : WorldObject()
{
    _uint32values=NULL;
    _type=TYPE_GAMEOBJECT;
    _typeid=TYPEID_GAMEOBJECT;
    _valuescount=GAMEOBJECT_END;
}

void GameObject::Create(uint64 guid)
{
    Object::Create(guid);
}
