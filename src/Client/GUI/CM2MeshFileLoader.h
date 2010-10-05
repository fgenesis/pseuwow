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
	u32 nBones;
	u32 ofsBones;
	u32 nF;
	u32 ofsF;

	u32 nVertices;
	u32 ofsVertices;
	u32 nViews; // number of skins ?

	u32 nColors;
	u32 ofsColors;

	u32 nTextures;
	u32 ofsTextures;

	u32 nTransparency; // H
	u32 ofsTransparency;
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

struct ModelVertex {
	core::vector3df pos;
	u8 weights[4];
	u8 bones[4];
	core::vector3df normal;
	core::vector2df texcoords;
	u32 unk1, unk2; // always 0,0 so this is probably unused
};

struct ModelView {
    c8 id[4]; // always "SKIN"
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

struct Animation{
    u32 animationID;
    u32 start, end;
    float movespeed;
    u32 loop, flags, unk1, unk2;
    u32 playbackspeed;
    float bbox[6];
    float radius;
    s16 indexSameID;
    u16 unk3;
};

struct AnimBlockHead{
    s16 interpolationType;
    s16 globalSequenceID;
    u32 nInterpolationRange;
    u32 ofsInterpolationRange;
    u32 nTimeStamp;
    u32 ofsTimeStamp;
    u32 nValues;
    u32 ofsValues;
};

struct InterpolationRange{
    u32 start, end;
};

struct AnimBlock{
    AnimBlockHead header;
    core::array<InterpolationRange> keyframes;
    core::array<u32> timestamps;
    core::array<float> values;
};

struct Bone{
    s32 indexF;
    u32 flags;
    s16 parentBone;
    u16 unk1;
    u32 unk2;
    AnimBlock translation, rotation, scaling;
    core::vector3df PivotPoint;
};


class CM2MeshFileLoader : public IMeshLoader
{
public:

	//! Constructor
	CM2MeshFileLoader(IrrlichtDevice* device);

	//! destructor
	virtual ~CM2MeshFileLoader();

	//! returns true if the file maybe is able to be loaded by this class
	//! based on the file extension (e.g. ".cob")
	virtual bool isALoadableFileExtension(const c8* fileName)const;

	//! creates/loads an animated mesh from the file.
	//! \return Pointer to the created mesh. Returns 0 if loading failed.
	//! If you no longer need the mesh, you should call IAnimatedMesh::drop().
	//! See IUnknown::drop() for more information.
	virtual scene::IAnimatedMesh* createMesh(io::IReadFile* file);
private:

	bool load();

	IrrlichtDevice* Device;
    core::stringc Texdir;
    io::IReadFile* MeshFile;

    CSkinnedMesh* AnimatedMesh;
    scene::CSkinnedMesh::SJoint* ParentJoint;



    ModelHeader header;
    core::stringc M2MeshName;
    SMesh* Mesh;
    //SSkinMeshBuffer* MeshBuffer;
    //Taken from the Model file, thus m2M*
    core::array<ModelVertex> M2MVertices;
    core::array<u16> M2MIndices;
    core::array<u16> M2MTriangles;
    core::array<ModelViewSubmesh> M2MSubmeshes;
    core::array<u16> M2MTextureLookup;
    core::array<TextureDefinition> M2MTextureDef;
    core::array<std::string> M2MTextureFiles;
    core::array<TextureUnit> M2MTextureUnit;
    core::array<RenderFlags> M2MRenderFlags;
    core::array<Animation> M2MAnimations;
    core::array<Bone> M2MBones;
    //Used for the Mesh, thus m2_noM_*
    core::array<video::S3DVertex> M2Vertices;
    core::array<u16> M2Indices;
    core::array<scene::ISkinnedMesh::SJoint> M2Joints;


};
}//namespace scene
}//namespace irr
