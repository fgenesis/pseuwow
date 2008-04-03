#include <iostream>
#include "CM2MeshFileLoader.h"

namespace irr
{
    namespace scene
    {

        CM2MeshFileLoader::CM2MeshFileLoader(IrrlichtDevice* device):Device(device)
        {

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
            file->seek(header.nameOfs);
            file->read(&M2MeshName[0],header.nameLength);
            std::cout << "Read name:"<<M2MeshName.c_str()<<"\n";
            //logger->log("Mesh Name",M2MeshName.c_str(),ELL_INFORMATION);
            //Now we load all kinds of data from the file

            //Vertices
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

            //Views == Sets of vertices. Usage yet unknown
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

            //Vertex indices of a specific view.
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


            //Triangles. Data Points point to the Vertex Indices, not the vertices themself. 3 Points = 1 Triangle
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

            //Texture Lookup table.
            file->seek(header.ofsTexLookup);
            for(u32 i=0;i<header.nTexLookup;i++)
            {

            }

            //Now, M2MTriangles refers to M2MIndices and not to M2MVertices.
            //And M2MVertices are not usable like this. Thus we transform
            if(M2Vertices.size()>0)
                M2Vertices.clear();

            for(u32 i=0;i<M2MVertices.size();i++)
            {
                M2Vertices.push_back(video::S3DVertex(M2MVertices[i].pos,M2MVertices[i].normal, video::SColor(255,100,100,100),M2MVertices[i].texcoords));
            }
            if(M2Indices.size()>0)
                M2Indices.clear();

            for(u32 i=0;i<M2MTriangles.size();i++)
            {
                M2Indices.push_back(M2MIndices[M2MTriangles[i]]);
            }

            Mesh=new SMesh();


            SMeshBuffer* IMB = new SMeshBuffer;
            while(Mesh->getMeshBufferCount()>0)
            {
                Mesh->MeshBuffers.erase(0);
            }

            std::cout << "Sending "<<M2Vertices.size() <<" Vertices and " << M2Indices.size() <<" Indices to the MeshBuffer\n";
            IMB->append(M2Vertices.const_pointer(),M2Vertices.size(),M2Indices.const_pointer(),M2Indices.size());

            IMB->recalculateBoundingBox();
            Mesh->addMeshBuffer(IMB);
            IMB->drop();
            aniMesh= new SAnimatedMesh();


            aniMesh->addMesh(Mesh);
            Mesh->drop();
            Mesh = 0;
            
            aniMesh->recalculateBoundingBox();

            return aniMesh;
        }

    }
}
