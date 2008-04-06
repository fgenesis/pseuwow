#include <iostream>
#include "CM2MeshFileLoader.h"



namespace irr
{
namespace scene
{

CM2MeshFileLoader::CM2MeshFileLoader(IrrlichtDevice* device, c8* texdir):Device(device), Texdir(texdir)
{
    Mesh = NULL;

}

CM2MeshFileLoader::~CM2MeshFileLoader()
{

}


bool CM2MeshFileLoader::isALoadableFileExtension(const c8* filename)const
{
	return strstr(filename, ".m2")!=0;
}


//! creates/loads an animated mesh from the file.
//! \return Pointer to the created mesh. Returns 0 if loading failed.
//! If you no longer need the mesh, you should call IAnimatedMesh::drop().
//! See IUnknown::drop() for more information.
IAnimatedMesh* CM2MeshFileLoader::createMesh(io::IReadFile* file)
{
ILogger* logger =Device->getLogger();

logger->log("Trying to open file",file->getFileName(),ELL_INFORMATION);


file->read(&header,sizeof(ModelHeader));
 if (header.version[0] != 4 && header.version[1] != 1 && header.version[2] != 0 && header.version[3] != 0) {
     logger->log("Something wrong!",ELL_ERROR);
     return 0;
     }
     else logger->log(L"header okay",ELL_INFORMATION);
//Name -> not very important I think, but save it nontheless;
std::cout << "Name offset:" << header.nameOfs << "Name length:" << header.nameLength << "\n";
//M2MeshName.clear();
//M2MeshName.reserve(header.nameLength);
file->seek(header.nameOfs);
//    file->read(&M2MeshName[0],header.nameLength);
//std::cout << "Read name:"<<M2MeshName.c_str()<<"Size: "<< M2MeshName.size() <<"|"<<M2MeshName[0]<< "\n";
//logger->log("Mesh Name",M2MeshName.c_str(),ELL_INFORMATION);
//Now we load all kinds of data from the file

//Vertices.  Global data
if(!M2MVertices.empty())
    M2MVertices.clear();

ModelVertex tempM2MVert;
file->seek(header.ofsVertices);

for(u32 i =0;i<header.nVertices;i++)
{
    file->read(&tempM2MVert,sizeof(ModelVertex));
    M2MVertices.push_back(tempM2MVert);
}
std::cout << "Read "<<M2MVertices.size()<<"/"<<header.nVertices<<" Vertices\n";

//Views == Sets of vertices. Usage yet unknown. Global data
if(M2MViews.size()>0)
    M2MViews.clear();
ModelView tempM2MView;
file->seek(header.ofsViews);
for(u32 i =0;i<header.nViews;i++)
{
    file->read(&tempM2MView,sizeof(ModelView));
    M2MViews.push_back(tempM2MView);
}
std::cout << "Read "<<M2MViews.size()<<"/"<<header.nViews<<" Views\n";

logger->log("Using View 0 for all further operations",ELL_INFORMATION);
std::cout<<"This View has "<<M2MViews[0].nSub<<" Submeshes\n";

//Vertex indices of a specific view.Local to View 0
if(M2MIndices.size()>0)
    M2MIndices.clear();

u16 tempM2Index;
file->seek(M2MViews[0].ofsIndex);
for(u32 i =0;i<M2MViews[0].nIndex;i++)
{
    file->read(&tempM2Index,sizeof(u16));
    M2MIndices.push_back(tempM2Index);
}
std::cout << "Read "<<M2MIndices.size()<<"/"<<M2MViews[0].nIndex<<" Indices\n";


//Triangles. Data Points point to the Vertex Indices, not the vertices themself. 3 Points = 1 Triangle, Local to View 0
if(M2MTriangles.size()>0)
    M2MTriangles.clear();

u16 tempM2Triangle;
file->seek(M2MViews[0].ofsTris);
for(u32 i =0;i<M2MViews[0].nTris;i++)
{
    file->read(&tempM2Triangle,sizeof(u16));
    M2MTriangles.push_back(tempM2Triangle);
}
std::cout << "Read "<<M2MTriangles.size()<<"/"<<M2MViews[0].nTris<<" Triangle Indices\n";

//Submeshes, Local to View 0
if(M2MSubmeshes.size()>0)
    M2MSubmeshes.clear();

ModelViewSubmesh tempM2Submesh;
file->seek(M2MViews[0].ofsSub);
for(u32 i =0;i<M2MViews[0].nSub;i++)
{
    file->read(&tempM2Submesh,sizeof(ModelViewSubmesh));
    M2MSubmeshes.push_back(tempM2Submesh);
//    std::cout<< "Submesh " <<i<<" ID "<<tempM2Submesh.meshpartId<<" starts at V/T "<<tempM2Submesh.ofsVertex<<"/"<<tempM2Submesh.ofsTris<<" and has "<<tempM2Submesh.nVertex<<"/"<<tempM2Submesh.nTris<<" V/T\n";
}
std::cout << "Read "<<M2MSubmeshes.size()<<"/"<<M2MViews[0].nSub<<" Submeshes\n";

//Texture units. Local to view 0
TextureUnit tempM2TexUnit;
if(!M2MTextureUnit.empty())
{
    M2MTextureUnit.clear();
}
file->seek(M2MViews[0].ofsTex);
for(u32 i=0;i<M2MViews[0].nTex;i++)
{
    file->read(&tempM2TexUnit,sizeof(TextureUnit));
    M2MTextureUnit.push_back(tempM2TexUnit);
}
std::cout << "Read "<<M2MTextureUnit.size()<<" Texture Unit entries for View 0\n";




//Texture Lookup table. This is global data
u16 tempM2TexLookup;
if(!M2MTextureLookup.empty())
{
    M2MTextureLookup.clear();
}
file->seek(header.ofsTexLookup);
for(u32 i=0;i<header.nTexLookup;i++)
{
    file->read(&tempM2TexLookup,sizeof(u16));
    M2MTextureLookup.push_back(tempM2TexLookup);
}
std::cout << "Read "<<M2MTextureLookup.size()<<" Texture lookup entries\n";

//Texture Definitions table. This is global data
TextureDefinition tempM2TexDef;
if(!M2MTextureDef.empty())
{
    M2MTextureDef.clear();
}
file->seek(header.ofsTextures);
for(u32 i=0;i<header.nTextures;i++)
{
    file->read(&tempM2TexDef,sizeof(TextureDefinition));
    M2MTextureDef.push_back(tempM2TexDef);
}
std::cout << "Read "<<M2MTextureDef.size()<<" Texture Definition entries\n";

//Render Flags table. This is global data
RenderFlags tempM2RF;
if(!M2MRenderFlags.empty())
{
    M2MRenderFlags.clear();
}
file->seek(header.ofsTexFlags);
for(u32 i=0;i<header.nTexFlags;i++)
{
    file->read(&tempM2RF,sizeof(RenderFlags));
    M2MRenderFlags.push_back(tempM2RF);
}
std::cout << "Read "<<M2MRenderFlags.size()<<" Render Flags\n";



//std::cout << M2MTextureUnit[0].submeshIndex1 <<","<<M2MTextureUnit[0].submeshIndex1 <<","<<M2MTextureUnit[0].textureIndex<<";\n";

if(!M2MTextureFiles.empty())
    M2MTextureFiles.clear();

std::string tempTexFileName="";
M2MTextureFiles.reallocate(M2MTextureDef.size());
for(u32 i=0; i<M2MTextureDef.size(); i++)
{
    tempTexFileName.reserve(M2MTextureDef[i].texFileLen + 1);
    file->seek(M2MTextureDef[i].texFileOfs);
    file->read(&tempTexFileName[0],M2MTextureDef[i].texFileLen);
    M2MTextureFiles.push_back(tempTexFileName.c_str());
    std::cout<<M2MTextureFiles.size()<<"-"<<M2MTextureFiles[i].c_str()<<"\n";
}
//    std::cout << "Read "<<M2MTextureFiles.size()<<"/"<<M2MTextureDef.size()<<" Texture file names\n";



//And M2MVertices are not usable like this. Thus we transform

if(M2Vertices.size()>0)
    M2Vertices.clear();

for(u32 i=0;i<M2MVertices.size();i++)
{
    M2Vertices.push_back(video::S3DVertex(M2MVertices[i].pos,M2MVertices[i].normal, video::SColor(255,100,100,100),M2MVertices[i].texcoords));
}



	if (Mesh)
		Mesh->drop(); // crash on vc9

Mesh=new SMesh();


while(Mesh->getMeshBufferCount()>0)
{
    Mesh->MeshBuffers.erase(0);
}


for(u32 i=0; i < M2MViews[0].nSub;i++)//
{
//std::cout << "Proceeding with Submesh "<<i<<"/"<<M2MViews[0].nSub<<"\n";
//Now, M2MTriangles refers to M2MIndices and not to M2MVertices.

if(M2Indices.size()>0)
    M2Indices.clear();

for(u32 j=M2MSubmeshes[i].ofsTris;j<M2MSubmeshes[i].ofsTris+M2MSubmeshes[i].nTris;j++)
{
    M2Indices.push_back(M2MIndices[M2MTriangles[j]]);
}

//std::cout << "Sending "<<M2Vertices.size() <<" Vertices and " << M2Indices.size() <<" Indices to the MeshBuffer\n";

IMB = new SMeshBuffer;
IMB->append(M2Vertices.const_pointer(),M2Vertices.size(),M2Indices.const_pointer(),M2Indices.size());
IMB->recalculateBoundingBox();

//IMB->getMaterial().DiffuseColor.set(255,255-(u32)(255/(M2MSubmeshes.size()))*i,(u32)(255/(M2MSubmeshes.size()))*i,0);
//IMB->getMaterial().DiffuseColor.set(255,(M2MSubmeshes[i].meshpartId==0?0:255),(M2MSubmeshes[i].meshpartId==0?255:0),0);


std::string TexName=Texdir.c_str();
TexName+="/";
if(i<M2MTextureUnit.size())
    TexName+=M2MTextureFiles[M2MTextureUnit[i].textureIndex].c_str();

while(TexName.find('\\')<TexName.size())//Replace \ by /
    {
    TexName.replace(TexName.find('\\'),1,"/");
    }
while(TexName.find(' ')<TexName.size())//Replace space by _
    {
    TexName.replace(TexName.find(' '),1,"_");
    }
std::transform(TexName.begin(), TexName.end(), TexName.begin(), tolower);

IMB->getMaterial().setTexture(0,Device->getVideoDriver()->getTexture(TexName.c_str()));
if(i<M2MRenderFlags.size())
{
    std::cout<<M2MRenderFlags[i].flags<<"--"<<M2MRenderFlags[i].blending<<"\n";
    IMB->getMaterial().BackfaceCulling=(M2MRenderFlags[i].flags & 0x04)?false:true;
    if(M2MRenderFlags[i].blending==1)
        IMB->getMaterial().MaterialType=video::EMT_TRANSPARENT_ALPHA_CHANNEL;
}
    Mesh->addMeshBuffer(IMB);
IMB->drop();
//std::cout << "Mesh now has "<<Mesh->getMeshBufferCount()<<" Buffers\n";
}

aniMesh= new SAnimatedMesh();
aniMesh->addMesh(Mesh);
Mesh->drop();
Mesh = 0;

aniMesh->recalculateBoundingBox();


return aniMesh;
}

}
}
