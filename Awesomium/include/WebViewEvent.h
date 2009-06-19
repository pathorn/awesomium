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

#ifndef __WEBVIEWEVENT_H__
#define __WEBVIEWEVENT_H__

#include "WebView.h"

class WebViewProxy;

class WebViewEvent
{
public:
	WebViewEvent(Awesomium::WebView* view);
	virtual ~WebViewEvent() {}
	virtual void run() = 0;
protected:
	Awesomium::WebView* view;
};

namespace WebViewEvents
{

class BeginLoad : public WebViewEvent
{
	std::string url;
	std::wstring frameName;
	int statusCode;
	std::wstring mimeType;
public:
	BeginLoad(Awesomium::WebView* view, const std::string& url, const std::wstring& frameName, int statusCode, const std::wstring& mimeType);
	void run();
};

class FinishLoad : public WebViewEvent
{
public:
	FinishLoad(Awesomium::WebView* view);
	void run();
};

class BeginNavigate : public WebViewEvent
{
	std::string url;
	std::wstring frameName;
public:
	BeginNavigate(Awesomium::WebView* view, const std::string& url, const std::wstring& frameName);
	void run();
};

class ReceiveTitle : public WebViewEvent
{
	std::wstring title;
	std::wstring frameName;
public:
	ReceiveTitle(Awesomium::WebView* view, const std::wstring& title, const std::wstring& frameName);
	void run();
};

class InvokeCallback : public WebViewEvent
{
	std::string name;
	Awesomium::JSArguments args;
public:
	InvokeCallback(Awesomium::WebView* view, const std::string& name, const Awesomium::JSArguments& args);
	void run();
};

class ChangeTooltip : public WebViewEvent
{
	std::wstring tooltip;
public:
	ChangeTooltip(Awesomium::WebView* view, const std::wstring& tooltip);
	void run();
};

#if defined(_WIN32)

class ChangeCursor : public WebViewEvent
{
	HCURSOR cursor;
public:
	ChangeCursor(Awesomium::WebView* view, const HCURSOR& cursor);
	void run();
};

#endif

class ChangeKeyboardFocus : public WebViewEvent
{
	bool isFocused;
public:
	ChangeKeyboardFocus(Awesomium::WebView* view, bool isFocused);
	void run();
};

class ChangeTargetURL : public WebViewEvent
{
	std::string url;
public:
	ChangeTargetURL(Awesomium::WebView* view, const std::string& url);
	void run();
};
}


#endif