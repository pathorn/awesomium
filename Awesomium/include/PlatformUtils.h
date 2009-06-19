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

#ifndef __PLATFORM_UTILS_H__
#define __PLATFORM_UTILS_H__

#if defined(__WIN32__) || defined(_WIN32)
#	if defined(OSM_NONCLIENT_BUILD)
#		define _OSMExport __declspec( dllexport )
#	else
#		define _OSMExport __declspec( dllimport )
#	endif
#elif defined(__APPLE__)
#	define _OSMExport __attribute__((visibility("default")))
#else
#	define _OSMExport
#endif

namespace Impl {
void initCommandLine();
void initWebCorePlatform();
}

#endif