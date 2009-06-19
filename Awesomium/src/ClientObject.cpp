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

#include "ClientObject.h"
#include "WebCore.h"
#include "WebViewEvent.h"

void initFromCppArgumentList(const CppArgumentList& args, Awesomium::JSArguments& result);

class NamedCallback
{
	std::string name;
	Awesomium::WebView* view;
public:
	NamedCallback(const std::string& name, Awesomium::WebView* view);

	void handleCallback(const CppArgumentList& args, CppVariant* result);
};

class FutureValueCallback
{
	Awesomium::WebView* view;
public:
	FutureValueCallback(Awesomium::WebView* view) : view(view) {}

	void handleCallback(const CppArgumentList& args, CppVariant* result);
};

class CheckKeyboardFocusCallback
{
	Awesomium::WebView* view;
public:
	CheckKeyboardFocusCallback(Awesomium::WebView* view) : view(view) {}

	void handleCallback(const CppArgumentList& args, CppVariant* result);
};

ClientObject::ClientObject(Awesomium::WebView* view) : view(view), futureValueCallback(0), checkKeyboardFocusCallback(0)
{
}

ClientObject::~ClientObject()
{
	if(futureValueCallback)
		delete futureValueCallback;

	if(checkKeyboardFocusCallback)
		delete checkKeyboardFocusCallback;

	for(std::map<std::string, CppVariant*>::iterator i = clientProperties.begin(); i != clientProperties.end(); i++)
		delete i->second;

	for(std::map<std::string, NamedCallback*>::iterator i = clientCallbacks.begin(); i != clientCallbacks.end(); i++)
		delete i->second;
}

void ClientObject::initInternalCallbacks()
{
	if(!futureValueCallback)
	{
		futureValueCallback = new FutureValueCallback(view);
		CppBoundClass::Callback* callback = NewCallback<FutureValueCallback, const CppArgumentList&, 
			CppVariant*>(futureValueCallback, &FutureValueCallback::handleCallback);
		BindCallback("____futureValue", callback);
	}

	if(!checkKeyboardFocusCallback)
	{
		checkKeyboardFocusCallback = new CheckKeyboardFocusCallback(view);
		CppBoundClass::Callback* callback = NewCallback<CheckKeyboardFocusCallback, const CppArgumentList&, 
			CppVariant*>(checkKeyboardFocusCallback, &CheckKeyboardFocusCallback::handleCallback);
		BindCallback("____checkKeyboardFocus", callback);
	}
}

void ClientObject::setProperty(const std::string& name, const Awesomium::JSValue& value)
{
	std::map<std::string, CppVariant*>::iterator i = clientProperties.find(name);

	if(i == clientProperties.end())
	{
		CppVariant* newValue = new CppVariant();

		if(value.isString())
			newValue->Set(value.toString());
		else if(value.isInteger())
			newValue->Set(value.toInteger());
		else if(value.isDouble())
			newValue->Set(value.toDouble());
		else if(value.isBoolean())
			newValue->Set(value.toBoolean());
		else
			newValue->SetNull();
		
		clientProperties[name] = newValue;

		BindProperty(name, newValue);
	}
	else
	{
		if(value.isString())
			i->second->Set(value.toString());
		else if(value.isInteger())
			i->second->Set(value.toInteger());
		else if(value.isDouble())
			i->second->Set(value.toDouble());
		else if(value.isBoolean())
			i->second->Set(value.toBoolean());
		else
			i->second->SetNull();
	}
}

void ClientObject::setCallback(const std::string& name)
{
	if(clientCallbacks.find(name) == clientCallbacks.end())
	{
		NamedCallback* namedCallback = new NamedCallback(name, view);
		
		clientCallbacks[name] = namedCallback;

		CppBoundClass::Callback* callback = NewCallback<NamedCallback, const CppArgumentList&, 
			CppVariant*>(namedCallback, &NamedCallback::handleCallback);
		BindCallback(name, callback);
	}
}

NamedCallback::NamedCallback(const std::string& name, Awesomium::WebView* view) : name(name), view(view)
{
}

void NamedCallback::handleCallback(const CppArgumentList& args, CppVariant* result)
{
	Awesomium::JSArguments jsArgs;
	initFromCppArgumentList(args, jsArgs);

	Awesomium::WebCore::Get().queueEvent(new WebViewEvents::InvokeCallback(view, name, jsArgs));
	result->SetNull();
}

void FutureValueCallback::handleCallback(const CppArgumentList& args, CppVariant* result)
{
	Awesomium::JSArguments jsArgs;
	initFromCppArgumentList(args, jsArgs);

	view->handleFutureJSValueCallback(jsArgs);
	result->SetNull();
}

void CheckKeyboardFocusCallback::handleCallback(const CppArgumentList& args, CppVariant* result)
{
	Awesomium::JSArguments jsArgs;
	initFromCppArgumentList(args, jsArgs);

	if(jsArgs.size())
		view->handleCheckKeyboardFocus(jsArgs[0].toBoolean());

	result->SetNull();
}

void initFromCppArgumentList(const CppArgumentList& args, Awesomium::JSArguments& result)
{
	for(CppArgumentList::const_iterator i = args.begin(); i != args.end(); i++)
	{
		if(i->isInt32())
			result.push_back(i->ToInt32());
		else if(i->isDouble())
			result.push_back(i->ToDouble());
		else if(i->isBool())
			result.push_back(i->ToBoolean());
		else if(i->isString())
			result.push_back(i->ToString());
		else
			result.push_back(Awesomium::JSValue());
	}
}