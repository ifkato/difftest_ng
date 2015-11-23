/*************************************************************************
** Copyright (c) 2011-2014 Accusoft Corporation                         **
**                                                                      **
** Written by Thomas Richter (richter@rus.uni-stuttgart.de)             **
** Sponsored by Accusoft Corporation, Tampa, FL and                     **
** the Computing Center of the University of Stuttgart                  **
**************************************************************************

This source file is part of difftest_ng, a universal image measuring
and conversion framework.

    difftest_ng is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    difftest_ng is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with difftest_ng.  If not, see <http://www.gnu.org/licenses/>.

*************************************************************************/

/*
**
** $Id: upsampler.cpp,v 1.3 2015/10/03 14:43:38 thor Exp $
**
** This class downscales in the spatial domain
*/

/// Includes
#include "diff/upsampler.hpp"
#include "std/string.hpp"
#include "std/math.hpp"
///

/// Upsampler::Filter
template<typename S>
void Upsampler::BilinearFilter(const S *org,ULONG obytesperpixel,ULONG obytesperrow,
			       S *dest,ULONG tbytesperpixel,ULONG tbytesperrow,
			       ULONG w,ULONG h,
			       S min,S max,
			       int sx,int sy)
{
  ULONG x,y;
  double fx = (sx & 1)?0.0:0.5;
  double fy = (sy & 1)?0.0:0.5;
  
  for(y = 0;y < h;y++) {
    S *dstrow = dest;
    for(x = 0;x < w;x++) {
      const S *srclt,*srcrt,*srclb,*srcrb;
      LONG xo = (x + sx - (sx >> 1)) / sx - 1;
      LONG yo = (y + sy - (sy >> 1)) / sy - 1; // left and top sample location.
      double wx = (x - double(xo * sx) - (sx >> 1) + fx) / double(sx);
      double wy = (y - double(yo * sy) - (sy >> 1) + fy) / double(sy); // weight for the right pixel
      LONG xl   = (xo >= 0)?(xo):(0);
      LONG xr   = (xo + 1 < LONG(w))?(xo + 1):(xo);
      LONG yt   = (yo >= 0)?(yo):(0);
      LONG yb   = (yo + 1 < LONG(h))?(yo + 1):(yo);
      assert(wx >= 0.0 && wx < 1.0);
      assert(wy >= 0.0 && wy < 1.0);
      srclt  = (const S *)(((const UBYTE *)org) + xl * obytesperpixel + yt * obytesperrow);
      srcrt  = (const S *)(((const UBYTE *)org) + xr * obytesperpixel + yt * obytesperrow);
      srclb  = (const S *)(((const UBYTE *)org) + xl * obytesperpixel + yb * obytesperrow);
      srcrb  = (const S *)(((const UBYTE *)org) + xr * obytesperpixel + yb * obytesperrow);
      //
      double v = (1.0-wx) * (1.0-wy) * *srclt + wx * (1.0-wy) * *srcrt + (1.0-wx) * wy * *srclb + wx * wy * *srcrb;
      
      if (v > max)
	v = max;
      if (v < min)
	v = min;
      *dstrow = S(v);
      dstrow  = (S *)(((UBYTE *)dstrow) + tbytesperpixel);
    }
    dest  = (S *)(((UBYTE *)dest) + tbytesperrow);
  }
}
///

/// Upsampler::~Upsampler
Upsampler::~Upsampler(void)
{
  ReleaseComponents(m_ppucSource);
  ReleaseComponents(m_ppucDestination);
}
///

/// Upsampler::ReleaseComponents
void Upsampler::ReleaseComponents(UBYTE **p)
{
  int i;

  if (p) {
    for(i = 0;i < m_usDepth;i++) {
      if (p[i])
        delete[] p[i];
    }
    delete[] p;
  }
}
///


/// Upsampler::Upsample
void Upsampler::Upsample(UBYTE **&data,class ImageLayout *src)
{
  UWORD i;
  // Delete the old image components. Does not release the
  // memory we hold.
  delete[] m_pComponent;
  m_pComponent  = NULL;
  ReleaseComponents(data);
  //
  if (m_bChromaOnly) {
    m_ulWidth     = src->WidthOf();
    m_ulHeight    = src->HeightOf();
  } else { 
    m_ulWidth     = src->WidthOf()  * m_ucScaleX;
    m_ulHeight    = src->HeightOf() * m_ucScaleY;
  }
  //
  m_usDepth     = src->DepthOf();
  m_pComponent  = new struct ComponentLayout[m_usDepth];
  //
  // Initialize component dimensions.
  for(i = 0; i < m_usDepth; i++) {
    if (m_bChromaOnly == false || i > 0) { 
      m_pComponent[i].m_ulWidth  = src->WidthOf(i)  * m_ucScaleX;
      m_pComponent[i].m_ulHeight = src->HeightOf(i) * m_ucScaleY;
    } else {
      m_pComponent[i].m_ulWidth  = src->WidthOf(i);
      m_pComponent[i].m_ulHeight = src->HeightOf(i);
    }
    m_pComponent[i].m_ucBits   = src->BitsOf(i);
    m_pComponent[i].m_bSigned  = src->isSigned(i);
    m_pComponent[i].m_bFloat   = src->isFloat(i);
    if (m_bChromaOnly && i > 0) { 
      m_pComponent[i].m_ucSubX   = 1;
      m_pComponent[i].m_ucSubY   = 1;
    } else {
      m_pComponent[i].m_ucSubX   = src->SubXOf(i);
      m_pComponent[i].m_ucSubY   = src->SubYOf(i);
    }
  }
  //
  data = new UBYTE *[m_usDepth];
  memset(data,0,sizeof(UBYTE *) * m_usDepth);
  //
  for(i = 0;i < m_usDepth;i++) {
    UBYTE bps = (m_pComponent[i].m_ucBits + 7) >> 3;
    if (m_pComponent[i].m_bFloat && m_pComponent[i].m_ucBits == 16)
      bps = sizeof(FLOAT); // stored as float
    //
    data[i]                           = new UBYTE[m_pComponent[i].m_ulWidth * m_pComponent[i].m_ulHeight * bps];
    m_pComponent[i].m_ulBytesPerPixel = bps;
    m_pComponent[i].m_ulBytesPerRow   = bps * m_pComponent[i].m_ulWidth;
    m_pComponent[i].m_pPtr            = data[i];
  }
  //
  for(i = 0;i < m_usDepth;i++) {
    UBYTE sx,sy;
    //
    if (m_bChromaOnly == false || i > 0) { 
      sx = m_ucScaleX;
      sy = m_ucScaleY;
    } else {
      sx = 1;
      sy = 1;
    }
    //
    if (isSigned(i)) {
      if (BitsOf(i) <= 8) {
	BilinearFilter<BYTE>((BYTE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			(BYTE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			WidthOf(i),HeightOf(i),
			BYTE(-1UL << (BitsOf(i) - 1)),BYTE((1UL << (BitsOf(i) - 1)) - 1),
			sx,sy);
      } else if (!isFloat(i) && BitsOf(i) <= 16) {
	BilinearFilter<WORD>((WORD *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			(WORD *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			WidthOf(i),HeightOf(i),
			WORD(-1UL << (BitsOf(i) - 1)),WORD((1UL << (BitsOf(i) - 1)) - 1),
			sx,sy);
      } else if (!isFloat(i) && BitsOf(i) <= 32) {
	BilinearFilter<LONG>((LONG *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			(LONG *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			WidthOf(i),HeightOf(i),
			LONG(-1UL << (BitsOf(i) - 1)),LONG((1UL << (BitsOf(i) - 1)) - 1),
			sx,sy);
      } else if (isFloat(i) && BitsOf(i) <= 32) {
	BilinearFilter<FLOAT>((FLOAT *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			(FLOAT *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			 WidthOf(i),HeightOf(i),
			 -HUGE_VAL,HUGE_VAL,
			 sx,sy);
      } else if (isFloat(i) && BitsOf(i) == 64) {
	BilinearFilter<DOUBLE>((DOUBLE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			  (DOUBLE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			  WidthOf(i),HeightOf(i),
			  -HUGE_VAL,HUGE_VAL,
			  sx,sy);
      } else {
	throw "unsupported data type";
      }
    } else {
      if (BitsOf(i) <= 8) {
	BilinearFilter<UBYTE>((UBYTE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			(UBYTE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			 WidthOf(i),HeightOf(i),
			 0,UBYTE((1UL << BitsOf(i)) - 1),
			 sx,sy);
      } else if (!isFloat(i) && BitsOf(i) <= 16) {
	BilinearFilter<UWORD>((UWORD *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			 (UWORD *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			 WidthOf(i),HeightOf(i),
			 0,UWORD((1UL << BitsOf(i)) - 1),
			 sx,sy);
      } else if (!isFloat(i) && BitsOf(i) <= 32) {
	BilinearFilter<ULONG>((ULONG *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			 (ULONG *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			 WidthOf(i),HeightOf(i),
			 0,LONG((1UL << BitsOf(i)) - 1),
			 sx,sy);
      } else if (isFloat(i) && BitsOf(i) <= 32) {
	BilinearFilter<FLOAT>((FLOAT *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			(FLOAT *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			 WidthOf(i),HeightOf(i),
			 0.0,HUGE_VAL,
			 sx,sy);
      } else if (isFloat(i) && BitsOf(i) == 64) {
	BilinearFilter<DOUBLE>((DOUBLE *)src->DataOf(i),src->BytesPerPixel(i),src->BytesPerRow(i),
			  (DOUBLE *)DataOf(i),BytesPerPixel(i),BytesPerRow(i),
			  WidthOf(i),HeightOf(i),
			  0.0,HUGE_VAL,
			  sx,sy);
      } else {
	throw "unsupported data type";
      }
    }
  }
  //
  Swap(*src);
}
///

/// Upsampler::Measure
double Upsampler::Measure(class ImageLayout *src,class ImageLayout *dest,double in)
{
  src->TestIfCompatible(dest);

  Upsample(m_ppucSource,src);
  Upsample(m_ppucDestination,dest);

  return in;
}
///

  