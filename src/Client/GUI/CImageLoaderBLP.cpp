#include <iostream>
#include <string>
#include "common.h"
#include "irrlicht/irrlicht.h"
#include "SImage.h"
#include "CImageLoaderBLP.h"

#if defined(_DEBUG) && !defined(DEBUG)
#define DEBUG(code) code;
#else
#define DEBUG(code) ;
#endif

namespace irr
{
namespace video
{

//! returns true if the file maybe is able to be loaded by this class
//! based on the file extension (e.g. ".tga")
bool CImageLoaderBLP::isALoadableFileExtension(const c8* fileName) const
{
//  Checking for file extension
    return strstr(fileName, ".blp")!=0;
}


//! returns true if the file maybe is able to be loaded by this class
bool CImageLoaderBLP::isALoadableFileFormat(io::IReadFile* file) const
{
 //Checking if file is a BLP file
	if (!file)
	{
	    DEBUG(logdebug("No such file: %s",file->getFileName()));
		return false;
	}
    std::string fileId;
	// Read the first few bytes of the BLP file
	if (file->read(&fileId[0], 4) != 4)
    {
        DEBUG(logdebug("Cannot read BLP file header\n"));
		return false;
    }

	if(fileId[0]=='B' && fileId[1]=='L' && fileId[2]=='P' && fileId[3]=='2')
    {
        DEBUG(logdebug("Header is BLP2, file should be loadable"));
        return true;
    }
    else
    {
        DEBUG(logdebug("Header doesn't match, this is no BLP file"));
        DEBUG(logdebug("Expected:BLP2 Got:%s",fileId.c_str()));
        return false;
    }
}


// load in the image data
IImage* CImageLoaderBLP::loadImage(io::IReadFile* file) const
{
	if (!file)
		return 0;

    BLPHeader header;
//    std::cout<<"Trying to load the image\n";
//	std::cout<<"Checking Header\n";
	file->read(&header,sizeof(BLPHeader));

//	std::cout<<"Header data: "<<header.fileID<<"\n Alpha depth:"<<(u32)header.alpha_bitdepth<<"bit\nCompression:"<<(u32)header.compression<<"\n";
//	std::cout<<"Mystery factor:"<<(u32)header.alpha_unk<<"\n";
//	std::cout<<"X-Res: "<< header.x_res<<"\nY-Res:"<<header.y_res<<"\n";
    u32 usedMips=0;
    for(u32 i=0;i<16;i++)
        {
            if(header.mip_ofs[i]!=0&&header.mip_size[i]!=0)
                usedMips++;
        }
 //   std::cout<<"Mip Levels:"<< usedMips<<"\n";
    core::array<PaletteColor> palette;
    PaletteColor tempColor;
    palette.reallocate(256);
    for(u32 i=0;i<256;i++)
        {
            file->read(&tempColor,sizeof(PaletteColor));
            palette.push_back(tempColor);
        }


//    std::cout<<"Loading Mip 0 Length is "<<header.mip_size[0]<<"\n";
    file->seek(header.mip_ofs[0]);

    video::IImage* image = 0;
    image = new SImage(ECF_A8R8G8B8, core::dimension2d<s32>(header.x_res, header.y_res));

    if(header.compression==2)
    {
    //Reading imageData for DXT1/3/5 (5 not really...)
        DXC5chunk tempChunk5;
        DXC3chunk tempChunk3;
        DXC1chunk tempChunk1;
        core::array<DXC5chunk> imagedata5;
        core::array<DXC3chunk> imagedata3;
        core::array<DXC1chunk> imagedata1;

        u32 count = header.mip_size[0] / (header.alpha_bitdepth > 1 ? 16 : 8);
        for(u32 i=0;i<count;i++)
        {
            if(header.compression==2&&header.alpha_bitdepth>1)
            {
                if(header.alpha_unk==7)//it is not absolutely necessary to divide DXT3 and 5 data here as both are 64bit blocks which have to be dissected later.
                {                      // But this way it is somehow clearer for me
                    file->read(&tempChunk5,sizeof(DXC5chunk));
                    imagedata5.push_back(tempChunk5);
                }
                else
                {
                    file->read(&tempChunk3,sizeof(DXC3chunk));
                    imagedata3.push_back(tempChunk3);
                }
            }
            file->read(&tempChunk1,sizeof(DXC1chunk));
            imagedata1.push_back(tempChunk1);

        }
//        std::cout << "Data read\n";
        u32 i=0;
        u32 alpha=255;
        u32 a[8];
        u32  r1, g1,b1,r2,g2,b2;
        u64 temptransp=0;
        bool transparency_bit=false;
        for(u32 y=0;y<header.y_res;y=y+4)
        {
            for(u32 x=0;x<header.x_res;x=x+4)
            {
                    f32 rb=256/31;
                    f32 g=256/63;
                    r1 = (u32)rb*(imagedata1[i].color1 & 0xF800) >>11;
                    g1 = (u32)g*(imagedata1[i].color1 & 0x07E0) >>5;
                    b1 = (u32)rb*(imagedata1[i].color1 & 0x001F) ;
                    r2 = (u32)rb*(imagedata1[i].color2 & 0xF800) >>11;
                    g2 = (u32)g*(imagedata1[i].color2 & 0x07E0) >>5;
                    b2 = (u32)rb*(imagedata1[i].color2 & 0x001F) ;
                if(imagedata1[i].color1>imagedata1[i].color2||header.alpha_bitdepth==8)
                {
                    transparency_bit=false;
                }
                else
                {
    /*	            std::cout << imagedata1[i].color1 <<","<<imagedata1[i].color2<<"\n";
    //                f32 rgb=256/31;
                    r1 = (u32)rgb*(imagedata1[i].color1 & 0xF800) >>11;
                    g1 = (u32)rgb*(imagedata1[i].color1 & 0x07C0) >>6;
                    b1 = (u32)rgb*(imagedata1[i].color1 & 0x003E) >>1;
                    r2 = (u32)rgb*(imagedata1[i].color2 & 0xF800) >>11;
                    g2 = (u32)rgb*(imagedata1[i].color2 & 0x07C0) >>6;
                    b2 = (u32)rgb*(imagedata1[i].color2 & 0x003E) >>1;
      */              transparency_bit=true;
                }

                u32 tempbitmap=imagedata1[i].bitmap;
                if(header.alpha_bitdepth==8)
                {
                    if(header.alpha_unk==7)
                    {
                        temptransp=(u64)imagedata5[i].bitmap[2]<<32|(u64)imagedata5[i].bitmap[1]<<16|imagedata5[i].bitmap[0];
                        a[0]=imagedata5[i].alpha1;
                        a[1]=imagedata5[i].alpha2;
                        if (a[0] > a[1]) {
                            // 8-alpha block:  derive the other six alphas.
                            // Bit code 000 = alpha_0, 001 = alpha_1, others are interpolated.
                            a[2] = (6 * a[0] + 1 * a[1]) / 7;    // bit code 010
                            a[3] = (5 * a[0] + 2 * a[1]) / 7;    // bit code 011
                            a[4] = (4 * a[0] + 3 * a[1]) / 7;    // bit code 100
                            a[5] = (3 * a[0] + 4 * a[1]) / 7;    // bit code 101
                            a[6] = (2 * a[0] + 5 * a[1]) / 7;    // bit code 110
                            a[7] = (1 * a[0] + 6 * a[1]) / 7;    // bit code 111
                        }
                        else
                        {
                            // 6-alpha block.
                            // Bit code 000 = a[0], 001 = a[1], others are interpolated.
                            a[2] = (4 * a[0] + 1 * a[1]) / 5;    // Bit code 010
                            a[3] = (3 * a[0] + 2 * a[1]) / 5;    // Bit code 011
                            a[4] = (2 * a[0] + 3 * a[1]) / 5;    // Bit code 100
                            a[5] = (1 * a[0] + 4 * a[1]) / 5;    // Bit code 101
                            a[6] = 0;                                      // Bit code 110
                            a[7] = 255;                                    // Bit code 111
                        }
                    }
                    else
                    {
                        temptransp=imagedata3[i].transparency_block;
                    }
                }
                for(u32 ty=0;ty<4;ty++)
                {
                    for(u32 tx=0;tx<4;tx++)
                    {
                        if(header.alpha_bitdepth==8)
                            {
                                if(header.alpha_unk==7)
                                {
                                alpha=a[(temptransp & 7)];
                                }
                                else
                                {
                                f32 a=256/15;
                                alpha=(u32)a * (temptransp & 15);
                                }
                            }
                        else
                            {
                                alpha=255;
                            }
                        switch(tempbitmap & 3)
                        {
                            case 0:
                            {
                                image->setPixel(x+tx,y+ty,video::SColor(alpha,r1,g1,b1));
                                break;
                            }
                            case 1:
                            {
                                image->setPixel(x+tx,y+ty,video::SColor(alpha,r2,g2,b2));
                                break;
                            }
                            case 2:
                            {
                                if(transparency_bit==false)
                                    image->setPixel(x+tx,y+ty,video::SColor(alpha,(u32)(0.667f*r1+0.333f*r2),(u32)(0.667f*g1+0.333f*g2),(u32)(0.667f*b1+0.333f*b2)));
                                else
                                //image->setPixel(x+tx,y+ty,video::SColor(255,255,0,0));
                                    image->setPixel(x+tx,y+ty,video::SColor(255,(u32)(0.5f*r1+0.5f*r2),(u32)(0.5f*g1+0.5f*g2),(u32)(0.5f*b1+0.5f*b2)));
                                break;
                            }
                            case 3:
                            {
                                if(transparency_bit==false)
                                    image->setPixel(x+tx,y+ty,video::SColor(alpha,(u32)(0.333f*r1+0.667f*r2),(u32)(0.333f*g1+0.667f*g2),(u32)(0.333f*b1+0.667f*b2)));
                                else
                                    if(header.alpha_bitdepth==1)
                                        image->setPixel(x+tx,y+ty,video::SColor(0,0,0,0));
                                    else
                                        image->setPixel(x+tx,y+ty,video::SColor(255,0,0,0));
                                break;
                            }
                        }
                        tempbitmap=tempbitmap>>2;
                        if(header.alpha_bitdepth==8)
                        {
                            if(header.alpha_unk==7)
                            {
                            temptransp=temptransp>>3;
                            }
                            else
                            {
                            temptransp=temptransp>>4;
                            }
                        }
                    }
                }
                i++;
            }
        }
    }
    else//Palette Images
    {
        u8 index;
        for(u32 y=0;y<header.y_res;y++)
        {
            for(u32 x=0;x<header.x_res;x++)
            {
                file->read(&index,sizeof(u8));
                image->setPixel(x,y,video::SColor(255,palette[index].R,palette[index].G,palette[index].B));
            }
        }
        if(header.alpha_bitdepth==1)//surely not the best way.
        {
            for(u32 y=0;y<header.y_res;y++)
            {
                video::SColor pixel;
                for(u32 x=0;x<header.x_res;x=x+8)
                {
                    file->read(&index,sizeof(u8));
                    for(u32 i=0;i<8;i++)
                    {
                        pixel =image->getPixel(x+i,y);
                        pixel.setAlpha(255*(index & 1));
                        image->setPixel(x+i,y,pixel);
                        index =index >>1;
                    }
                }
            }
        }

        if(header.alpha_bitdepth==8)//surely not the best way.
        {
            for(u32 y=0;y<header.y_res;y++)
            {
                video::SColor pixel;
                for(u32 x=0;x<header.x_res;x++)
                {
                    file->read(&index,sizeof(u8));
                    pixel =image->getPixel(x,y);
                    pixel.setAlpha(index);
                    image->setPixel(x,y,pixel);
                }
            }
        }
    }

	return image;
}

}//namespace video
}//namespace irr
