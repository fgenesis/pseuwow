/*-----------------------------------------------------------------------------*
| headerfile TLTMesh.h                                                         |
|                                                                              |
| version 1.10                                                                 |
| date: (07.04.2008)                                                           |
|                                                                              |
| author:  Michal Švantner                                                     |
|                                                                              |
| Some structures used for Tiled Terrain                                       |
| Writen for Irrlicht engine version 1.4                                       |
*-----------------------------------------------------------------------------*/

#ifndef TLTMESH_H
#define TLTMESH_H

#include "irrlicht/irrlicht.h"
using namespace irr;



// dynamic 2d array
template <class T> class array2d
{
   T** data;
   s32 w, h;

public:
   array2d() : w(0), h(0) {}

   array2d(s32 width, s32 height) : w(width), h(height)
   {
      data = new T*[w];
      for(int i=0; i<w; i++) data[i] = new T[h];
   }

   virtual ~array2d()
   {
      if(w && h)
      {
         for(int i=0; i<w; i++) delete [] data[i];
         delete [] data;
      }
   }

   virtual void reset(int width, int height)
   {
      if(w && h)
      {
         for(int i=0; i<w; i++) delete data[i];
         delete [] data;
      }

      if(width && height)
      {
         w = width;
         h = height;

         data = new T*[w];
         for(int i=0; i<w; i++) data[i] = new T[h];
      }
      else
      {
         w = 0;
         h = 0;
      }
   }

   virtual T& operator ()(s32 index1, s32 index2)
   {
      return data[index1][index2];
   }

   virtual s32 width() {return w;}

   virtual s32 height() {return h;}
};



// enumeration of tile vertices
enum TILE_VERTEX
{
    LOWER_LEFT = 0,
    UPPER_LEFT,
    UPPER_RIGHT,
    LOWER_RIGHT,
};



// structure holding pointers to 4 vertices of tile
// makes possible to acces vertex data based on tile structure
struct TlTTile
{
    video::S3DVertex2TCoords* Vertex[4];
};



// structure holding texture coordinates of tile
struct TlTCoords
{
    core::vector2d<f32> Vertex[4];
};



// structure which holds some data about spot on tiled terrain
struct TlTData
{
    f32 Height;
    core::vector3df Normal;
    video::SColor Color;
};



// structure which is used as meshbuffer for Tiled Terrain
struct TlTSector
{
    // position relative to whole mesh of which sector is part in tiles
    core::vector2d<s32> Offset;

    // dimension of sector in tiles
    core::dimension2d<s32> Size;

    // array of vertices
    core::array<video::S3DVertex2TCoords> Vertex;

    // array of indices
	core::array<u16> Index;

	// axis aligned bounding box
	core::aabbox3d<f32> BoundingBox;

	// update texture flag
	bool UpdateTexture;

	// update vertices flag
	bool UpdateVertices;

	// vissibility flag
	bool isVissible;
};
#endif
