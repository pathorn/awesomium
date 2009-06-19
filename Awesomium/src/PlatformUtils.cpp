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

#include "PlatformUtils.h"

#ifndef SUPPRESS_LEAK_CHECKING
#define SUPPRESS_LEAK_CHECKING 1
#endif

#if SUPPRESS_LEAK_CHECKING
#include "wtf/RefCountedLeakCounter.h"
#endif

void suppressLeakChecking()
{
#if SUPPRESS_LEAK_CHECKING
	WTF::RefCountedLeakCounter::suppressMessages("...");
#endif
}

#include "base/command_line.h"

void Impl::initCommandLine()
{
	const char* argv[] = { "Awesomium" };
	CommandLine::Init(1, argv);
}

#if defined(__APPLE__)
#include "AtomicString.h"
#include "TextEncodingRegistry.h"
#include "WebSystemInterface.h"
#include "base/message_loop.h"

void Impl::initWebCorePlatform()
{
	//MessageLoopForUI main_message_loop;
	
	WebCore::AtomicString::init();
	WebCore::atomicCanonicalTextEncodingName("fake");
	InitWebCoreSystemInterface();
	suppressLeakChecking();
}
#elif defined(_WIN32)
#include "AtomicString.h"
#include "TextEncodingRegistry.h"
void Impl::initWebCorePlatform()
{
	WebCore::AtomicString::init();
	WebCore::atomicCanonicalTextEncodingName("fake");
	suppressLeakChecking();
}
#endif