/*************************************************************************
** Written by Thomas Richter (THOR Software) for Accusoft	        **
** All Rights Reserved							**
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
** $Id: flip.cpp,v 1.6 2017/01/31 11:58:04 thor Exp $
**
** This class flips the image in X or Y direction.
*/

/// Includes
#include "img/imglayout.hpp"
#include "diff/flip.hpp"
///

/// Flip::FlipX
// Templated implementations: Flip horizontally.
template<typename T>
void Flip::doFlipX(T *org,ULONG obytesperpixel,ULONG obytesperrow,
		   ULONG w,ULONG h) const
{
  ULONG x;
  ULONG y;

  for(y = 0;y < h;y++) {
    T *left  = org;
    T *right = (T *)((UBYTE *)(org) + obytesperpixel * w);
    for(x = 0;x < (w >> 1);x++) {
      T tmp;
      //
      right  = (T *)((UBYTE *)(right) - obytesperpixel);
      tmp    = *right;
      *right = *left;
      *left  = tmp;
      left   = (T *)((UBYTE *)(left)  + obytesperpixel);
    }
    org = (T *)((UBYTE *)(org) + obytesperrow);
  }
}
///

/// Flip::FlipY
// Flip vertically.
template<typename T>
void Flip::doFlipY(T *org,ULONG obytesperpixel,ULONG obytesperrow,
		   ULONG w,ULONG h) const
{ 
  ULONG x;
  ULONG y;
  T *top    = org;
  T *bottom = (T *)((UBYTE *)(org) + h * obytesperrow);
  
  for(y = 0;y < (h >> 1);y++) {
    bottom  = (T *)((UBYTE *)(bottom) - obytesperrow);
    T *p1   = top;
    T *p2   = bottom;
    for(x = 0;x < w;x++) {
	T tmp = *p1;
	*p1   = *p2;
	*p2   = tmp;
	//
	p1    = (T *)((UBYTE *)(p1) + obytesperpixel);
	p2    = (T *)((UBYTE *)(p2) + obytesperpixel);
    }
    top = (T *)((UBYTE *)(top) + obytesperrow);
  }
}
///

/// Flip::flip
// Perform the flip on a single image
void Flip::flip(class ImageLayout *img) const
{
  UWORD comp,d  = img->DepthOf();

  for(comp = 0;comp < d;comp++) {
    ULONG  w = img->WidthOf(comp);
    ULONG  h = img->HeightOf(comp);
    //
    if (img->BitsOf(comp) <= 8) {
      if (m_Type == FlipX) {
	Flip::doFlipX<UBYTE>((UBYTE *)(img->DataOf(comp)),img->BytesPerPixel(comp),img->BytesPerRow(comp),w,h);
      } else {
	Flip::doFlipY<UBYTE>((UBYTE *)(img->DataOf(comp)),img->BytesPerPixel(comp),img->BytesPerRow(comp),w,h);
      }
    } else if (!img->isFloat(comp) && img->BitsOf(comp) <= 16) { // 16 bit float is internally stored as 32 bit.
      if (m_Type == FlipX) {
	Flip::doFlipX<UWORD>((UWORD *)(img->DataOf(comp)),img->BytesPerPixel(comp),img->BytesPerRow(comp),w,h);
      } else {
	Flip::doFlipY<UWORD>((UWORD *)(img->DataOf(comp)),img->BytesPerPixel(comp),img->BytesPerRow(comp),w,h);
      }
    } else if (img->BitsOf(comp) <= 32) {
      if (m_Type == FlipX) {
	Flip::doFlipX<ULONG>((ULONG *)(img->DataOf(comp)),img->BytesPerPixel(comp),img->BytesPerRow(comp),w,h);
      } else {
	Flip::doFlipY<ULONG>((ULONG *)(img->DataOf(comp)),img->BytesPerPixel(comp),img->BytesPerRow(comp),w,h);
      }
    } else if (img->BitsOf(comp) <= 64) {
      if (m_Type == FlipX) {
	Flip::doFlipX<UQUAD>((UQUAD *)(img->DataOf(comp)),img->BytesPerPixel(comp),img->BytesPerRow(comp),w,h);
      } else {
	Flip::doFlipY<UQUAD>((UQUAD *)(img->DataOf(comp)),img->BytesPerPixel(comp),img->BytesPerRow(comp),w,h);
      }
    } else {
      throw "unsupported data type";
    }
  }
}
///

/// Flip::Measure
double Flip::Measure(class ImageLayout *src,class ImageLayout *dst,double in)
{
  flip(src);
  flip(dst);

  return in;
}
///
