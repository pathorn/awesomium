/*
	This file is a part of Awesomium, a library that makes it easy for 
	developers to embed web-content in their applications.

	Copyright (C) 2009 Adam J. Simmons

	Project Website:
	<http://princeofcode.com/awesomium.php>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
	02110-1301 USA
*/

#ifndef __RENDERBUFFER_H__
#define __RENDERBUFFER_H__

#include "base/gfx/rect.h"

namespace Awesomium {

void copyBuffers(int width, int height, unsigned char* src, int srcRowSpan, unsigned char* dest, int destRowSpan, int destDepth, bool convertToRGBA);

class RenderBuffer
{
public:
	unsigned char* buffer;
	int width, height, rowSpan;

	RenderBuffer(int width, int height);

	~RenderBuffer();

	void reserve(int width, int height);
	void copyFrom(unsigned char* srcBuffer, int srcRowSpan);
	void copyArea(unsigned char* srcBuffer, int srcRowSpan, const gfx::Rect& srcRect, bool forceOpaque = false);
	void copyTo(unsigned char* destBuffer, int destRowSpan, int destDepth, bool convertToRGBA);
};

}

#endif