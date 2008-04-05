#include <vector>
#include "irrlicht/IImageLoader.h"
typedef unsigned long long int u64;

namespace irr
{
namespace video
{

//!  Surface Loader for BLP files
class CImageLoaderBLP : public IImageLoader
{
public:

   //! returns true if the file maybe is able to be loaded by this class
   //! based on the file extension (e.g. ".blp")
   virtual bool isALoadableFileExtension(const c8* fileName) const;

   //! returns true if the file maybe is able to be loaded by this class
   virtual bool isALoadableFileFormat(io::IReadFile* file) const;

   //! creates a surface from the file
   virtual IImage* loadImage(io::IReadFile* file) const;
private:
    struct BLPHeader
    {
        c8 fileID[4];
        u32 version;
        u8 compression;
        u8 alpha_bitdepth;
        u8 alpha_unk;
        u8 miplevel;
        u32 x_res;
        u32 y_res;
        u32 mip_ofs[16];
        u32 mip_size[16];
    };
    struct DXC1chunk
    {
        u16 color1;
        u16 color2;
        u32 bitmap;
    };
    struct DXC3chunk//This is kind of useless
    {
        u64 transparency_block;
    };
    struct DXC5chunk
    {
        u8 alpha1, alpha2;
        u16 bitmap[3];//how do i express an "u48"?
    };

    struct PaletteColor//Not sure if an Irrlicht color can handle the changed color sequence
    {
        u8 B,G,R,A;
    };

};


} // end namespace video
} // end namespace irr
