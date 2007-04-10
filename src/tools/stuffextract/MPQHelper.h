#ifndef MPQHELPER_H
#define MPQHELPER_H

#define MAX_PATCH_NUMBER 9

class MPQHelper
{
public:
    MPQHelper();
    bool AssignArchive(char*);
    ByteBuffer ExtractFile(char*);
private:
    std::list<std::string> _patches; // patch.mpq - patch-9.mpq
    std::string _archive;
};

#endif