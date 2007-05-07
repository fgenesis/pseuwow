#ifndef MPQHELPER_H
#define MPQHELPER_H

#define MAX_PATCH_NUMBER 9

class MPQFile;

class MPQHelper
{
public:
    MPQHelper(char*);
    ~MPQHelper();
    ByteBuffer ExtractFile(char*);
    bool FileExists(char*);
private:
    std::list<MPQFile*> _files;
    std::list<std::string> _patches;
};

#endif