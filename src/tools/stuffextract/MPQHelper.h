#ifndef MPQHELPER_H
#define MPQHELPER_H

#define MAX_PATCH_NUMBER 9

class MPQFile;

class MPQHelper
{
public:
    MPQHelper(const char*);
    ~MPQHelper();
    ByteBuffer ExtractFile(const char*);
    bool FileExists(const char*);
private:
    std::list<MPQFile*> _files;
    std::list<std::string> _patches;
};

#endif
