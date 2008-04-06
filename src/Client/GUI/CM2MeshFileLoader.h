#include "irrlicht/irrlicht.h"
#include "irrlicht/IMeshLoader.h"
#include <string>
#include <vector>
#include <algorithm>

namespace irr
{
namespace scene
{

struct ModelHeader {
	c8 id[4];
	u8 version[4];
	u32 nameLength;
	u32 nameOfs;
	u32 type;

	u32 nGlobalSequences;
	u32 ofsGlobalSequences;
	u32 nAnimations;
	u32 ofsAnimations;
	u32 nC;
	u32 ofsC;
	u32 nD;
	u32 ofsD;
	u32 nBones;
	u32 ofsBones;
	u32 nF;
	u32 ofsF;

	u32 nVertices;
	u32 ofsVertices;
	u32 nViews;
	u32 ofsViews;

	u32 nColors;
	u32 ofsColors;

	u32 nTextures;
	u32 ofsTextures;

	u32 nTransparency; // H
	u32 ofsTransparency;
	u32 nI;   // always unused ?
	u32 ofsI;
	u32 nTexAnims;	// J
	u32 ofsTexAnims;
	u32 nTexReplace;
	u32 ofsTexReplace;

	u32 nTexFlags;
	u32 ofsTexFlags;
	u32 nY;
	u32 ofsY;

	u32 nTexLookup;
	u32 ofsTexLookup;

	u32 nTexUnitLookup;		// L
	u32 ofsTexUnitLookup;
	u32 nTransparencyLookup; // M
	u32 ofsTransparencyLookup;
	u32 nTexAnimLookup;
	u32 ofsTexAnimLookup;

	f32 floats[14];

	u32 nBoundingTriangles;
	u32 ofsBoundingTriangles;
	u32 nBoundingVertices;
	u32 ofsBoundingVertices;
	u32 nBoundingNormals;
	u32 ofsBoundingNormals;

	u32 nAttachments; // O
	u32 ofsAttachments;
	u32 nAttachLookup; // P
	u32 ofsAttachLookup;
	u32 nQ; // Q
	u32 ofsQ;
	u32 nLights; // R
	u32 ofsLights;
	u32 nCameras; // S
	u32 ofsCameras;
	u32 nT;
	u32 ofsT;
	u32 nRibbonEmitters; // U
	u32 ofsRibbonEmitters;
	u32 nParticleEmitters; // V
	u32 ofsParticleEmitters;

};

struct TextureDefinition {
    u32 texType;
    u16 unk;
    u16 texFlags;
    u32 texFileLen;
    u32 texFileOfs;
};

class CM2MeshFileLoader : public IMeshLoader
{
public:

	//! Constructor
	CM2MeshFileLoader(IrrlichtDevice* device, c8* texdir);

	//! destructor
	virtual ~CM2MeshFileLoader();

	//! returns true if the file maybe is able to be loaded by this class
	//! based on the file extension (e.g. ".cob")
	virtual bool isALoadableFileExtension(const c8* fileName)const;

	//! creates/loads an animated mesh from the file.
	//! \return Pointer to the created mesh. Returns 0 if loading failed.
	//! If you no longer need the mesh, you should call IAnimatedMesh::drop().
	//! See IUnknown::drop() for more information.
	virtual scene::IAnimatedMesh* createMesh(irr::io::IReadFile* file);
private:
    ModelHeader header;

struct ModelVertex {
	core::vector3df pos;//Use Irrlicht Vector here!
	u8 weights[4];
	u8 bones[4];
	core::vector3df normal;//Use Irrlicht Vector here!
	core::vector2df texcoords;//Use Irrlicht Vector here!
	u32 unk1, unk2; // always 0,0 so this is probably unused
};

struct ModelView {
    u32 nIndex, ofsIndex; // Vertices in this model (index into vertices[])
    u32 nTris, ofsTris;	 // indices
    u32 nProps, ofsProps; // additional vtx properties
    u32 nSub, ofsSub;	 // materials/renderops/submeshes
    u32 nTex, ofsTex;	 // material properties/textures
	s32 lod;				 // LOD bias?
};

struct ModelViewSubmesh {
    u32 meshpartId;
    u16 ofsVertex;//Starting vertex number for this submesh
    u16 nVertex;
    u16 ofsTris;//Starting Triangle index
    u16 nTris;
    u16 unk1, unk2, unk3, unk4;
    core::vector3df v;
    float unkf[4];
};

struct TextureUnit{
    u16 Flags;
    s16 renderOrder;
    u16 submeshIndex1, submeshIndex2;
    s16 colorIndex;
    u16 renderFlagsIndex;
    u16 TextureUnitNumber;
    u16 unk1;
    u16 textureIndex;
    u16 TextureUnitNumber2;
    u16 transparencyIndex;
    u16 texAnimIndex;
};

struct RenderFlags{
    u16 flags;
    u16 blending;
};

//
	io::IFileSystem* FileSystem;
	IrrlichtDevice* Device;
//    scene::IMeshManipulator* Manipulator;
    core::stringc M2MeshName;
    core::stringc Texdir;
    SAnimatedMesh* aniMesh;
    SMesh* Mesh;
    SMeshBuffer* IMB;
    //Taken from the Model file, thus m2M*
    core::array<ModelVertex> M2MVertices;
    core::array<ModelView> M2MViews;
    core::array<u16> M2MIndices;
    core::array<u16> M2MTriangles;
    core::array<ModelViewSubmesh> M2MSubmeshes;
    core::array<u16> M2MTextureLookup;
    core::array<TextureDefinition> M2MTextureDef;
    core::array<std::string> M2MTextureFiles;
    core::array<TextureUnit> M2MTextureUnit;
    core::array<RenderFlags> M2MRenderFlags;
    //Used for the Mesh, thus m2_noM_*
    core::array<video::S3DVertex> M2Vertices;
    core::array<u16> M2Indices;

};
}//namespace scene
}//namespace irr
