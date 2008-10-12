#include <iostream>
#include "CM2MeshFileLoader.h"
#include "SSkinnedMesh.h"
#define _DEBUG
#include "common.h"
#ifdef _DEBUG
#define DEBUG(code) code;
#else
#define DEBUG(code) ;
#endif

/*

void logdebug(const char *str, ...)
{
    if(!str)
        return;
    va_list ap;
//    _log_setcolor(true,LBLUE);
    va_start(ap, str);
    vprintf( str, ap );
    va_end(ap);
//    _log_resetcolor(true);


    printf("\n");

  fflush(stdout);
}*/

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
    if(!file)
        return 0;
    MeshFile = file;
    AnimatedMesh = new scene::CSkinnedMesh();

	if ( load() )
	{
		AnimatedMesh->finalize();
	}
	else
	{
		AnimatedMesh->drop();
		AnimatedMesh = 0;
	}

	return AnimatedMesh;
}
bool CM2MeshFileLoader::load()
{
DEBUG(logdebug("Trying to open file %s",MeshFile->getFileName()));


MeshFile->read(&header,sizeof(ModelHeader));
if (header.version[0] != 4 && header.version[1] != 1 && header.version[2] != 0 && header.version[3] != 0) {
     printf("Wrong header! File version doesn't match or file is not a M2 file.");
     return 0;
     }
     else
     {
         DEBUG(logdebug("header okay"));
     }
//Name -> not very important I think, but save it nontheless;
//M2MeshName.clear();
//M2MeshName.reserve(header.nameLength);
//file->seek(header.nameOfs);
//file->read(&M2MeshName[0],header.nameLength);
//std::cout << "Read name:"<<M2MeshName.c_str()<<"Size: "<< M2MeshName.size() <<"|"<<M2MeshName[0]<< "\n";
//logger->log("Mesh Name",M2MeshName.c_str(),ELL_INFORMATION);
//Now we load all kinds of data from the file

//Vertices.  Global data
if(!M2MVertices.empty())
    M2MVertices.clear();

ModelVertex tempM2MVert;
f32 tempYZ;
MeshFile->seek(header.ofsVertices);

for(u32 i =0;i<header.nVertices;i++)
{
    MeshFile->read(&tempM2MVert,sizeof(ModelVertex));
    tempYZ = tempM2MVert.pos.Y;
    tempM2MVert.pos.Y=tempM2MVert.pos.Z;
    tempM2MVert.pos.Z=tempYZ;
    tempYZ = tempM2MVert.normal.Y;
    tempM2MVert.normal.Y=tempM2MVert.normal.Z;
    tempM2MVert.normal.Z=tempYZ;

    M2MVertices.push_back(tempM2MVert);
}
DEBUG(logdebug("Read %u/%u Vertices",M2MVertices.size(),header.nVertices));

//Views == Sets of vertices. Usage yet unknown. Global data
if(M2MViews.size()>0)
    M2MViews.clear();
ModelView tempM2MView;
MeshFile->seek(header.ofsViews);
for(u32 i =0;i<header.nViews;i++)
{
    MeshFile->read(&tempM2MView,sizeof(ModelView));
    M2MViews.push_back(tempM2MView);
}
//std::cout << "Read "<<M2MViews.size()<<"/"<<header.nViews<<" Views\n";

DEBUG(logdebug("Using View 0 for all further operations"));
DEBUG(logdebug("This View has %u Submeshes",M2MViews[0].nSub));

//Vertex indices of a specific view.Local to View 0
if(M2MIndices.size()>0)
    M2MIndices.clear();

u16 tempM2Index;
MeshFile->seek(M2MViews[0].ofsIndex);
for(u32 i =0;i<M2MViews[0].nIndex;i++)
{
    MeshFile->read(&tempM2Index,sizeof(u16));
    M2MIndices.push_back(tempM2Index);
}
DEBUG(logdebug("Read %u/%u Indices",M2MIndices.size(),M2MViews[0].nIndex));


//Triangles. Data Points point to the Vertex Indices, not the vertices themself. 3 Points = 1 Triangle, Local to View 0
if(M2MTriangles.size()>0)
    M2MTriangles.clear();

u16 tempM2Triangle;
MeshFile->seek(M2MViews[0].ofsTris);
for(u32 i =0;i<M2MViews[0].nTris;i++)
{
    MeshFile->read(&tempM2Triangle,sizeof(u16));
    M2MTriangles.push_back(tempM2Triangle);
}
DEBUG(logdebug("Read %u/%u Triangles",M2MTriangles.size(),M2MViews[0].nTris));
//Submeshes, Local to View 0
if(M2MSubmeshes.size()>0)
    M2MSubmeshes.clear();

ModelViewSubmesh tempM2Submesh;
MeshFile->seek(M2MViews[0].ofsSub);
for(u32 i =0;i<M2MViews[0].nSub;i++)
{
    MeshFile->read(&tempM2Submesh,sizeof(ModelViewSubmesh));
    M2MSubmeshes.push_back(tempM2Submesh);
//    std::cout<< "Submesh " <<i<<" ID "<<tempM2Submesh.meshpartId<<" starts at V/T "<<tempM2Submesh.ofsVertex<<"/"<<tempM2Submesh.ofsTris<<" and has "<<tempM2Submesh.nVertex<<"/"<<tempM2Submesh.nTris<<" V/T\n";
}
DEBUG(logdebug("Read %u/%u Submeshes",M2MSubmeshes.size(),M2MViews[0].nSub));

//Texture units. Local to view 0
TextureUnit tempM2TexUnit;
if(!M2MTextureUnit.empty())
{
    M2MTextureUnit.clear();
}
MeshFile->seek(M2MViews[0].ofsTex);
for(u32 i=0;i<M2MViews[0].nTex;i++)
{
    MeshFile->read(&tempM2TexUnit,sizeof(TextureUnit));
    M2MTextureUnit.push_back(tempM2TexUnit);
DEBUG(logdebug(" TexUnit %u: Submesh: %u %u Render Flag: %u TextureUnitNumber: %u %u TTU: %u",i,tempM2TexUnit.submeshIndex1,tempM2TexUnit.submeshIndex2, tempM2TexUnit.renderFlagsIndex, tempM2TexUnit.TextureUnitNumber, tempM2TexUnit.TextureUnitNumber2 ,tempM2TexUnit.textureIndex));
}
DEBUG(logdebug("Read %u Texture Unit entries for View 0",M2MTextureUnit.size()));




//Texture Lookup table. This is global data
u16 tempM2TexLookup;
if(!M2MTextureLookup.empty())
{
    M2MTextureLookup.clear();
}
MeshFile->seek(header.ofsTexLookup);
for(u32 i=0;i<header.nTexLookup;i++)
{
    MeshFile->read(&tempM2TexLookup,sizeof(u16));
    M2MTextureLookup.push_back(tempM2TexLookup);
    printf("Texture %u Type %u\n",i,tempM2TexLookup);
}
DEBUG(logdebug("Read %u Texture lookup entries",M2MTextureLookup.size()));

//Texture Definitions table. This is global data
TextureDefinition tempM2TexDef;
if(!M2MTextureDef.empty())
{
    M2MTextureDef.clear();
}
MeshFile->seek(header.ofsTextures);
for(u32 i=0;i<header.nTextures;i++)
{
    MeshFile->read(&tempM2TexDef,sizeof(TextureDefinition));
    M2MTextureDef.push_back(tempM2TexDef);
    printf("Texture %u Type %u\n",i,tempM2TexDef.texType);
}
DEBUG(logdebug("Read %u Texture Definition entries",M2MTextureDef.size()));

//Render Flags table. This is global data
RenderFlags tempM2RF;
if(!M2MRenderFlags.empty())
{
    M2MRenderFlags.clear();
}
MeshFile->seek(header.ofsTexFlags);
for(u32 i=0;i<header.nTexFlags;i++)
{
    MeshFile->read(&tempM2RF,sizeof(RenderFlags));
    M2MRenderFlags.push_back(tempM2RF);
    DEBUG(logdebug("Flag %u: (%u, %u)",i,tempM2RF.blending,tempM2RF.flags));
}
DEBUG(logdebug("Read %u Renderflags",M2MRenderFlags.size()));



//std::cout << M2MTextureUnit[0].submeshIndex1 <<","<<M2MTextureUnit[0].submeshIndex1 <<","<<M2MTextureUnit[0].textureIndex<<";\n";

if(!M2MTextureFiles.empty())
    M2MTextureFiles.clear();

std::string tempTexFileName="";
M2MTextureFiles.reallocate(M2MTextureDef.size());
for(u32 i=0; i<M2MTextureDef.size(); i++)
{
    tempTexFileName.reserve(M2MTextureDef[i].texFileLen + 1);
    MeshFile->seek(M2MTextureDef[i].texFileOfs);
    MeshFile->read((void*)tempTexFileName.c_str(),M2MTextureDef[i].texFileLen);
    M2MTextureFiles.push_back(tempTexFileName.c_str());
    DEBUG(logdebug("Texture: %u (%s)",M2MTextureFiles.size(),M2MTextureFiles[i].c_str()));
}
//    std::cout << "Read "<<M2MTextureFiles.size()<<"/"<<M2MTextureDef.size()<<" Texture file names\n";

///////////////////////////////////////
//      Animation related stuff      //
///////////////////////////////////////

printf("Global Sequences: %u\n",header.nGlobalSequences);
//Ignored at the moment, as wolf.m2 has none
printf("Animations: %u\n",header.nAnimations);
//Animations. This is global data
Animation tempAnimation;
if(!M2MAnimations.empty())
{
    M2MAnimations.clear();
}
MeshFile->seek(header.ofsAnimations);
for(u32 i=0;i<header.nAnimations;i++)
{
    MeshFile->read(&tempAnimation,sizeof(Animation));
    M2MAnimations.push_back(tempAnimation);
    //std::cout<<tempAnimation.start<<" "<<tempAnimation.end<<"\n";
}
DEBUG(logdebug("Read %u Animations",M2MAnimations.size()));
printf("Read %u Animations\n",M2MAnimations.size());

printf("Bones: %u\n",header.nBones);
//Bones. This is global data
Bone tempBone;
if(!M2MBones.empty())
{
    M2MBones.clear();
}
MeshFile->seek(header.ofsBones);
for(u32 i=0;i<header.nBones;i++)
{
    MeshFile->read(&tempBone,16);
    MeshFile->read(&tempBone.translation.header,sizeof(AnimBlockHead));
    MeshFile->read(&tempBone.rotation.header,sizeof(AnimBlockHead));
    MeshFile->read(&tempBone.scaling.header,sizeof(AnimBlockHead));
    MeshFile->read(&tempBone.PivotPoint,sizeof(core::vector3df));
    tempYZ=tempBone.PivotPoint.Y;
    tempBone.PivotPoint.Y=tempBone.PivotPoint.Z;
    tempBone.PivotPoint.Z=tempYZ;
    M2MBones.push_back(tempBone);
    //std::cout<<i<<":"<<tempBone.flags<<" "<<tempBone.parentBone<<"  |"<<tempBone.translation.header.interpolationType<<" "<<tempBone.translation.header.nInterpolationRange<<" "<<tempBone.translation.header.nTimeStamp<<" "<<tempBone.translation.header.nValues<<"|"<<tempBone.rotation.header.interpolationType<<" "<<tempBone.rotation.header.nInterpolationRange<<" "<<tempBone.rotation.header.nTimeStamp<<" "<<tempBone.rotation.header.nValues<<"|"<<tempBone.scaling.header.interpolationType<<" "<<tempBone.scaling.header.nInterpolationRange<<" "<<tempBone.scaling.header.nTimeStamp<<" "<<tempBone.scaling.header.nValues<<"\n";
}
//Fill in values referenced in Bones. local to each bone
InterpolationRange tempBoneIR;
u32 tempBoneTS;
float tempBoneValue;
for(u32 i=0; i<M2MBones.size(); i++)
{
    if(M2MBones[i].translation.header.nInterpolationRange>0)
    {
        MeshFile->seek(M2MBones[i].translation.header.ofsInterpolationRange);
        for(u32 j=0; j<M2MBones[i].translation.header.nInterpolationRange;j++)
        {
            MeshFile->read(&tempBoneIR, sizeof(InterpolationRange));
            M2MBones[i].translation.keyframes.push_back(tempBoneIR);
        }
    }
    if(M2MBones[i].rotation.header.nInterpolationRange>0)
    {
        MeshFile->seek(M2MBones[i].rotation.header.ofsInterpolationRange);
        for(u32 j=0; j<M2MBones[i].rotation.header.nInterpolationRange;j++)
        {
            MeshFile->read(&tempBoneIR, sizeof(InterpolationRange));
            M2MBones[i].rotation.keyframes.push_back(tempBoneIR);
        }
    }
    if(M2MBones[i].scaling.header.nInterpolationRange>0)
    {
        MeshFile->seek(M2MBones[i].scaling.header.ofsInterpolationRange);
        for(u32 j=0; j<M2MBones[i].scaling.header.nInterpolationRange;j++)
        {
            MeshFile->read(&tempBoneIR, sizeof(InterpolationRange));
            M2MBones[i].scaling.keyframes.push_back(tempBoneIR);
        }
    }

    if(M2MBones[i].translation.header.nTimeStamp>0)
    {
        MeshFile->seek(M2MBones[i].translation.header.ofsTimeStamp);
        for(u32 j=0; j<M2MBones[i].translation.header.nTimeStamp;j++)
        {
            MeshFile->read(&tempBoneTS, sizeof(u32));
            M2MBones[i].translation.timestamps.push_back(tempBoneTS);
        }
    }
    if(M2MBones[i].rotation.header.nTimeStamp>0)
    {
        MeshFile->seek(M2MBones[i].rotation.header.ofsTimeStamp);
        for(u32 j=0; j<M2MBones[i].rotation.header.nTimeStamp;j++)
        {
            MeshFile->read(&tempBoneTS, sizeof(u32));
            M2MBones[i].rotation.timestamps.push_back(tempBoneTS);
        }
    }
    if(M2MBones[i].scaling.header.nTimeStamp>0)
    {
        MeshFile->seek(M2MBones[i].scaling.header.ofsTimeStamp);
        for(u32 j=0; j<M2MBones[i].scaling.header.nTimeStamp;j++)
        {
            MeshFile->read(&tempBoneTS, sizeof(u32));
            M2MBones[i].scaling.timestamps.push_back(tempBoneTS);
        }
    }
    if(M2MBones[i].translation.header.nValues>0)
    {
        MeshFile->seek(M2MBones[i].translation.header.ofsValues);
        for(u32 j=0; j<M2MBones[i].translation.header.nValues*3;j++)
        {
            MeshFile->read(&tempBoneValue, sizeof(float));
            M2MBones[i].translation.values.push_back(tempBoneValue);
        }
    }
    if(M2MBones[i].rotation.header.nValues>0)
    {
        MeshFile->seek(M2MBones[i].rotation.header.ofsValues);
        for(u32 j=0; j<M2MBones[i].rotation.header.nValues*4;j++)
        {
            s16 tempBoneShort;
            MeshFile->read(&tempBoneShort, sizeof(s16));
            tempBoneValue=(tempBoneShort>0?tempBoneShort-32767:tempBoneShort+32767)/32767.0f;
            M2MBones[i].rotation.values.push_back(tempBoneValue);
        }
    }
    if(M2MBones[i].scaling.header.nValues>0)
    {
        MeshFile->seek(M2MBones[i].scaling.header.ofsValues);
        for(u32 j=0; j<M2MBones[i].scaling.header.nValues*3;j++)
        {
            MeshFile->read(&tempBoneValue, sizeof(float));
            M2MBones[i].scaling.values.push_back(tempBoneValue);
        }
    }
}

DEBUG(logdebug("Read %u Bones",M2MBones.size()));


scene::CSkinnedMesh::SJoint* Joint;
for(u32 i=0;i<M2MBones.size();i++)
{
    if(M2MBones[i].parentBone == -1)
    {
        ParentJoint=(scene::CSkinnedMesh::SJoint*)0;
    }
    else
    {
        ParentJoint=AnimatedMesh->getAllJoints()[M2MBones[i].parentBone];
    }
    Joint=AnimatedMesh->createJoint(ParentJoint);


    //std::cout << i << " "<<Joint->GlobalMatrix.getTranslation().X<< " "<<Joint->GlobalMatrix.getTranslation().Y<< " "<<Joint->GlobalMatrix.getTranslation().Z<<'\n';
    //std::cout << i << " "<<M2MBones[i].PivotPoint.X<< " "<<M2MBones[i].PivotPoint.Y<< " "<<M2MBones[i].PivotPoint.Z<<'\n';

 /*  for(u32 j=0;j<M2MBones[i].translation.header.nValues;j++)
    {
    scene::CSkinnedMesh::SPositionKey* pos=AnimatedMesh->createPositionKey(Joint);
    pos->frame=M2MBones[i].translation.timestamps[j]*.01f;
    pos->position=core::vector3df(M2MBones[i].translation.values[j*3],M2MBones[i].translation.values[j*3+1],M2MBones[i].translation.values[j*3+2]);
    }
if(M2MBones[i].rotation.header.nValues>0)
{
    for(u32 j=0;j<M2MBones[i].rotation.header.nValues;j++)
    {
    scene::CSkinnedMesh::SRotationKey* rot=AnimatedMesh->createRotationKey(Joint);
    rot->frame=M2MBones[i].rotation.timestamps[j]*.01f;
    core::quaternion tempQ=core::quaternion(M2MBones[i].rotation.values[j*4+0],M2MBones[i].rotation.values[j*4+1],M2MBones[i].rotation.values[j*4+2],M2MBones[i].rotation.values[j*4+3]);
    rot->rotation=tempQ;
//    std::cout <<" "<< M2MBones[i].rotation.values[j*4+0] <<" "<< M2MBones[i].rotation.values[j*4+1] <<" "<< M2MBones[i].rotation.values[j*4+2] <<" "<< M2MBones[i].rotation.values[j*4+3] <<'\n';
    }
}
if(M2MBones[i].scaling.header.nValues>0){
    for(u32 j=0;j<M2MBones[i].scaling.header.nValues;j++)
    {
    scene::CSkinnedMesh::SScaleKey* scale=AnimatedMesh->createScaleKey(Joint);
    scale->frame=M2MBones[i].scaling.timestamps[j]*.01f;
    scale->scale=core::vector3df(M2MBones[i].scaling.values[j*3],M2MBones[i].scaling.values[j*3+1],M2MBones[i].scaling.values[j*3+2]);
    }
*/
   // Joint->Animatedposition=M2MBones[i].PivotPoint;
//	        std::cout<<Joint->Animatedposition.X<<' '<<Joint->Animatedposition.Y<<' '<<Joint->Animatedposition.Z<<' '<<'\n';

//    Joint->Animatedscale=core::vector3df(1.0f,1.0f,1.0f);
    //Joint->Animatedrotation=core::quaternion(0.0f,0.0f,0.0f,0.0f);
    core::matrix4 positionMatrix;
//	positionMatrix.setTranslation( Joint->Animatedposition );
	core::matrix4 scaleMatrix;
	//scaleMatrix.setScale( Joint->Animatedscale );
	core::matrix4 rotationMatrix;// = Joint->Animatedrotation.getMatrix();

	Joint->GlobalMatrix = positionMatrix * rotationMatrix * scaleMatrix;//

	if (ParentJoint)
    {
        core::matrix4 InverseParentGlobal;
        ParentJoint->GlobalMatrix.getInverse(InverseParentGlobal);
		Joint->LocalMatrix = InverseParentGlobal * Joint->GlobalMatrix;
    }
	else
		Joint->LocalMatrix = Joint->GlobalMatrix;
}

//std::cout<<AnimatedMesh->getAllJoints()[1]->Children.size()<<" Children\n";
//And M2MVertices are not usable like this. Thus we transform

if(M2Vertices.size()>0)
    M2Vertices.clear();

for(u32 i=0;i<M2MVertices.size();i++)
{
    //M2Vertices.push_back(video::S3DVertex(core::vector3df(M2MVertices[i].pos.X,M2MVertices[i].pos.Z,M2MVertices[i].pos.Y),core::vector3df(M2MVertices[i].normal.X,M2MVertices[i].normal.Z,M2MVertices[i].normal.Y), video::SColor(255,100,100,100),M2MVertices[i].texcoords));
    //rotation happens when reading from file, so swapping Y and Z here is no longer necessary
    M2Vertices.push_back(video::S3DVertex(core::vector3df(M2MVertices[i].pos.X,M2MVertices[i].pos.Y,M2MVertices[i].pos.Z),core::vector3df(M2MVertices[i].normal.X,M2MVertices[i].normal.Y,M2MVertices[i].normal.Z), video::SColor(255,100,100,100),M2MVertices[i].texcoords));
}
//Loop through the submeshes
for(u32 i=0; i < M2MViews[0].nSub;i++)//
{
    //Now, M2MTriangles refers to M2MIndices and not to M2MVertices.
    scene::SSkinMeshBuffer *MeshBuffer = AnimatedMesh->createBuffer();

    //Put the Indices and Vertices of the Submesh into a mesh buffer
    //Each Submesh contains only the Indices and Vertices that belong to it.
    //Because of this the Index values for the Submeshes must be corrected by the Vertex offset of the Submesh
    for(u32 j=M2MSubmeshes[i].ofsTris;j<M2MSubmeshes[i].ofsTris+M2MSubmeshes[i].nTris;j++)
    {
        MeshBuffer->Indices.push_back(M2MIndices[M2MTriangles[j]]-M2MSubmeshes[i].ofsVertex);
    }
  //  std::cout << i << ": " << MeshBuffer->Indices.size() << "\n";
    for(u32 j=M2MSubmeshes[i].ofsVertex;j<M2MSubmeshes[i].ofsVertex+M2MSubmeshes[i].nVertex;j++)
    {
        MeshBuffer->Vertices_Standard.push_back(M2Vertices[j]);
        for(u32 k=0; k<4; k++)
        {
            //std::cout << (u32)M2MVertices[j].bones[k] << " ";
            if((M2MVertices[j].weights[k]/255.0f)>0.0f)
            {
            scene::CSkinnedMesh::SWeight* weight = AnimatedMesh->createWeight(AnimatedMesh->getAllJoints()[(u32)M2MVertices[j].bones[k]]);
            weight->strength=M2MVertices[j].weights[k]/255.0f;
            weight->vertex_id=j-M2MSubmeshes[i].ofsVertex;
            weight->buffer_id=i;
            }
            //std::cout<<weight->buffer_id << " " << weight->vertex_id << " " << weight->strength <<"|";
        }
    //    std::cout<<'\n';
    }
    //std::cout << i << ": " << MeshBuffer->Vertices_Standard.size() <<" "<<M2MSubmeshes[i].ofsVertex<<" "<<M2MSubmeshes[i].nVertex<< "\n";


    MeshBuffer->recalculateBoundingBox();
    //MeshBuffer->getMaterial().DiffuseColor.set(255,255-(u32)(255/(M2MSubmeshes.size()))*i,(u32)(255/(M2MSubmeshes.size()))*i,0);
    //MeshBuffer->getMaterial().DiffuseColor.set(255,(M2MSubmeshes[i].meshpartId==0?0:255),(M2MSubmeshes[i].meshpartId==0?255:0),0);
    for(u32 j=0;j<M2MTextureUnit.size();j++)//Loop through texture units
        {
        if(M2MTextureUnit[j].submeshIndex1==i)//if a texture unit belongs to this submesh
            {
            std::string TexName=Texdir.c_str();
            TexName+="/";
            if(i<M2MTextureUnit.size())
				TexName+=M2MTextureFiles[M2MTextureLookup[M2MTextureUnit[j].textureIndex]].c_str();
            while(TexName.find('\\')<TexName.size())//Replace \ by /
                {
                TexName.replace(TexName.find('\\'),1,"/");
                }
            while(TexName.find(' ')<TexName.size())//Replace space by _
                {
                TexName.replace(TexName.find(' '),1,"_");
                }
            std::transform(TexName.begin(), TexName.end(), TexName.begin(), tolower);
            MeshBuffer->getMaterial().setTexture(M2MTextureUnit[j].TextureUnitNumber,Device->getVideoDriver()->getTexture(TexName.c_str()));

            DEBUG(logdebug("Render Flags: %u %u",M2MRenderFlags[M2MTextureUnit[j].renderFlagsIndex].flags,M2MRenderFlags[M2MTextureUnit[j].renderFlagsIndex].blending));
            MeshBuffer->getMaterial().BackfaceCulling=(M2MRenderFlags[M2MTextureUnit[j].renderFlagsIndex].flags & 0x04)?false:true;
            if(M2MRenderFlags[M2MTextureUnit[j].renderFlagsIndex].blending==1)
            MeshBuffer->getMaterial().MaterialType=video::EMT_TRANSPARENT_ALPHA_CHANNEL;
            }

        }


    //MeshBuffer->recalculateBoundingBox();
    //    Mesh->addMeshBuffer(MeshBuffer);
    //    Mesh->recalculateBoundingBox();
    //MeshBuffer->drop();
    //std::cout << "Mesh now has "<<Mesh->getMeshBufferCount()<<" Buffers\n";
}




Device->getSceneManager()->getMeshManipulator()->flipSurfaces(AnimatedMesh); //Fix inverted surfaces after the rotation
Device->getSceneManager()->getMeshManipulator()->recalculateNormals(AnimatedMesh,true);//just to be sure

AnimatedMesh->setInterpolationMode(scene::EIM_LINEAR);

M2MTriangles.clear();
M2Vertices.clear();
M2Indices.clear();
M2MIndices.clear();
M2MVertices.clear();
M2MRenderFlags.clear();
M2MTextureUnit.clear();
M2MTextureDef.clear();
M2MSubmeshes.clear();
M2MTextureFiles.clear();
M2MTextureLookup.clear();
M2MViews.clear();
return true;
}

}
}
