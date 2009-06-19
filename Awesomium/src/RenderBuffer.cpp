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

#include "RenderBuffer.h"
#include <string.h>
#include <assert.h>
#include "base/gfx/rect.h"

using namespace Awesomium;

void Awesomium::copyBuffers(int width, int height, unsigned char* src, int srcRowSpan, unsigned char* dest, int destRowSpan, int destDepth, bool convertToRGBA)
{
	assert(destDepth == 3 || destDepth == 4);

	if(!convertToRGBA)
	{
		if(destDepth == 3)
		{
			for(int row = 0; row < height; row++)
				for(int col = 0; col < width; col++)
					memcpy(dest + row * destRowSpan + col * 3, src + row * srcRowSpan + col * 4, 3);
		}
		else if(destDepth == 4)
		{
			for(int row = 0; row < height; row++)
				memcpy(dest + row * destRowSpan, src + row * srcRowSpan, srcRowSpan);
		}
	}
	else
	{
		if(destDepth == 3)
		{
			int srcRowOffset, destRowOffset;
			for(int row = 0; row < height; row++)
			{
				srcRowOffset = row * srcRowSpan;
				destRowOffset = row * destRowSpan;

				for(int col = 0; col < width; col++)
				{
					dest[destRowOffset + col * 3] = src[srcRowOffset + col * 4 + 2];
					dest[destRowOffset + col * 3 + 1] = src[srcRowOffset + col * 4 + 1];
					dest[destRowOffset + col * 3 + 2] = src[srcRowOffset + col * 4];
				}
			}
		}
		else if(destDepth == 4)
		{
			int srcRowOffset, destRowOffset;
			for(int row = 0; row < height; row++)
			{
				srcRowOffset = row * srcRowSpan;
				destRowOffset = row * destRowSpan;

				for(int colOffset = 0; colOffset < srcRowSpan; colOffset += 4)
				{
					dest[destRowOffset + colOffset] = src[srcRowOffset + colOffset + 2];
					dest[destRowOffset + colOffset + 1] = src[srcRowOffset + colOffset + 1];
					dest[destRowOffset + colOffset + 2] = src[srcRowOffset + colOffset];
					dest[destRowOffset + colOffset + 3] = src[srcRowOffset + colOffset + 3];
				}
			}
		}
	}
}

RenderBuffer::RenderBuffer(int width, int height) : width(0), height(0), buffer(0), rowSpan(0)
{
	reserve(width, height);
}

RenderBuffer::~RenderBuffer()
{
	if(buffer)
		delete[] buffer;
}

void RenderBuffer::reserve(int width, int height)
{
	if(this->width != width || this->height != height)
	{
		this->width = width;
		this->height = height;

		rowSpan = width * 4;

		if(buffer)
			delete[] buffer;

		buffer = new unsigned char[width * height * 4];
		memset(buffer, 255, width * height * 4);
	}
}

void RenderBuffer::copyFrom(unsigned char* srcBuffer, int srcRowSpan)
{
	for(int row = 0; row < height; row++)
		memcpy(buffer + row * rowSpan, srcBuffer + row * srcRowSpan, rowSpan);
}

void RenderBuffer::copyArea(unsigned char* srcBuffer, int srcRowSpan, const gfx::Rect& srcRect, bool forceOpaque)
{
	if(gfx::Rect(width, height).Contains(srcRect))
	{
		for(int row = 0; row < srcRect.height(); row++)
			memcpy(buffer + (row + srcRect.y()) * rowSpan + (srcRect.x() * 4), srcBuffer + row * srcRowSpan, srcRect.width() * 4);

		if(forceOpaque)
			for(int row = 0; row < srcRect.height(); row++)
				for(int col = 0; col < srcRect.width(); col++)
					buffer[(row + srcRect.y()) * rowSpan + (col + srcRect.x()) * 4 + 3] = 255;
	}
	else
	{
		gfx::Rect intersect = gfx::Rect(width, height).Intersect(srcRect);
		if(intersect.IsEmpty())
			return;

		int srcOffsetX = intersect.x() - srcRect.x();
		int srcOffsetY = intersect.y() - srcRect.y();

		for(int row = 0; row < intersect.height(); row++)
			memcpy(buffer + (row + intersect.y()) * rowSpan + (intersect.x() * 4), srcBuffer + (row + srcOffsetY) * srcRowSpan + (srcOffsetX * 4), intersect.width() * 4);

		if(forceOpaque)
			for(int row = 0; row < intersect.height(); row++)
				for(int col = 0; col < intersect.width(); col++)
					buffer[(row + intersect.y()) * rowSpan + (col + intersect.x()) * 4 + 3] = 255;
	}
}

void RenderBuffer::copyTo(unsigned char* destBuffer, int destRowSpan, int destDepth, bool convertToRGBA)
{
	copyBuffers(width, height, buffer, width * 4, destBuffer, destRowSpan, destDepth, convertToRGBA);
}
