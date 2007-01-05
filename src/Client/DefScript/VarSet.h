#ifndef __VARSET_H
#define __VARSET_H

#include <string>
#include <list>


struct Var {
    std::string name, value;
};
	

class VarSet {
public:
    void Set(std::string,std::string);
    std::string Get(std::string);
	void Clear(void);
	void Unset(std::string);
	unsigned int Size(void);
	bool Exists(std::string);
    bool ReadVarsFromFile(std::string fn);
	VarSet();
	~VarSet();
	// far future: MergeWith(VarSet,bool overwrite);

private:
    std::list<Var> buffer;
    std::string toLower(std::string);
    std::string toUpper(std::string);

	
};


#endif