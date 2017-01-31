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
** $Id: pre.hpp,v 1.7 2017/01/31 11:58:04 thor Exp $
**
** This class measures the logarithmic peak error between two images.
*/

#ifndef DIFF_PRE_HPP
#define DIFF_PRE_HPP

/// Includes
#include "diff/meter.hpp"
///

/// Forwards
class ImageLayout;
///

/// class PRE
class PRE : public Meter {
  //
  // PRE measurement type
  int m_Type;
  //
  // Templated implementations
  template<typename T>
  double PeakError(T *org,ULONG obytesperpixel,ULONG obytesperrow,
		   T *dst,ULONG dbytesperpixel,ULONG dbytesperrow,
		   ULONG w,ULONG h);
public:
  //
  // Several options: Mean PRE, minimum PRE
  enum Type {
    Mean,
    Min
  };
  //
  PRE(Type t)
    : m_Type(t)
  { }
  //
  virtual double Measure(class ImageLayout *src,class ImageLayout *dst,double in);
  //
  virtual const char *NameOf(void) const
  {
    return "PeakRelativeError";
  }
};
///

///
#endif
