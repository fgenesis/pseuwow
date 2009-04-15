#ifndef MEMORYINTERFACE_H
#define MEMORYINTERFACE_H



namespace irr
{

class IrrlichtDevice;

namespace io
{

class IReadFile;


IReadFile *IrrCreateIReadFileBasic(IrrlichtDevice*, std::string);


}
}


#endif
