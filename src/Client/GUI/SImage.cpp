#include "SImage.h"
namespace irr
{
namespace video
{


	//! constructor for empty image
//! constructor
SImage::SImage(ECOLOR_FORMAT format, const core::dimension2d<s32>& size): Size(size), Format(format), Data(0)
{
	initData();
}

void SImage::initData()
{
    setBitMasks();
	BitsPerPixel = getBitsPerPixelFromFormat(Format);
	BytesPerPixel = BitsPerPixel / 8;

	// Pitch should be aligned...
	Pitch = BytesPerPixel * Size.Width;

	if (!Data)
		Data = new s8[Size.Height * Pitch];
}

u32 SImage::getBitsPerPixelFromFormat(ECOLOR_FORMAT format)
{
	switch(format)
	{
	case ECF_A1R5G5B5:
		return 16;
	case ECF_R5G6B5:
		return 16;
	case ECF_R8G8B8:
		return 24;
	case ECF_A8R8G8B8:
		return 32;
	}

	return 0;
}
SImage::~SImage()
{
    if (Data)
        delete Data;
}


//! Returns width and height of image data.
const core::dimension2d<s32>& SImage::getDimension() const
{
	return Size;
}



//! Returns bits per pixel.
u32 SImage::getBitsPerPixel() const
{
	return BitsPerPixel;
}


//! Returns bytes per pixel
u32 SImage::getBytesPerPixel() const
{
	return BytesPerPixel;
}



//! Returns image data size in bytes
u32 SImage::getImageDataSizeInBytes() const
{
	return Pitch * Size.Height;
}



//! Returns image data size in pixels
u32 SImage::getImageDataSizeInPixels() const
{
	return Size.Width * Size.Height;
}



//! returns mask for red value of a pixel
u32 SImage::getRedMask() const
{
	return RedMask;
}



//! returns mask for green value of a pixel
u32 SImage::getGreenMask() const
{
	return GreenMask;
}



//! returns mask for blue value of a pixel
u32 SImage::getBlueMask() const
{
	return BlueMask;
}



//! returns mask for alpha value of a pixel
u32 SImage::getAlphaMask() const
{
	return AlphaMask;
}

void SImage::setBitMasks()
{
	switch(Format)
	{
	case ECF_A1R5G5B5:
		AlphaMask = 0x1<<15;
		RedMask = 0x1F<<10;
		GreenMask = 0x1F<<5;
		BlueMask = 0x1F;
	break;
	case ECF_R5G6B5:
		AlphaMask = 0x0;
		RedMask = 0x1F<<11;
		GreenMask = 0x3F<<5;
		BlueMask = 0x1F;
	break;
	case ECF_R8G8B8:
		AlphaMask = 0x0;
		RedMask   = 0x00FF0000;
		GreenMask = 0x0000FF00;
		BlueMask  = 0x000000FF;
	break;
	case ECF_A8R8G8B8:
		AlphaMask = 0xFF000000;
		RedMask   = 0x00FF0000;
		GreenMask = 0x0000FF00;
		BlueMask  = 0x000000FF;
	break;
	}
}

//! sets a pixel
void SImage::setPixel(u32 x, u32 y, const SColor &color )
{
	if (x >= (u32)Size.Width || y >= (u32)Size.Height)
		return;

	switch(Format)
	{
		case ECF_A1R5G5B5:
		{
			u16 * dest = (u16*) ((u8*) Data + ( y * Pitch ) + ( x << 1 ));
			*dest = video::A8R8G8B8toA1R5G5B5 ( color.color );
		} break;

		case ECF_R5G6B5:
		{
			u16 * dest = (u16*) ((u8*) Data + ( y * Pitch ) + ( x << 1 ));
			*dest = video::A8R8G8B8toR5G6B5 ( color.color );
		} break;

		case ECF_R8G8B8:
		{
			u8* dest = (u8*) Data + ( y * Pitch ) + ( x * 3 );
			dest[0] = color.getRed();
			dest[1] = color.getGreen();
			dest[2] = color.getBlue();
		} break;

		case ECF_A8R8G8B8:
		{
			u32 * dest = (u32*) ((u8*) Data + ( y * Pitch ) + ( x << 2 ));
			*dest = color.color;
		} break;
	}
}


//! returns a pixel
SColor SImage::getPixel(u32 x, u32 y) const
{
	if (x >= (u32)Size.Width || y >= (u32)Size.Height)
		return SColor(0);

	switch(Format)
	{
	case ECF_A1R5G5B5:
		return A1R5G5B5toA8R8G8B8(((u16*)Data)[y*Size.Width + x]);
	case ECF_R5G6B5:
		return R5G6B5toA8R8G8B8(((u16*)Data)[y*Size.Width + x]);
	case ECF_A8R8G8B8:
		return ((u32*)Data)[y*Size.Width + x];
	case ECF_R8G8B8:
		{
			u8* p = &((u8*)Data)[(y*3)*Size.Width + (x*3)];
			return SColor(255,p[0],p[1],p[2]);
		}
	}

	return SColor(0);
}


//! returns the color format
ECOLOR_FORMAT SImage::getColorFormat() const
{
	return Format;
}

//! copies this surface into another, scaling it to the target image size
// note: this is very very slow. (i didn't want to write a fast version.
// but hopefully, nobody wants to scale surfaces every frame.
void SImage::copyToScaling(void* target, s32 width, s32 height, ECOLOR_FORMAT format, u32 pitch)
{
	if (!target || !width || !height)
		return;

	const u32 bpp=getBitsPerPixelFromFormat(format)/8;
	if (0==pitch)
		pitch = width*bpp;

	if (Format==format && Size.Width==width && Size.Height==height)
	{
		if (pitch==Pitch)
		{
			memcpy(target, Data, height*pitch);
			return;
		}
		else
		{
			u8* tgtpos = (u8*) target;
			u8* dstpos = (u8*) Data;
			const u32 bwidth = width*bpp;
			for (s32 y=0; y<height; ++y)
			{
				memcpy(target, Data, height*pitch);
				memset(tgtpos+width, 0, pitch-bwidth);
				tgtpos += pitch;
				dstpos += Pitch;
			}
			return;
		}
	}

	const f32 sourceXStep = (f32)Size.Width / (f32)width;
	const f32 sourceYStep = (f32)Size.Height / (f32)height;
	s32 yval=0, syval=0;
	f32 sy = 0.0f;
	for (s32 y=0; y<height; ++y)
	{
		f32 sx = 0.0f;
		for (s32 x=0; x<width; ++x)
		{
//			CColorConverter::convert_viaFormat(((u8*)Data)+ syval + ((s32)sx)*BytesPerPixel, Format, 1, ((u8*)target)+ yval + (x*bpp), format);
			sx+=sourceXStep;
		}
		sy+=sourceYStep;
		syval=((s32)sy)*Pitch;
		yval+=pitch;
	}
}

//! copies this surface into another, scaling it to the target image size
// note: this is very very slow. (i didn't want to write a fast version.
// but hopefully, nobody wants to scale surfaces every frame.
void SImage::copyToScaling(IImage* target)
{
	if (!target)
		return;

	const core::dimension2d<s32>& targetSize = target->getDimension();

	if (targetSize==Size)
	{
		copyTo(target);
		return;
	}

	copyToScaling(target->lock(), targetSize.Width, targetSize.Height, target->getColorFormat());
	target->unlock();
}

//! copies this surface into another
void SImage::copyTo(IImage* target, const core::position2d<s32>& pos)
{
//	Blit (	BLITTER_TEXTURE, target, 0, &pos, this, 0, 0 );
}


//! copies this surface into another
void SImage::copyTo(IImage* target, const core::position2d<s32>& pos, const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect)
{
//	Blit (	BLITTER_TEXTURE, target, clipRect, &pos, this, &sourceRect, 0 );
}


}//video
}//irr

