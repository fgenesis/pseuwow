#ifndef TLMESH_H
#define TLMESH_H

#include <irrlicht.h>
using namespace irr;

template <class T> class array2d
{
    T** data;
    u32 w, h;
    
public:
    array2d() : w(0), h(0) {}
    
    array2d(int width, int height) : w(width), h(height)
    {
        data = new T*[w];
        for(int i=0; i<w; i++) data[i] = new T[h];
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
    
    ~array2d()
    {
        if(w && h)
        {
            for(int i=0; i<w; i++) delete data[i];
            delete [] data;
        }
    }
    
    virtual T& operator ()(u32 index1, u32 index2)
    {
        return data[index1][index2];
    }
    
    virtual u32 width() {return w;}
    
    virtual u32 height() {return h;}
};

enum TILE_VERTEX
{
    LOWER_LEFT = 0,
    UPPER_LEFT,
    UPPER_RIGHT,
    LOWER_RIGHT,
};

struct TlTTile
{
    video::S3DVertex2TCoords* Vertex[4];
};

struct TlTCoords
{
    core::vector2d<f32> Vertex[4];
};

struct TlTData
{
    f32 Height;
    core::vector3df Normal;
    video::SColor Color;
};

struct TlTSector
{
    core::vector2d<s32> Offset;
    core::dimension2d<u32> Size;
    core::array<video::S3DVertex2TCoords> Vertex;
	core::array<u16> Index;
	core::aabbox3d<f32> BoundingBox;
	
	array2d<TlTTile> Tile;
	
	virtual void addTile()
    {
        u32 n = Vertex.size();
        
        video::S3DVertex2TCoords v;
    
        Vertex.push_back(v);
        Vertex.push_back(v);
        Vertex.push_back(v);
        Vertex.push_back(v);
    
        Index.push_back(n);
        Index.push_back(n+1);
        Index.push_back(n+2);
    
        Index.push_back(n);
        Index.push_back(n+2);
        Index.push_back(n+3);
    }
    
    virtual void recalculateBoundingBox()
    {
        if(Vertex.empty())
            BoundingBox.reset(0,0,0);
        else
        {
            BoundingBox.reset(Vertex[0].Pos);
            for(u32 i=1; i<Vertex.size(); ++i)
				BoundingBox.addInternalPoint(Vertex[i].Pos);
        }
    }
};
#endif
