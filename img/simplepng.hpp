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
 * An image class to load and save PNG images
 * from a file or to a file.
 * $Id: simplepng.hpp,v 1.6 2017/01/31 11:58:04 thor Exp $
 */

#ifndef SIMPLEPNG_HPP
#define SIMPLEPNG_HPP

/// Includes
#include "imglayout.hpp"
#include "std/stdio.hpp"
#include "config.h"
///

/// Forwards
struct ImgSpecs;
#ifdef USE_PNG
///

/// SimplePng
// This is the class for PNG images
class SimplePng : public ImageLayout {
  //
  // Pointer to the image bitmap, i.e. where the data is kept.
  APTR   m_pImage;
  //
  // The libPNG requires "row pointers" where each pointer points to
  // the start of a row. They are here...
  APTR  *m_ppRowPointers;
  //  
  // Palette of the image, if any.
  UQUAD *m_puqRed;
  UQUAD *m_puqGreen;
  UQUAD *m_puqBlue;
  //
  // A boolean indicator whether this is a palettized image (likely).
  // Note that gray-scale images are also palette images in the
  // sense of BMP.
  bool   m_bPalettized;
  //
  // Number of entries in the palette.
  ULONG  m_ulPaletteSize;
  //
public:
  //
  // Default constructor
  SimplePng(void);
  //
  // Copy the image from another source for later saving.
  SimplePng(const class ImageLayout &layout);
  // destructor
  ~SimplePng(void);
  //
  // Returns an indicator whether this is a palette image. This only
  // works right now for BMP, even though J2K supports this, too.
  virtual bool IsPalettized(void) const
  {
    return m_bPalettized;
  }
  //
  // Return the size of the palette table, in entries.
  virtual ULONG PaletteSize(void) const
  {
    return m_ulPaletteSize;
  }
  //
  // Return the pointers to the palette table of the n-th channel.
  virtual const UQUAD *PaletteTableOf(int channel) const
  {
    switch(channel) {
    case 0:
      return m_puqRed;
    case 1:
      return m_puqGreen;
    case 2:
      return m_puqBlue;
    }
    return NULL;
  }
  //
  // Save an image to a level 1 file descriptor, given its
  // width, height and depth. We only support grey level and
  // RGB here, no palette images.
  void SaveImage(const char *basename,const struct ImgSpecs &specs);
  //
  // Load an image from a level 1 file descriptor, keep it within
  // the internals of this class. The accessor methods below
  // should be used to find out more about this image.
  void LoadImage(const char *basename,struct ImgSpecs &specs);
};
///

///
#endif
#endif
///
