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

#ifndef __WEBVIEWLISTENER_H__
#define __WEBVIEWLISTENER_H__

#include <string>
#include "JSValue.h"

#if defined(_WIN32)
#include <windows.h>
#endif

namespace Awesomium {

/**
* WebViewListener is a virtual interface that you can use to receive notifications
* from a certain WebView. Simply make a class that inherits from WebViewListener
* and register it via WebView::setListener.
*/
class _OSMExport WebViewListener
{
public:
	/**
	* This event is fired when a WebView begins navigating to a new URL.
	*
	* @param	url		The URL that is being navigated to.
	*
	* @param	frameName	The name of the frame that this event originated from.
	*/
	virtual void onBeginNavigation(const std::string& url, const std::wstring& frameName) = 0;

	/**
	* This event is fired when a WebView begins to actually receive data from a server.
	*
	* @param	url		The URL of the frame that is being loaded.
	*
	* @param	frameName	The name of the frame that this event originated from.
	*
	* @param	statusCode	The HTTP status code returned by the server.
	*
	* @param	mimeType	The mime-type of the content that is being loaded.
	*/
	virtual void onBeginLoading(const std::string& url, const std::wstring& frameName, int statusCode, const std::wstring& mimeType) = 0;

	/**
	* This event is fired when all loads have finished for a WebView.
	*/
	virtual void onFinishLoading() = 0;

	/**
	* This event is fired when a Client callback has been invoked via Javascript from a page.
	*
	* @param	name	The name of the client callback that was invoked (specifically, "Client._this_name_here_(...)").
	*
	* @param	args	The arguments passed to the callback.
	*/
	virtual void onCallback(const std::string& name, const Awesomium::JSArguments& args) = 0;
	
	/**
	* This event is fired when a page title is received.
	*
	* @param	title	The page title.
	*
	* @param	frameName	The name of the frame that this event originated from.
	*/
	virtual void onReceiveTitle(const std::wstring& title, const std::wstring& frameName) = 0;

	/**
	* This event is fired when a tooltip has changed state.
	*
	* @param	tooltip		The tooltip text (or, is an empty string when the tooltip should disappear).
	*/
	virtual void onChangeTooltip(const std::wstring& tooltip) = 0;

#if defined(_WIN32)
	/**
	* This event is fired when a cursor has changed state. [Windows-only]
	*
	* @param	cursor	The cursor handle/type.
	*/
	virtual void onChangeCursor(const HCURSOR& cursor) = 0;
#endif

	/**
	* This event is fired when keyboard focus has changed.
	*
	* @param	isFocused	Whether or not the keyboard is currently focused.
	*/
	virtual void onChangeKeyboardFocus(bool isFocused) = 0;

	/**
	* This event is fired when the target URL has changed. This is usually the result of 
	* hovering over a link on the page.
	*
	* @param	url	The updated target URL (or empty if the target URL is cleared).
	*/
	virtual void onChangeTargetURL(const std::string& url) = 0;
};

}

#endif