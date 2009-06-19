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

#include "WebViewEvent.h"
#include "WebViewProxy.h"

WebViewEvent::WebViewEvent(Awesomium::WebView* view) : view(view)
{
}

using namespace WebViewEvents;

BeginLoad::BeginLoad(Awesomium::WebView* view, const std::string& url, const std::wstring& frameName, int statusCode, const std::wstring& mimeType) : WebViewEvent(view),
url(url), frameName(frameName), statusCode(statusCode), mimeType(mimeType)
{
}

void BeginLoad::run()
{
	Awesomium::WebViewListener* listener = view->getListener();

	if(listener)
		listener->onBeginLoading(url, frameName, statusCode, mimeType);
}

FinishLoad::FinishLoad(Awesomium::WebView* view) : WebViewEvent(view)
{
}

void FinishLoad::run()
{
	Awesomium::WebViewListener* listener = view->getListener();

	if(listener)
		listener->onFinishLoading();
}

BeginNavigate::BeginNavigate(Awesomium::WebView* view, const std::string& url, const std::wstring& frameName) : WebViewEvent(view), url(url), frameName(frameName)
{
}

void BeginNavigate::run()
{
	Awesomium::WebViewListener* listener = view->getListener();

	if(listener)
		listener->onBeginNavigation(url, frameName);
}

ReceiveTitle::ReceiveTitle(Awesomium::WebView* view, const std::wstring& title, const std::wstring& frameName) : WebViewEvent(view), title(title), frameName(frameName)
{
}

void ReceiveTitle::run()
{
	Awesomium::WebViewListener* listener = view->getListener();

	if(listener)
		listener->onReceiveTitle(title, frameName);
}

InvokeCallback::InvokeCallback(Awesomium::WebView* view, const std::string& name, const Awesomium::JSArguments& args) : WebViewEvent(view),
	name(name), args(args)
{
}

void InvokeCallback::run()
{
	Awesomium::WebViewListener* listener = view->getListener();

	if(listener)
		listener->onCallback(name, args);
}

ChangeTooltip::ChangeTooltip(Awesomium::WebView* view, const std::wstring& tooltip) : WebViewEvent(view), tooltip(tooltip)
{
}

void ChangeTooltip::run()
{
	Awesomium::WebViewListener* listener = view->getListener();

	if(listener)
		listener->onChangeTooltip(tooltip);
}

#if defined(_WIN32)

ChangeCursor::ChangeCursor(Awesomium::WebView* view, const HCURSOR& cursor) : WebViewEvent(view), cursor(cursor)
{
}

void ChangeCursor::run()
{
	Awesomium::WebViewListener* listener = view->getListener();

	if(listener)
		listener->onChangeCursor(cursor);
}

#endif

ChangeKeyboardFocus::ChangeKeyboardFocus(Awesomium::WebView* view, bool isFocused) : WebViewEvent(view), isFocused(isFocused)
{
}

void ChangeKeyboardFocus::run()
{
	Awesomium::WebViewListener* listener = view->getListener();

	if(listener)
		listener->onChangeKeyboardFocus(isFocused);
}

ChangeTargetURL::ChangeTargetURL(Awesomium::WebView* view, const std::string& url) : WebViewEvent(view), url(url)
{
}

void ChangeTargetURL::run()
{
	Awesomium::WebViewListener* listener = view->getListener();

	if(listener)
		listener->onChangeTargetURL(url);
}

