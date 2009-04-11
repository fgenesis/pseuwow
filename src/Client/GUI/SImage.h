//most simplistic IImage Implementation, copypasted form irrlichts CImage
#include "irrlicht/irrlicht.h"
namespace irr
{
namespace video
{

class SImage : public IImage
{
public:
	//! constructor for empty image
	SImage(ECOLOR_FORMAT format, const core::dimension2d<s32>& size);
		//! destructor
	virtual ~SImage();

	//! returns a pixel
	virtual SColor getPixel(u32 x, u32 y) const;

	//! sets a pixel
	virtual void setPixel(u32 x, u32 y, const SColor &color );

	//! Lock function.
	virtual void* lock()
	{
		return Data;
	};

	//! Unlock function.
	virtual void unlock() {};

	//! Returns width and height of image data.
	virtual const core::dimension2d<s32>& getDimension() const;

	//! Returns bits per pixel.
	virtual u32 getBitsPerPixel() const;

	//! Returns bytes per pixel
	virtual u32 getBytesPerPixel() const;

	//! Returns image data size in bytes
	virtual u32 getImageDataSizeInBytes() const;

	//! Returns image data size in pixels
	virtual u32 getImageDataSizeInPixels() const;

	//! returns mask for red value of a pixel
	virtual u32 getRedMask() const;

	//! returns mask for green value of a pixel
	virtual u32 getGreenMask() const;

	//! returns mask for blue value of a pixel
	virtual u32 getBlueMask() const;

	//! returns mask for alpha value of a pixel
	virtual u32 getAlphaMask() const;

	//! returns the color format
	virtual ECOLOR_FORMAT getColorFormat() const;

	//! copies this surface into another
	void copyTo(IImage* target, const core::position2d<s32>& pos=core::position2d<s32>(0,0));

	//! copies this surface into another
	void copyTo(IImage* target, const core::position2d<s32>& pos, const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect=0);

	//! copies this surface into another, using the alpha mask, an cliprect and a color to add with
	virtual void copyToWithAlpha(IImage* target, const core::position2d<s32>& pos,
			const core::rect<s32>& sourceRect, const SColor &color,
			const core::rect<s32>* clipRect = 0);

	//! fills the surface with black or white
	virtual void fill(const SColor &color);

	//! copies this surface into another, scaling it to fit.
	void copyToScaling(void* target, s32 width, s32 height, ECOLOR_FORMAT format, u32 pitch=0);

	//! copies this surface into another, scaling it to fit.
	void copyToScaling(IImage* target);

    //! returns pitch of image
	virtual u32 getPitch() const
	{
		return Pitch;
	}

    static u32 getBitsPerPixelFromFormat(ECOLOR_FORMAT format);
private:

	//! assumes format and size has been set and creates the rest
	void initData();

	void setBitMasks();
	inline SColor getPixelBox ( s32 x, s32 y, s32 fx, s32 fy, s32 bias ) const;
	void* Data;
    core::dimension2d<s32> Size;
	u32 BitsPerPixel;
	u32 BytesPerPixel;
	u32 Pitch;
	ECOLOR_FORMAT Format;

    u32 RedMask;
	u32 GreenMask;
	u32 BlueMask;
	u32 AlphaMask;
};

}//video
}//irr
