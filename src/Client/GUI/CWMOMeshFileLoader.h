#include "irrlicht/irrlicht.h"
#include "irrlicht/IMeshLoader.h"
#include "SSkinnedMesh.h"
#include <string>
#include <vector>
#include <algorithm>

namespace irr
{
namespace scene
{

struct RootHeader {
/*000h*/  u32 nTextures;
/*004h*/  u32 nGroups;
/*008h*/  u32 nPortals;
/*00Ch*/  u32 nLights;
/*010h*/  u32 nModels;
/*014h*/  u32 nDoodads;
/*018h*/  u32 nSets;
/*01Ch*/  u8  colR;
/*01Dh*/  u8  colG;
/*01Eh*/  u8  colB;
/*01Fh*/  u8  colX;
/*020h*/  u32 wmoID;
/*024h*/  float  bb1[3];
/*030h*/  float  bb2[3];
/*03Ch*/  u32 nullish;
};
struct WMOColor{
    u8 b,g,r,a;

};
struct MOPY_Data{
    u8 flags,textureID;

};
struct MOMT_Data{
/*000h*/  u32 flags1;
/*004h*/  u32 flags2;
/*008h*/  u32 blendMode;
/*00Ch*/  u32 startNameIndex;
/*010h*/  u32 color;
/*014h*/  u32 unk1;
/*018h*/  u32 endNameIndex;
/*01Ch*/  u32 frameSidnColor[3];
/*020h*/
/*024h*/
/*028h*/  u32 envNameIndex;
/*02Ch*/  float diffColor[3];
/*030h*/
/*034h*/
/*038h*/  u32 groundType;
/*03Ch*/  u32 hMaps;

};


class CWMOMeshFileLoader : public IMeshLoader
{
public:

	//! Constructor
	CWMOMeshFileLoader(IrrlichtDevice* device, c8* texdir);

	//! destructor
	virtual ~CWMOMeshFileLoader();

	//! returns true if the file maybe is able to be loaded by this class
	//! based on the file extension (e.g. ".cob")
	virtual bool isALoadableFileExtension(const c8* fileName)const;

	//! creates/loads an animated mesh from the file.
	//! \return Pointer to the created mesh. Returns 0 if loading failed.
	//! If you no longer need the mesh, you should call IAnimatedMesh::drop().
	//! See IUnknown::drop() for more information.
	virtual scene::IAnimatedMesh* createMesh(io::IReadFile* file);
private:

	bool load(bool _root);

	IrrlichtDevice* Device;
    core::stringc Texdir;
    io::IReadFile* MeshFile;

    CSkinnedMesh* Mesh;


//Stuff from root file
    RootHeader rootHeader;
    core::array<MOMT_Data> WMOMTexDefinition;
    core::array<std::string> WMOMTextureFiles;


//Stuff from group file
    core::array<u16> WMOMIndices;
    core::array<core::vector3df> WMOMVertices;
    core::array<core::vector3df> WMOMNormals;
    core::array<core::vector2df> WMOMTexcoord;
    core::array<video::SColor> WMOMVertexColor;
    core::array<MOPY_Data> WMOMTexData;
    core::array<u16> submeshes;

    core::array<video::S3DVertex> WMOVertices;
    SSkinMeshBuffer* MeshBuffer;
/*
    ModelHeader header;
    core::stringc WMOMeshName;
    SMesh* Mesh;
    //
    //Taken from the Model file, thus m2M*
    core::array<ModelVertex> WMOMVertices;
    core::array<ModelView> WMOMViews;
    core::array<u16> WMOMIndices;
    core::array<u16> WMOMTriangles;
    core::array<ModelViewSubmesh> WMOMSubmeshes;
    core::array<u16> WMOMTextureLookup;
    core::array<TextureDefinition> WMOMTextureDef;
    core::array<std::string> WMOMTextureFiles;
    core::array<TextureUnit> WMOMTextureUnit;
    core::array<RenderFlags> WMOMRenderFlags;
    core::array<Animation> WMOMAnimations;
    core::array<Bone> WMOMBones;
    //Used for the Mesh, thus m2_noM_*
    core::array<video::S3DVertex> WMOVertices;
    core::array<u16> WMOIndices;
    core::array<scene::ISkinnedMesh::SJoint> WMOJoints;
*/

};
}//namespace scene
}//namespace irr
