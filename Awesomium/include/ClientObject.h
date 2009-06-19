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

#ifndef __CLIENTOBJECT_H__
#define __CLIENTOBJECT_H__

#include "webkit/glue/cpp_bound_class.h"
#include "JSValue.h"

namespace Awesomium { class WebView; }
class NamedCallback;
class FutureValueCallback;
class CheckKeyboardFocusCallback;

class ClientObject : public CppBoundClass
{
public:
	ClientObject(Awesomium::WebView* view);
	~ClientObject();

	void initInternalCallbacks();

	void setProperty(const std::string& name, const Awesomium::JSValue& value);

	void setCallback(const std::string& name);

protected:

	std::map<std::string, CppVariant*> clientProperties;
	std::map<std::string, NamedCallback*> clientCallbacks;
	FutureValueCallback* futureValueCallback;
	CheckKeyboardFocusCallback* checkKeyboardFocusCallback;
	Awesomium::WebView* view;
};

#endif