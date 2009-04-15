#include "common.h"
#include "MemoryDataHolder.h"
#include "irrlicht/irrlicht.h"
#include "CMDHMemoryReadFile.h"
#include "MemoryInterface.h"

/*
struct IrrFileLoadedCallback
{
    irr::IrrlichtDevice *device;
};

struct IrrTextureLoadedCallback : public IrrFileLoadedCallback
{
    irr::scene::ISceneNode *node;
};


void IrrModelLoadedCallbackFunc(void *ptr,std::string filename, uint32 flags)
{
}
*/

namespace irr
{
namespace io
{


IReadFile * IrrCreateIReadFileBasic(irr::IrrlichtDevice *device, std::string fn)
{
    MemoryDataHolder::memblock mb = MemoryDataHolder::GetFileBasic(fn);
    if(!mb.ptr)
        return NULL;

    return new CMDHReadFile(mb.ptr, mb.size, fn.c_str());
}
  

}
}
