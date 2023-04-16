// xImaPal.cpp : Palette and Pixel functions
/* 07/08/2001 v1.00 - Davide Pizzolato - www.xdp.it
 * CxImage version 7.0.1 07/Jan/2011
 */

#include "ximage.h"

////////////////////////////////////////////////////////////////////////////////
/**
 * returns the palette dimension in byte
 */
uint32_t CxImage::GetPaletteSize()
{
	return (head.biClrUsed * sizeof(RGBQUAD));
}
////////////////////////////////////////////////////////////////////////////////
void CxImage::SetPaletteColor(uint8_t idx, uint8_t r, uint8_t g, uint8_t b, uint8_t alpha)
{
	if ((pDib)&&(head.biClrUsed)){
		uint8_t* iDst = (uint8_t*)(pDib) + sizeof(BITMAPINFOHEADER);
		if (idx<head.biClrUsed){
			int32_t ldx=idx*sizeof(RGBQUAD);
			iDst[ldx++] = (uint8_t) b;
			iDst[ldx++] = (uint8_t) g;
			iDst[ldx++] = (uint8_t) r;
			iDst[ldx] = (uint8_t) alpha;
			info.last_c_isvalid = false;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////
void CxImage::SetPaletteColor(uint8_t idx, RGBQUAD c)
{
	if ((pDib)&&(head.biClrUsed)){
		uint8_t* iDst = (uint8_t*)(pDib) + sizeof(BITMAPINFOHEADER);
		if (idx<head.biClrUsed){
			int32_t ldx=idx*sizeof(RGBQUAD);
			iDst[ldx++] = (uint8_t) c.rgbBlue;
			iDst[ldx++] = (uint8_t) c.rgbGreen;
			iDst[ldx++] = (uint8_t) c.rgbRed;
			iDst[ldx] = (uint8_t) c.rgbReserved;
			info.last_c_isvalid = false;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////
void CxImage::SetPaletteColor(uint8_t idx, COLORREF cr)
{
	if ((pDib)&&(head.biClrUsed)){
		uint8_t* iDst = (uint8_t*)(pDib) + sizeof(BITMAPINFOHEADER);
		if (idx<head.biClrUsed){
			int32_t ldx=idx*sizeof(RGBQUAD);
			iDst[ldx++] = (uint8_t) GetBValue(cr);
			iDst[ldx++] = (uint8_t) GetGValue(cr);
			iDst[ldx++] = (uint8_t) GetRValue(cr);
			iDst[ldx] = (uint8_t) 0;
			info.last_c_isvalid = false;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////
/**
 * returns the pointer to the first palette index
 */
RGBQUAD* CxImage::GetPalette() const
{
	if ((pDib)&&(head.biClrUsed))
		return (RGBQUAD*)((uint8_t*)pDib + sizeof(BITMAPINFOHEADER));
	return NULL;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Returns the color of the specified index.
 */
RGBQUAD CxImage::GetPaletteColor(uint8_t idx)
{
	RGBQUAD rgb = {0,0,0,0};
	if ((pDib)&&(head.biClrUsed)){
		uint8_t* iDst = (uint8_t*)(pDib) + sizeof(BITMAPINFOHEADER);
		if (idx<head.biClrUsed){
			int32_t ldx=idx*sizeof(RGBQUAD);
			rgb.rgbBlue = iDst[ldx++];
			rgb.rgbGreen=iDst[ldx++];
			rgb.rgbRed =iDst[ldx++];
			rgb.rgbReserved = iDst[ldx];
		}
	}
	return rgb;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Returns the palette index of the specified pixel.
 */
uint8_t CxImage::GetPixelIndex(int32_t x,int32_t y)
{
	if ((pDib==NULL)||(head.biClrUsed==0)) return 0;

	if ((x<0)||(y<0)||(x>=head.biWidth)||(y>=head.biHeight)) {
		if (info.nBkgndIndex >= 0)	return (uint8_t)info.nBkgndIndex;
		else return *info.pImage;
	}
	if (head.biBitCount==8){
		return info.pImage[y*info.dwEffWidth + x];
	} else {
		uint8_t pos;
		uint8_t iDst= info.pImage[y*info.dwEffWidth + (x*head.biBitCount >> 3)];
		if (head.biBitCount==4){
			pos = (uint8_t)(4*(1-x%2));
			iDst &= (0x0F<<pos);
			return (uint8_t)(iDst >> pos);
		} else if (head.biBitCount==1){
			pos = (uint8_t)(7-x%8);
			iDst &= (0x01<<pos);
			return (uint8_t)(iDst >> pos);
		}
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////
uint8_t CxImage::BlindGetPixelIndex(const int32_t x,const int32_t y)
{
#ifdef _DEBUG
	if ((pDib==NULL) || (head.biClrUsed==0) || !IsInside(x,y))
  #if CXIMAGE_SUPPORT_EXCEPTION_HANDLING
		throw 0;
  #else
		return 0;
  #endif
#endif

	if (head.biBitCount==8){
		return info.pImage[y*info.dwEffWidth + x];
	} else {
		uint8_t pos;
		uint8_t iDst= info.pImage[y*info.dwEffWidth + (x*head.biBitCount >> 3)];
		if (head.biBitCount==4){
			pos = (uint8_t)(4*(1-x%2));
			iDst &= (0x0F<<pos);
			return (uint8_t)(iDst >> pos);
		} else if (head.biBitCount==1){
			pos = (uint8_t)(7-x%8);
			iDst &= (0x01<<pos);
			return (uint8_t)(iDst >> pos);
		}
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////
RGBQUAD CxImage::GetPixelColor(int32_t x,int32_t y, bool bGetAlpha)
{
//	RGBQUAD rgb={0,0,0,0};
	RGBQUAD rgb=info.nBkgndColor; //<mpwolski>
	if ((pDib==NULL)||(x<0)||(y<0)||
		(x>=head.biWidth)||(y>=head.biHeight)){
		if (info.nBkgndIndex >= 0){
			if (head.biBitCount<24) return GetPaletteColor((uint8_t)info.nBkgndIndex);
			else return info.nBkgndColor;
		} else if (pDib) return GetPixelColor(0,0);
		return rgb;
	}

	if (head.biClrUsed){
		rgb = GetPaletteColor(BlindGetPixelIndex(x,y));
	} else {
		uint8_t* iDst  = info.pImage + y*info.dwEffWidth + x*3;
		rgb.rgbBlue = *iDst++;
		rgb.rgbGreen= *iDst++;
		rgb.rgbRed  = *iDst;
	}
#if CXIMAGE_SUPPORT_ALPHA
	if (pAlpha && bGetAlpha) rgb.rgbReserved = BlindAlphaGet(x,y);
#else
	rgb.rgbReserved = 0;
#endif //CXIMAGE_SUPPORT_ALPHA
	return rgb;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * This is (a bit) faster version of GetPixelColor. 
 * It tests bounds only in debug mode (_DEBUG defined).
 * 
 * It is an error to request out-of-borders pixel with this method. 
 * In DEBUG mode an exception will be thrown, and data will be violated in non-DEBUG mode. 
 * \author ***bd*** 2.2004
 */
RGBQUAD CxImage::BlindGetPixelColor(const int32_t x,const int32_t y, bool bGetAlpha)
{
  RGBQUAD rgb;
#ifdef _DEBUG
	if ((pDib==NULL) || !IsInside(x,y))
  #if CXIMAGE_SUPPORT_EXCEPTION_HANDLING
		throw 0;
  #else
		{rgb.rgbReserved = 0; return rgb;}
  #endif
#endif

	if (head.biClrUsed){
		rgb = GetPaletteColor(BlindGetPixelIndex(x,y));
	} else {
		uint8_t* iDst  = info.pImage + y*info.dwEffWidth + x*3;
		rgb.rgbBlue = *iDst++;
		rgb.rgbGreen= *iDst++;
		rgb.rgbRed  = *iDst;
		rgb.rgbReserved = 0; //needed for images without alpha layer
	}
#if CXIMAGE_SUPPORT_ALPHA
	if (pAlpha && bGetAlpha) rgb.rgbReserved = BlindAlphaGet(x,y);
#else
	rgb.rgbReserved = 0;
#endif //CXIMAGE_SUPPORT_ALPHA
	return rgb;
}
////////////////////////////////////////////////////////////////////////////////
uint8_t CxImage::GetPixelGray(int32_t x, int32_t y)
{
	RGBQUAD color = GetPixelColor(x,y);
	return (uint8_t)RGB2GRAY(color.rgbRed,color.rgbGreen,color.rgbBlue);
}
////////////////////////////////////////////////////////////////////////////////
void CxImage::BlindSetPixelIndex(int32_t x,int32_t y,uint8_t i)
{
#ifdef _DEBUG
	if ((pDib==NULL)||(head.biClrUsed==0)||
		(x<0)||(y<0)||(x>=head.biWidth)||(y>=head.biHeight))
  #if CXIMAGE_SUPPORT_EXCEPTION_HANDLING
		throw 0;
  #else
		return;
  #endif
#endif

	if (head.biBitCount==8){
		info.pImage[y*info.dwEffWidth + x]=i;
		return;
	} else {
		uint8_t pos;
		uint8_t* iDst= info.pImage + y*info.dwEffWidth + (x*head.biBitCount >> 3);
		if (head.biBitCount==4){
			pos = (uint8_t)(4*(1-x%2));
			*iDst &= ~(0x0F<<pos);
			*iDst |= ((i & 0x0F)<<pos);
			return;
		} else if (head.biBitCount==1){
			pos = (uint8_t)(7-x%8);
			*iDst &= ~(0x01<<pos);
			*iDst |= ((i & 0x01)<<pos);
			return;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////
void CxImage::SetPixelIndex(int32_t x,int32_t y,uint8_t i)
{
	if ((pDib==NULL)||(head.biClrUsed==0)||
		(x<0)||(y<0)||(x>=head.biWidth)||(y>=head.biHeight)) return ;

	if (head.biBitCount==8){
		info.pImage[y*info.dwEffWidth + x]=i;
		return;
	} else {
		uint8_t pos;
		uint8_t* iDst= info.pImage + y*info.dwEffWidth + (x*head.biBitCount >> 3);
		if (head.biBitCount==4){
			pos = (uint8_t)(4*(1-x%2));
			*iDst &= ~(0x0F<<pos);
			*iDst |= ((i & 0x0F)<<pos);
			return;
		} else if (head.biBitCount==1){
			pos = (uint8_t)(7-x%8);
			*iDst &= ~(0x01<<pos);
			*iDst |= ((i & 0x01)<<pos);
			return;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////
void CxImage::SetPixelColor(int32_t x,int32_t y,COLORREF cr)
{
	SetPixelColor(x,y,RGBtoRGBQUAD(cr));
}
////////////////////////////////////////////////////////////////////////////////
void CxImage::BlindSetPixelColor(int32_t x,int32_t y,RGBQUAD c, bool bSetAlpha)
{
#ifdef _DEBUG
	if ((pDib==NULL)||(x<0)||(y<0)||
		(x>=head.biWidth)||(y>=head.biHeight))
  #if CXIMAGE_SUPPORT_EXCEPTION_HANDLING
		throw 0;
  #else
		return;
  #endif
#endif
	if (head.biClrUsed)
		BlindSetPixelIndex(x,y,GetNearestIndex(c));
	else {
		uint8_t* iDst = info.pImage + y*info.dwEffWidth + x*3;
		*iDst++ = c.rgbBlue;
		*iDst++ = c.rgbGreen;
		*iDst   = c.rgbRed;
	}
#if CXIMAGE_SUPPORT_ALPHA
	if (bSetAlpha) AlphaSet(x,y,c.rgbReserved);
#endif //CXIMAGE_SUPPORT_ALPHA
}
////////////////////////////////////////////////////////////////////////////////
void CxImage::SetPixelColor(int32_t x,int32_t y,RGBQUAD c, bool bSetAlpha)
{
	if ((pDib==NULL)||(x<0)||(y<0)||
		(x>=head.biWidth)||(y>=head.biHeight)) return;
	if (head.biClrUsed)
		BlindSetPixelIndex(x,y,GetNearestIndex(c));
	else {
		uint8_t* iDst = info.pImage + y*info.dwEffWidth + x*3;
		*iDst++ = c.rgbBlue;
		*iDst++ = c.rgbGreen;
		*iDst   = c.rgbRed;
	}
#if CXIMAGE_SUPPORT_ALPHA
	if (bSetAlpha) AlphaSet(x,y,c.rgbReserved);
#endif //CXIMAGE_SUPPORT_ALPHA
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Blends the current pixel color with a new color.
 * \param x,y = pixel
 * \param c = new color
 * \param blend = can be from 0 (no effect) to 1 (full effect).
 * \param bSetAlpha = if true, blends also the alpha component stored in c.rgbReserved
 */
void CxImage::BlendPixelColor(int32_t x,int32_t y,RGBQUAD c, float blend, bool bSetAlpha)
{
	if ((pDib==NULL)||(x<0)||(y<0)||
		(x>=head.biWidth)||(y>=head.biHeight)) return;

	int32_t a0 = (int32_t)(256*blend);
	int32_t a1 = 256 - a0;

	RGBQUAD c0 = BlindGetPixelColor(x,y);
	c.rgbRed  = (uint8_t)((c.rgbRed * a0 + c0.rgbRed * a1)>>8);
	c.rgbBlue  = (uint8_t)((c.rgbBlue * a0 + c0.rgbBlue * a1)>>8);
	c.rgbGreen  = (uint8_t)((c.rgbGreen * a0 + c0.rgbGreen * a1)>>8);

	if (head.biClrUsed)
		BlindSetPixelIndex(x,y,GetNearestIndex(c));
	else {
		uint8_t* iDst = info.pImage + y*info.dwEffWidth + x*3;
		*iDst++ = c.rgbBlue;
		*iDst++ = c.rgbGreen;
		*iDst   = c.rgbRed;
#if CXIMAGE_SUPPORT_ALPHA
		if (bSetAlpha) AlphaSet(x,y,c.rgbReserved);
#endif //CXIMAGE_SUPPORT_ALPHA
	}
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Returns the best palette index that matches a specified color.
 */
uint8_t CxImage::GetNearestIndex(RGBQUAD c)
{
	if ((pDib==NULL)||(head.biClrUsed==0)) return 0;

	// <RJ> check matching with the previous result
	if (info.last_c_isvalid && (*(int32_t*)&info.last_c == *(int32_t*)&c)) return info.last_c_index;
	info.last_c = c;
	info.last_c_isvalid = true;

	uint8_t* iDst = (uint8_t*)(pDib) + sizeof(BITMAPINFOHEADER);
	int32_t distance=200000;
	int32_t i,j = 0;
	int32_t k,l;
	int32_t m = (int32_t)(head.biClrImportant==0 ? head.biClrUsed : head.biClrImportant);
	for(i=0,l=0;i<m;i++,l+=sizeof(RGBQUAD)){
		k = (iDst[l]-c.rgbBlue)*(iDst[l]-c.rgbBlue)+
			(iDst[l+1]-c.rgbGreen)*(iDst[l+1]-c.rgbGreen)+
			(iDst[l+2]-c.rgbRed)*(iDst[l+2]-c.rgbRed);
//		k = abs(iDst[l]-c.rgbBlue)+abs(iDst[l+1]-c.rgbGreen)+abs(iDst[l+2]-c.rgbRed);
		if (k==0){
			j=i;
			break;
		}
		if (k<distance){
			distance=k;
			j=i;
		}
	}
	info.last_c_index = (uint8_t)j;
	return (uint8_t)j;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * swaps the blue and red components (for RGB images)
 * \param buffer : pointer to the pixels
 * \param length : number of bytes to swap. lenght may not exceed the scan line.
 */
void CxImage::RGBtoBGR(uint8_t *buffer, int32_t length)
{
	if (buffer && (head.biClrUsed==0)){
		uint8_t temp;
		length = min(length,(int32_t)info.dwEffWidth);
		length = min(length,(int32_t)(3*head.biWidth));
		for (int32_t i=0;i<length;i+=3){
			temp = buffer[i]; buffer[i] = buffer[i+2]; buffer[i+2] = temp;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////
RGBQUAD CxImage::RGBtoRGBQUAD(COLORREF cr)
{
	RGBQUAD c;
	c.rgbRed = GetRValue(cr);	/* get R, G, and B out of uint32_t */
	c.rgbGreen = GetGValue(cr);
	c.rgbBlue = GetBValue(cr);
	c.rgbReserved=0;
	return c;
}
////////////////////////////////////////////////////////////////////////////////
COLORREF CxImage::RGBQUADtoRGB (RGBQUAD c)
{
	return RGB(c.rgbRed,c.rgbGreen,c.rgbBlue);
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Returns the color of the specified index.
 * \param i = palette index
 * \param r, g, b = output color channels
 */
bool CxImage::GetPaletteColor(uint8_t i, uint8_t* r, uint8_t* g, uint8_t* b)
{
	RGBQUAD* ppal=GetPalette();
	if (ppal) {
		*r = ppal[i].rgbRed;
		*g = ppal[i].rgbGreen;
		*b = ppal[i].rgbBlue; 
		return true;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////
void CxImage::SetPalette(uint32_t n, uint8_t *r, uint8_t *g, uint8_t *b)
{
	if ((!r)||(pDib==NULL)||(head.biClrUsed==0)) return;
	if (!g) g = r;
	if (!b) b = g;
	RGBQUAD* ppal=GetPalette();
	uint32_t m=min(n,head.biClrUsed);
	for (uint32_t i=0; i<m;i++){
		ppal[i].rgbRed=r[i];
		ppal[i].rgbGreen=g[i];
		ppal[i].rgbBlue=b[i];
	}
	info.last_c_isvalid = false;
}
////////////////////////////////////////////////////////////////////////////////
void CxImage::SetPalette(rgb_color *rgb,uint32_t nColors)
{
	if ((!rgb)||(pDib==NULL)||(head.biClrUsed==0)) return;
	RGBQUAD* ppal=GetPalette();
	uint32_t m=min(nColors,head.biClrUsed);
	for (uint32_t i=0; i<m;i++){
		ppal[i].rgbRed=rgb[i].r;
		ppal[i].rgbGreen=rgb[i].g;
		ppal[i].rgbBlue=rgb[i].b;
	}
	info.last_c_isvalid = false;
}
////////////////////////////////////////////////////////////////////////////////
void CxImage::SetPalette(RGBQUAD* pPal,uint32_t nColors)
{
	if ((pPal==NULL)||(pDib==NULL)||(head.biClrUsed==0)) return;
	memcpy(GetPalette(),pPal,min(GetPaletteSize(),nColors*sizeof(RGBQUAD)));
	info.last_c_isvalid = false;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Sets (or replaces) the palette to gray scale palette.
 * The function doesn't change the pixels; for standard
 * gray scale conversion use GrayScale().
 */
void CxImage::SetGrayPalette()
{
	if ((pDib==NULL)||(head.biClrUsed==0)) return;
	RGBQUAD* pal=GetPalette();
	for (uint32_t ni=0;ni<head.biClrUsed;ni++)
		pal[ni].rgbBlue=pal[ni].rgbGreen = pal[ni].rgbRed = (uint8_t)(ni*(255/(head.biClrUsed-1)));
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Colorize the palette.
 * \sa Colorize
 */
void CxImage::BlendPalette(COLORREF cr,int32_t perc)
{
	if ((pDib==NULL)||(head.biClrUsed==0)) return;
	uint8_t* iDst = (uint8_t*)(pDib) + sizeof(BITMAPINFOHEADER);
	uint32_t i,r,g,b;
	RGBQUAD* pPal=(RGBQUAD*)iDst;
	r = GetRValue(cr);
	g = GetGValue(cr);
	b = GetBValue(cr);
	if (perc>100) perc=100;
	for(i=0;i<head.biClrUsed;i++){
		pPal[i].rgbBlue=(uint8_t)((pPal[i].rgbBlue*(100-perc)+b*perc)/100);
		pPal[i].rgbGreen =(uint8_t)((pPal[i].rgbGreen*(100-perc)+g*perc)/100);
		pPal[i].rgbRed =(uint8_t)((pPal[i].rgbRed*(100-perc)+r*perc)/100);
	}
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Returns true if the image has 256 colors and a linear grey scale palette.
 */
bool CxImage::IsGrayScale()
{
	RGBQUAD* ppal=GetPalette();
	if(!(pDib && ppal && head.biClrUsed)) return false;
	for(uint32_t i=0;i<head.biClrUsed;i++){
		if (ppal[i].rgbBlue!=i || ppal[i].rgbGreen!=i || ppal[i].rgbRed!=i) return false;
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * swap two indexes in the image and their colors in the palette
 */
void CxImage::SwapIndex(uint8_t idx1, uint8_t idx2)
{
	RGBQUAD* ppal=GetPalette();
	if(!(pDib && ppal)) return;
	//swap the colors
	RGBQUAD tempRGB=GetPaletteColor(idx1);
	SetPaletteColor(idx1,GetPaletteColor(idx2));
	SetPaletteColor(idx2,tempRGB);
	//swap the pixels
	uint8_t idx;
	for(int32_t y=0; y < head.biHeight; y++){
		for(int32_t x=0; x < head.biWidth; x++){
			idx=BlindGetPixelIndex(x,y);
			if (idx==idx1) BlindSetPixelIndex(x,y,idx2);
			if (idx==idx2) BlindSetPixelIndex(x,y,idx1);
		}
	}
}
////////////////////////////////////////////////////////////////////////////////
/**
 * swap Red and Blue colors
 */
void CxImage::SwapRGB2BGR()
{
	if (!pDib) return;

	if (head.biClrUsed){
		RGBQUAD* ppal=GetPalette();
		uint8_t b;
		if(!ppal) return;
		for(uint16_t a=0;a<head.biClrUsed;a++){
			b=ppal[a].rgbBlue; ppal[a].rgbBlue=ppal[a].rgbRed; ppal[a].rgbRed=b;
		}
	} else {
		for(int32_t y=0;y<head.biHeight;y++){
			RGBtoBGR(GetBits(y),3*head.biWidth);
		}
	}
}
////////////////////////////////////////////////////////////////////////////////
bool CxImage::IsTransparent(int32_t x, int32_t y)
{
	if (!pDib) return false;

	if (info.nBkgndIndex>=0){
		if (head.biClrUsed){
			if (GetPixelIndex(x,y) == info.nBkgndIndex) return true;
		} else {
			RGBQUAD ct = info.nBkgndColor;
			RGBQUAD c = GetPixelColor(x,y,false);
			if (*(int32_t*)&c==*(int32_t*)&ct) return true;
		}
	}

#if CXIMAGE_SUPPORT_ALPHA
	if (pAlpha) return AlphaGet(x,y)==0;
#endif

	return false;
}
////////////////////////////////////////////////////////////////////////////////
bool CxImage::GetTransparentMask(CxImage* iDst)
{
	if (!pDib) return false;

	CxImage tmp;
	tmp.Create(head.biWidth, head.biHeight, 1, GetType());
	tmp.SetStdPalette();
	tmp.Clear(0);

	for(int32_t y=0; y<head.biHeight; y++){
		for(int32_t x=0; x<head.biWidth; x++){
			if (IsTransparent(x,y)){
				tmp.BlindSetPixelIndex(x,y,1);
			}
		}
	}

	if (iDst) iDst->Transfer(tmp);
	else Transfer(tmp);

	return true;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Checks if image has the same palette, if any.
 * \param img = image to compare.
 * \param bCheckAlpha = check also the rgbReserved field.
 */
bool CxImage::IsSamePalette(CxImage &img, bool bCheckAlpha)
{
	if (head.biClrUsed != img.head.biClrUsed)
		return false;
	if (head.biClrUsed == 0)
		return false;

	RGBQUAD c1,c2;
	for (uint32_t n=0; n<head.biClrUsed; n++){
		c1 = GetPaletteColor((uint8_t)n);
		c2 = img.GetPaletteColor((uint8_t)n);
		if (c1.rgbRed != c2.rgbRed) return false;
		if (c1.rgbBlue != c2.rgbBlue) return false;
		if (c1.rgbGreen != c2.rgbGreen) return false;
		if (bCheckAlpha && (c1.rgbReserved != c2.rgbReserved)) return false;
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * \sa SetClrImportant
 */
uint32_t CxImage::GetClrImportant() const
{
	return head.biClrImportant;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * sets the maximum number of colors that some functions like
 * DecreaseBpp() or GetNearestIndex() will use on indexed images
 * \param ncolors should be less than 2^bpp,
 * or 0 if all the colors are important.
 */
void CxImage::SetClrImportant(uint32_t ncolors)
{
	if (ncolors==0 || ncolors>256) {
		head.biClrImportant = 0;
		return;
	}

	switch(head.biBitCount){
	case 1:
		head.biClrImportant = min(ncolors,2);
		break;
	case 4:
		head.biClrImportant = min(ncolors,16);
		break;
	case 8:
		head.biClrImportant = ncolors;
		break;
	}
	return;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Returns pointer to pixel. Currently implemented only for truecolor images.
 *  
 * \param  x,y - coordinates
 *
 * \return pointer to first byte of pixel data
 *
 * \author ***bd*** 2.2004
 */
void* CxImage::BlindGetPixelPointer(const int32_t x, const int32_t y)
{
#ifdef _DEBUG
	if ((pDib==NULL) || !IsInside(x,y))
  #if CXIMAGE_SUPPORT_EXCEPTION_HANDLING
		throw 0;
  #else
		return 0;
  #endif
#endif
  if (!IsIndexed())
    return info.pImage + y*info.dwEffWidth + x*3;
  else
    return 0;
}
////////////////////////////////////////////////////////////////////////////////
void CxImage::DrawLine(int32_t StartX, int32_t EndX, int32_t StartY, int32_t EndY, COLORREF cr)
{
	DrawLine(StartX, EndX, StartY, EndY, RGBtoRGBQUAD(cr));
}
////////////////////////////////////////////////////////////////////////////////
void CxImage::DrawLine(int32_t StartX, int32_t EndX, int32_t StartY, int32_t EndY, RGBQUAD color, bool bSetAlpha)
{
	if (!pDib) return;
	//////////////////////////////////////////////////////
	// Draws a line using the Bresenham line algorithm
	// Thanks to Jordan DeLozier <JDL>
	//////////////////////////////////////////////////////
	int32_t x1 = StartX;
	int32_t y1 = StartY;
	int32_t x = x1;                       // Start x off at the first pixel
	int32_t y = y1;                       // Start y off at the first pixel
	int32_t x2 = EndX;
	int32_t y2 = EndY;

	int32_t xinc1,xinc2,yinc1,yinc2;      // Increasing values
	int32_t den, num, numadd,numpixels;   
	int32_t deltax = abs(x2 - x1);        // The difference between the x's
	int32_t deltay = abs(y2 - y1);        // The difference between the y's

	// Get Increasing Values
	if (x2 >= x1) {                // The x-values are increasing
		xinc1 = 1;
		xinc2 = 1;
	} else {                         // The x-values are decreasing
		xinc1 = -1;
		xinc2 = -1;
	}

	if (y2 >= y1) {                // The y-values are increasing
		yinc1 = 1;
		yinc2 = 1;
	} else {                         // The y-values are decreasing
		yinc1 = -1;
		yinc2 = -1;
	}

	// Actually draw the line
	if (deltax >= deltay)         // There is at least one x-value for every y-value
	{
		xinc1 = 0;                  // Don't change the x when numerator >= denominator
		yinc2 = 0;                  // Don't change the y for every iteration
		den = deltax;
		num = deltax / 2;
		numadd = deltay;
		numpixels = deltax;         // There are more x-values than y-values
	}
	else                          // There is at least one y-value for every x-value
	{
		xinc2 = 0;                  // Don't change the x for every iteration
		yinc1 = 0;                  // Don't change the y when numerator >= denominator
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay;         // There are more y-values than x-values
	}
	
	for (int32_t curpixel = 0; curpixel <= numpixels; curpixel++)
	{
		// Draw the current pixel
		SetPixelColor(x,y,color,bSetAlpha);
		
		num += numadd;              // Increase the numerator by the top of the fraction
		if (num >= den)             // Check if numerator >= denominator
		{
			num -= den;               // Calculate the new numerator value
			x += xinc1;               // Change the x as appropriate
			y += yinc1;               // Change the y as appropriate
		}
		x += xinc2;                 // Change the x as appropriate
		y += yinc2;                 // Change the y as appropriate
	}
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Sets a palette with standard colors for 1, 4 and 8 bpp images.
 */
void CxImage::SetStdPalette()
{
	if (!pDib) return;
	switch (head.biBitCount){
	case 8:
		{
		const uint8_t pal256[1024] = { 0, 0, 0, 0,
			 128,   0,   0, 0 ,    // 001 dark red
			 0, 128,   0, 0 ,    // 002 dark green
			 128, 128,   0, 0 ,    // 003 dark brown
			 0,   0, 128, 0 ,    // 004 dark blue
			 128,   0, 128, 0 ,    // 005 dark purple
			 0, 128, 128, 0 ,    // 006 dark teal
			 192, 192, 192, 0 ,    // 007 light gray
			 192, 220, 192, 0 ,    // 008 pale green
			 166, 202, 240, 0 ,    // 009 sky blue
			 4,   4,   4, 4 ,    // 010 dark gray ramp
			 8,   8,   8, 4 ,    // 011
			 12,  12,  12, 4 ,    // 012
			 17,  17,  17, 4 ,    // 013
			 22,  22,  22, 4 ,    // 014
			 28,  28,  28, 4 ,    // 015
			 34,  34,  34, 4 ,    // 016
			 41,  41,  41, 4 ,    // 017
			 85,  85,  85, 4 ,    // 018
			 77,  77,  77, 4 ,    // 019
			 66,  66,  66, 4 ,    // 020
			 57,  57,  57, 4 ,    // 021
			 255, 124, 128, 4 ,    // 022
			 255,  80,  80, 4 ,    // 023
			 214,   0, 147, 4 ,    // 024
			 204, 236, 255, 4 ,    // 025
			 239, 214, 198, 4 ,    // 026
			 231, 231, 214, 4 ,    // 027
			 173, 169, 144, 4 ,    // 028
			 51,   0,   0, 4 ,    // 029
			 102,   0,   0, 4 ,    // 030
			 153,   0,   0, 4 ,    // 031
			 204,   0,   0, 4 ,    // 032
			 0,  51,   0, 4 ,    // 033
			 51,  51,   0, 4 ,    // 034
			 102,  51,   0, 4 ,    // 035
			 153,  51,   0, 4 ,    // 036
			 204,  51,   0, 4 ,    // 037
			 255,  51,   0, 4 ,    // 038
			 0, 102,   0, 4 ,    // 039
			 51, 102,   0, 4 ,    // 040
			 102, 102,   0, 4 ,    // 041
			 153, 102,   0, 4 ,    // 042
			 204, 102,   0, 4 ,    // 043
			 255, 102,   0, 4 ,    // 044
			 0, 153,   0, 4 ,    // 045
			 51, 153,   0, 4 ,    // 046
			 102, 153,   0, 4 ,    // 047
			 153, 153,   0, 4 ,    // 048
			 204, 153,   0, 4 ,    // 049
			 255, 153,   0, 4 ,    // 050
			 0, 204,   0, 4 ,    // 051
			 51, 204,   0, 4 ,    // 052
			 102, 204,   0, 4 ,    // 053
			 153, 204,   0, 4 ,    // 054
			 204, 204,   0, 4 ,    // 055
			 255, 204,   0, 4 ,    // 056
			 102, 255,   0, 4 ,    // 057
			 153, 255,   0, 4 ,    // 058
			 204, 255,   0, 4 ,    // 059
			 0,   0,  51, 4 ,    // 060
			 51,   0,  51, 4 ,    // 061
			 102,   0,  51, 4 ,    // 062
			 153,   0,  51, 4 ,    // 063
			 204,   0,  51, 4 ,    // 064
			 255,   0,  51, 4 ,    // 065
			 0,  51,  51, 4 ,    // 066
			 51,  51,  51, 4 ,    // 067
			 102,  51,  51, 4 ,    // 068
			 153,  51,  51, 4 ,    // 069
			 204,  51,  51, 4 ,    // 070
			 255,  51,  51, 4 ,    // 071
			 0, 102,  51, 4 ,    // 072
			 51, 102,  51, 4 ,    // 073
			 102, 102,  51, 4 ,    // 074
			 153, 102,  51, 4 ,    // 075
			 204, 102,  51, 4 ,    // 076
			 255, 102,  51, 4 ,    // 077
			 0, 153,  51, 4 ,    // 078
			 51, 153,  51, 4 ,    // 079
			 102, 153,  51, 4 ,    // 080
			 153, 153,  51, 4 ,    // 081
			 204, 153,  51, 4 ,    // 082
			 255, 153,  51, 4 ,    // 083
			 0, 204,  51, 4 ,    // 084
			 51, 204,  51, 4 ,    // 085
			 102, 204,  51, 4 ,    // 086
			 153, 204,  51, 4 ,    // 087
			 204, 204,  51, 4 ,    // 088
			 255, 204,  51, 4 ,    // 089
			 51, 255,  51, 4 ,    // 090
			 102, 255,  51, 4 ,    // 091
			 153, 255,  51, 4 ,    // 092
			 204, 255,  51, 4 ,    // 093
			 255, 255,  51, 4 ,    // 094
			 0,   0, 102, 4 ,    // 095
			 51,   0, 102, 4 ,    // 096
			 102,   0, 102, 4 ,    // 097
			 153,   0, 102, 4 ,    // 098
			 204,   0, 102, 4 ,    // 099
			 255,   0, 102, 4 ,    // 100
			 0,  51, 102, 4 ,    // 101
			 51,  51, 102, 4 ,    // 102
			 102,  51, 102, 4 ,    // 103
			 153,  51, 102, 4 ,    // 104
			 204,  51, 102, 4 ,    // 105
			 255,  51, 102, 4 ,    // 106
			 0, 102, 102, 4 ,    // 107
			 51, 102, 102, 4 ,    // 108
			 102, 102, 102, 4 ,    // 109
			 153, 102, 102, 4 ,    // 110
			 204, 102, 102, 4 ,    // 111
			 0, 153, 102, 4 ,    // 112
			 51, 153, 102, 4 ,    // 113
			 102, 153, 102, 4 ,    // 114
			 153, 153, 102, 4 ,    // 115
			 204, 153, 102, 4 ,    // 116
			 255, 153, 102, 4 ,    // 117
			 0, 204, 102, 4 ,    // 118
			 51, 204, 102, 4 ,    // 119
			 153, 204, 102, 4 ,    // 120
			 204, 204, 102, 4 ,    // 121
			 255, 204, 102, 4 ,    // 122
			 0, 255, 102, 4 ,    // 123
			 51, 255, 102, 4 ,    // 124
			 153, 255, 102, 4 ,    // 125
			 204, 255, 102, 4 ,    // 126
			 255,   0, 204, 4 ,    // 127
			 204,   0, 255, 4 ,    // 128
			 0, 153, 153, 4 ,    // 129
			 153,  51, 153, 4 ,    // 130
			 153,   0, 153, 4 ,    // 131
			 204,   0, 153, 4 ,    // 132
			 0,   0, 153, 4 ,    // 133
			 51,  51, 153, 4 ,    // 134
			 102,   0, 153, 4 ,    // 135
			 204,  51, 153, 4 ,    // 136
			 255,   0, 153, 4 ,    // 137
			 0, 102, 153, 4 ,    // 138
			 51, 102, 153, 4 ,    // 139
			 102,  51, 153, 4 ,    // 140
			 153, 102, 153, 4 ,    // 141
			 204, 102, 153, 4 ,    // 142
			 255,  51, 153, 4 ,    // 143
			 51, 153, 153, 4 ,    // 144
			 102, 153, 153, 4 ,    // 145
			 153, 153, 153, 4 ,    // 146
			 204, 153, 153, 4 ,    // 147
			 255, 153, 153, 4 ,    // 148
			 0, 204, 153, 4 ,    // 149
			 51, 204, 153, 4 ,    // 150
			 102, 204, 102, 4 ,    // 151
			 153, 204, 153, 4 ,    // 152
			 204, 204, 153, 4 ,    // 153
			 255, 204, 153, 4 ,    // 154
			 0, 255, 153, 4 ,    // 155
			 51, 255, 153, 4 ,    // 156
			 102, 204, 153, 4 ,    // 157
			 153, 255, 153, 4 ,    // 158
			 204, 255, 153, 4 ,    // 159
			 255, 255, 153, 4 ,    // 160
			 0,   0, 204, 4 ,    // 161
			 51,   0, 153, 4 ,    // 162
			 102,   0, 204, 4 ,    // 163
			 153,   0, 204, 4 ,    // 164
			 204,   0, 204, 4 ,    // 165
			 0,  51, 153, 4 ,    // 166
			 51,  51, 204, 4 ,    // 167
			 102,  51, 204, 4 ,    // 168
			 153,  51, 204, 4 ,    // 169
			 204,  51, 204, 4 ,    // 170
			 255,  51, 204, 4 ,    // 171
			 0, 102, 204, 4 ,    // 172
			 51, 102, 204, 4 ,    // 173
			 102, 102, 153, 4 ,    // 174
			 153, 102, 204, 4 ,    // 175
			 204, 102, 204, 4 ,    // 176
			 255, 102, 153, 4 ,    // 177
			 0, 153, 204, 4 ,    // 178
			 51, 153, 204, 4 ,    // 179
			 102, 153, 204, 4 ,    // 180
			 153, 153, 204, 4 ,    // 181
			 204, 153, 204, 4 ,    // 182
			 255, 153, 204, 4 ,    // 183
			 0, 204, 204, 4 ,    // 184
			 51, 204, 204, 4 ,    // 185
			 102, 204, 204, 4 ,    // 186
			 153, 204, 204, 4 ,    // 187
			 204, 204, 204, 4 ,    // 188
			 255, 204, 204, 4 ,    // 189
			 0, 255, 204, 4 ,    // 190
			 51, 255, 204, 4 ,    // 191
			 102, 255, 153, 4 ,    // 192
			 153, 255, 204, 4 ,    // 193
			 204, 255, 204, 4 ,    // 194
			 255, 255, 204, 4 ,    // 195
			 51,   0, 204, 4 ,    // 196
			 102,   0, 255, 4 ,    // 197
			 153,   0, 255, 4 ,    // 198
			 0,  51, 204, 4 ,    // 199
			 51,  51, 255, 4 ,    // 200
			 102,  51, 255, 4 ,    // 201
			 153,  51, 255, 4 ,    // 202
			 204,  51, 255, 4 ,    // 203
			 255,  51, 255, 4 ,    // 204
			 0, 102, 255, 4 ,    // 205
			 51, 102, 255, 4 ,    // 206
			 102, 102, 204, 4 ,    // 207
			 153, 102, 255, 4 ,    // 208
			 204, 102, 255, 4 ,    // 209
			 255, 102, 204, 4 ,    // 210
			 0, 153, 255, 4 ,    // 211
			 51, 153, 255, 4 ,    // 212
			 102, 153, 255, 4 ,    // 213
			 153, 153, 255, 4 ,    // 214
			 204, 153, 255, 4 ,    // 215
			 255, 153, 255, 4 ,    // 216
			 0, 204, 255, 4 ,    // 217
			 51, 204, 255, 4 ,    // 218
			 102, 204, 255, 4 ,    // 219
			 153, 204, 255, 4 ,    // 220
			 204, 204, 255, 4 ,    // 221
			 255, 204, 255, 4 ,    // 222
			 51, 255, 255, 4 ,    // 223
			 102, 255, 204, 4 ,    // 224
			 153, 255, 255, 4 ,    // 225
			 204, 255, 255, 4 ,    // 226
			 255, 102, 102, 4 ,    // 227
			 102, 255, 102, 4 ,    // 228
			 255, 255, 102, 4 ,    // 229
			 102, 102, 255, 4 ,    // 230
			 255, 102, 255, 4 ,    // 231
			 102, 255, 255, 4 ,    // 232
			 165,   0,  33, 4 ,    // 233
			 95,  95,  95, 4 ,    // 234
			 119, 119, 119, 4 ,    // 235
			 134, 134, 134, 4 ,    // 236
			 150, 150, 150, 4 ,    // 237
			 203, 203, 203, 4 ,    // 238
			 178, 178, 178, 4 ,    // 239
			 215, 215, 215, 4 ,    // 240
			 221, 221, 221, 4 ,    // 241
			 227, 227, 227, 4 ,    // 242
			 234, 234, 234, 4 ,    // 243
			 241, 241, 241, 4 ,    // 244
			 248, 248, 248, 4 ,    // 245
			 255, 251, 240, 0 ,    // 246
			 160, 160, 164, 0 ,    // 247
			 128, 128, 128, 0 ,    // 248
			 255,   0,   0, 0 ,    // 249
			 0, 255,   0, 0 ,    // 250
			 255, 255,   0, 0 ,    // 251
			 0,   0, 255, 0 ,    // 252
			 255,   0, 255, 0 ,    // 253
			 0, 255, 255, 0 ,    // 254
			 255, 255, 255, 0      // 255 always white
		};



#if 0

			const uint8_t pal256[1024] = {0,0,0,0,0,0,128,0,0,128,0,0,0,128,128,0,128,0,0,0,128,0,128,0,128,128,0,0,192,192,192,0,
			192,220,192,0,240,202,166,0,212,240,255,0,177,226,255,0,142,212,255,0,107,198,255,0,
			72,184,255,0,37,170,255,0,0,170,255,0,0,146,220,0,0,122,185,0,0,98,150,0,0,74,115,0,0,
			50,80,0,212,227,255,0,177,199,255,0,142,171,255,0,107,143,255,0,72,115,255,0,37,87,255,0,0,
			85,255,0,0,73,220,0,0,61,185,0,0,49,150,0,0,37,115,0,0,25,80,0,212,212,255,0,177,177,255,0,
			142,142,255,0,107,107,255,0,72,72,255,0,37,37,255,0,0,0,254,0,0,0,220,0,0,0,185,0,0,0,150,0,
			0,0,115,0,0,0,80,0,227,212,255,0,199,177,255,0,171,142,255,0,143,107,255,0,115,72,255,0,
			87,37,255,0,85,0,255,0,73,0,220,0,61,0,185,0,49,0,150,0,37,0,115,0,25,0,80,0,240,212,255,0,
			226,177,255,0,212,142,255,0,198,107,255,0,184,72,255,0,170,37,255,0,170,0,255,0,146,0,220,0,
			122,0,185,0,98,0,150,0,74,0,115,0,50,0,80,0,255,212,255,0,255,177,255,0,255,142,255,0,255,107,255,0,
			255,72,255,0,255,37,255,0,254,0,254,0,220,0,220,0,185,0,185,0,150,0,150,0,115,0,115,0,80,0,80,0,
			255,212,240,0,255,177,226,0,255,142,212,0,255,107,198,0,255,72,184,0,255,37,170,0,255,0,170,0,
			220,0,146,0,185,0,122,0,150,0,98,0,115,0,74,0,80,0,50,0,255,212,227,0,255,177,199,0,255,142,171,0,
			255,107,143,0,255,72,115,0,255,37,87,0,255,0,85,0,220,0,73,0,185,0,61,0,150,0,49,0,115,0,37,0,
			80,0,25,0,255,212,212,0,255,177,177,0,255,142,142,0,255,107,107,0,255,72,72,0,255,37,37,0,254,0,
			0,0,220,0,0,0,185,0,0,0,150,0,0,0,115,0,0,0,80,0,0,0,255,227,212,0,255,199,177,0,255,171,142,0,
			255,143,107,0,255,115,72,0,255,87,37,0,255,85,0,0,220,73,0,0,185,61,0,0,150,49,0,0,115,37,0,
			0,80,25,0,0,255,240,212,0,255,226,177,0,255,212,142,0,255,198,107,0,255,184,72,0,255,170,37,0,
			255,170,0,0,220,146,0,0,185,122,0,0,150,98,0,0,115,74,0,0,80,50,0,0,255,255,212,0,255,255,177,0,
			255,255,142,0,255,255,107,0,255,255,72,0,255,255,37,0,254,254,0,0,220,220,0,0,185,185,0,0,150,150,0,
			0,115,115,0,0,80,80,0,0,240,255,212,0,226,255,177,0,212,255,142,0,198,255,107,0,184,255,72,0,
			170,255,37,0,170,255,0,0,146,220,0,0,122,185,0,0,98,150,0,0,74,115,0,0,50,80,0,0,227,255,212,0,
			199,255,177,0,171,255,142,0,143,255,107,0,115,255,72,0,87,255,37,0,85,255,0,0,73,220,0,0,61,185,0,
			0,49,150,0,0,37,115,0,0,25,80,0,0,212,255,212,0,177,255,177,0,142,255,142,0,107,255,107,0,72,255,72,0,
			37,255,37,0,0,254,0,0,0,220,0,0,0,185,0,0,0,150,0,0,0,115,0,0,0,80,0,0,212,255,227,0,177,255,199,0,
			142,255,171,0,107,255,143,0,72,255,115,0,37,255,87,0,0,255,85,0,0,220,73,0,0,185,61,0,0,150,49,0,0,
			115,37,0,0,80,25,0,212,255,240,0,177,255,226,0,142,255,212,0,107,255,198,0,72,255,184,0,37,255,170,0,
			0,255,170,0,0,220,146,0,0,185,122,0,0,150,98,0,0,115,74,0,0,80,50,0,212,255,255,0,177,255,255,0,
			142,255,255,0,107,255,255,0,72,255,255,0,37,255,255,0,0,254,254,0,0,220,220,0,0,185,185,0,0,
			150,150,0,0,115,115,0,0,80,80,0,242,242,242,0,230,230,230,0,218,218,218,0,206,206,206,0,194,194,194,0,
			182,182,182,0,170,170,170,0,158,158,158,0,146,146,146,0,134,134,134,0,122,122,122,0,110,110,110,0,
			98,98,98,0,86,86,86,0,74,74,74,0,62,62,62,0,50,50,50,0,38,38,38,0,26,26,26,0,14,14,14,0,240,251,255,0,
			164,160,160,0,128,128,128,0,0,0,255,0,0,255,0,0,0,255,255,0,255,0,0,0,255,0,255,0,255,255,0,0,255,255,255,0};
#endif
			memcpy(GetPalette(),pal256,1024);
			break;
		}
	case 4:
		{
			const uint8_t pal16[64]={0,0,0,0,0,0,128,0,0,128,0,0,0,128,128,0,128,0,0,0,128,0,128,0,128,128,0,0,192,192,192,0,
								128,128,128,0,0,0,255,0,0,255,0,0,0,255,255,0,255,0,0,0,255,0,255,0,255,255,0,0,255,255,255,0};
			memcpy(GetPalette(),pal16,64);
			break;
		}
	case 1:
		{
			const uint8_t pal2[8]={0,0,0,0,255,255,255,0};
			memcpy(GetPalette(),pal2,8);
			break;
		}
	}
	info.last_c_isvalid = false;
	return;
}
////////////////////////////////////////////////////////////////////////////////
