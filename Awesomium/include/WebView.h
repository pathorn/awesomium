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

#ifndef __WEBVIEW_H__
#define __WEBVIEW_H__

#include "WebViewListener.h"
#include <map>

#if defined(_WIN32)
#include <windows.h>
#pragma warning( disable: 4251 )
#elif defined(__APPLE__)
class NSEvent;
#endif

class WebViewWaitState;
class WebViewProxy;
class JSValueFutureImpl;
class FutureValueCallback;
class CheckKeyboardFocusCallback;
namespace base { class Thread; }
class LockImpl;
namespace WebViewEvents { class InvokeCallback; }

namespace Awesomium {

class WebCore;

/**
* Mouse button enumerations, used with WebView::injectMouseDown 
* and WebView::injectMouseUp
*/
enum MouseButton {
	LEFT_MOUSE_BTN,
	MIDDLE_MOUSE_BTN,
	RIGHT_MOUSE_BTN
};

/**
* A simple rectangle class, used with WebView::render
*/
struct _OSMExport Rect {
	int x, y, width, height;

	Rect();
	Rect(int x, int y, int width, int height);
	bool isEmpty() const;
};

/**
* A WebView is essentially a single instance of a web-browser (created via the WebCore singleton)
* that you can interact with (via input injection, javascript, etc.) and render to an off-screen buffer.
*/
class _OSMExport WebView
{
public:

	/**
	* Explicitly destroys this WebView instance. If you neglect to call this, the WebCore singleton
	* will automatically destroy all lingering WebView instances at shutdown.
	*
	* @note	This should NEVER be called directly from one of the notifications of WebViewListener.
	*/
	void destroy();

	/**
	* Registers a WebViewListener to call upon various events (such as load completions, callbacks, title receptions,
	* cursor changes, etc).
	*
	* @param	listener	The WebViewListener to register. Or, you can pass '0' to undo any current registrations.
	*/
	void setListener(WebViewListener* listener);

	/**
	* Retrieves the current WebViewListener.
	*
	* @return	If a WebViewListener is registered, returns a pointer to the instance, otherwise returns 0.
	*/
	WebViewListener* getListener();

	/**
	* Loads a URL into the WebView asynchronously.
	*
	* @param	url	The URL to load.
	*
	* @param	frameName	Optional, the name of the frame to load the URL in; leave this blank to load in the main frame.
	*
	* @param	username	Optional, if the URL requires authentication, the username to authorize as.
	*
	* @param	password	Optional, if the URL requires authentication, the password to use.
	*/
	void loadURL(const std::string& url, const std::wstring& frameName = L"", const std::string& username = "", const std::string& password = "");

	/**
	* Loads a string of HTML into the WebView asynchronously.
	*
	* @param	html	The HTML string to load.
	*
	* @param	frameName	Optional, the name of the frame to load the HTML in; leave this blank to load in the main frame.
	*
	* @note	The base directory (specified via WebCore::setBaseDirectory) will be used to resolve
	*		relative URL's/resources (such as images, links, etc).
	*/
	void loadHTML(const std::string& html, const std::wstring& frameName = L"");

	/**
	* Loads a local file into the WebView asynchronously.
	*
	* @param	file	The file to load.
	*
	* @param	frameName	Optional, the name of the frame to load the file in; leave this blank to load in the main frame.
	*
	* @note	The file should exist within the base directory (specified via WebCore::setBaseDirectory).
	*/
	void loadFile(const std::string& file, const std::wstring& frameName = L"");

	/**
	* Navigates back/forward in history via a relative offset.
	*
	* @note
	*	For example, to go back one page:
	*		myView->goToHistoryOffset(-1);
	*	Or, to go forward one page:
	*		myView->goToHistoryOffset(1);
	*
	* @param	offset	The relative offset in history to navigate to.
	*/
	void goToHistoryOffset(int offset);

	/**
	* Executes a string of Javascript in the context of the current page asynchronously.
	*
	* @param	javascript	The string of Javascript to execute.
	*
	* @param	frameName	Optional, the name of the frame to execute in; leave this blank to execute in the main frame.
	*/
	void executeJavascript(const std::string& javascript, const std::wstring& frameName = L"");

	/**
	* Executes a string of Javascript in the context of the current page asynchronously with a result.
	*
	* @param	javascript	The string of Javascript to execute.
	*
	* @param	frameName	Optional, the name of the frame to execute in; leave this blank to execute in the main frame.
	*
	* @return	Returns a 'FutureJSValue' which is basically an 'IOU' for the future JSValue result.
	*			You can obtain the actual result via FutureJSValue::get later.
	*/
	Awesomium::FutureJSValue executeJavascriptWithResult(const std::string& javascript, const std::wstring& frameName = L"");

	/**
	* Sets a global 'Client' property that can be accessed via Javascript from
	* within all pages loaded into this web-view.
	*
	* @param	name	The name of the property.
	* @param	value	The javascript-value of the property.
	*
	* @note	You can access all properties you set via the 'Client' object using Javascript. For example,
	*		if you set the property with a name of 'color' and a value of 'blue', you could access
	*		this from the page using Javascript: document.write("The color is " + Client.color);
	*/
	void setProperty(const std::string& name, const JSValue& value);

	/**
	* Registers a global 'Client' callback that can be invoked via Javascript from
	* within all pages loaded into this view. You will need to register a WebViewListener
	* (WebView::setListener) to receieve notification of callbacks (WebViewListener::onCallback).
	*
	* @param	name	The name of the callback. You can invoke the callback in Javascript
	*					as: Client.your_name_here(arg1, arg2, ...);
	*
	* @note	In the future, it won't be necessary to register the callback name in advance.
	*/
	void setCallback(const std::string& name);

	/**
	* Returns whether or not the current web-view is dirty and needs to be re-rendered.
	*
	* @return	If the web-view is dirty, returns true, otherwise returns false.
	*/
	bool isDirty();
	
	/**
	* Renders the WebView to an off-screen buffer.
	*
	* @param	destination	The buffer to render to, its width and height should match the WebView's.
	*
	* @param	destRowSpan	The row-span of the destination buffer (number of bytes per row).
	*
	* @param	destDepth	The depth (bytes per pixel) of the destination buffer. Valid options
	*						include 3 (BGR/RGB) or 4 (BGRA/RGBA).
	*
	* @param	renderedRect	Optional (pass 0 to ignore); if asynchronous rendering is not enabled,
	*							you can provide a pointer to a Rect to store the dimensions of the 
	*							rendered area, or rather, the dimensions of the area that actually
	*							changed since the last render.
	*/
	void render(unsigned char* destination, int destRowSpan, int destDepth, Awesomium::Rect* renderedRect = 0);

	/**
	* Injects a mouse-move event in local coordinates.
	*
	* @param	x	The absolute x-coordinate of the mouse (localized to the WebView).
	* @param	y	The absolute y-coordinate of the mouse (localized to the WebView).
	*/
	void injectMouseMove(int x, int y);

	/**
	* Injects a mouse-down event.
	*
	* @param	button	The button that was pressed.
	*/
	void injectMouseDown(Awesomium::MouseButton button);

	/**
	* Injects a mouse-up event.
	*
	* @param	button	The button that was released.
	*/
	void injectMouseUp(Awesomium::MouseButton button);

	/**
	* Injects a mouse-wheel event.
	*
	* @param	scrollAmount	The amount of pixels to scroll by.
	*/
	void injectMouseWheel(int scrollAmount);

#if defined(_WIN32)
	/**
	* Injects a keyboard event. [Windows]
	*
	* @note	The native Windows keyboard message should be passed, valid message types include:
	*		- WM_KEYDOWN
	*		- WM_KEYUP
	*		- WM_SYSKEYDOWN
	*		- WM_SYSKEYUP
	*		- WM_CHAR
	*		- WM_IMECHAR
	*		- WM_SYSCHAR
	*/
	void injectKeyboardEvent(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

#elif defined(__APPLE__)
	/**
	* Injects a keyboard event. [Mac OSX]
	*
	* @note	The native OSX keyboard event should be passed.
	*/
	void injectKeyboardEvent(NSEvent* keyboardEvent);
#endif

	/**
	* Invokes a 'cut' action using the system clipboard.
	*/
	void cut();

	/**
	* Invokes a 'copy' action using the system clipboard.
	*/
	void copy();

	/**
	* Invokes a 'paste' action using the system clipboard.
	*/
	void paste();

	/**
	* Selects all items on the current page.
	*/
	void selectAll();

	/**
	* De-selects all items on the current page.
	*/
	void deselectAll();

	/**
	* Retrieves the content of the current page as plain text.
	*
	* @param	result	The wide string to store the retrieved text in.
	*
	* @param	maxChars	The maximum number of characters to retrieve.
	*
	* @note	Warning: The performance of this function depends upon the number of retrieved characters
	*		and the complexity of the page.
	*/
	void getContentAsText(std::wstring& result, int maxChars);

	/**
	* Zooms into the page, enlarging by 20%.
	*/
	void zoomIn();

	/**
	* Zooms out of the page, reducing by 20%.
	*/
	void zoomOut();

	/**
	* Resets the zoom level.
	*/
	void resetZoom();

	/**
	* Resizes this WebView to certain dimensions.
	*
	* @param	width	The width to resize to.
	* @param	height	The height to resize to.
	*/
	void resize(int width, int height);

	/**
	* Notifies the current page that it has lost focus.
	*/
	void unfocus();

	/**
	* Notifies the current page that is has gained focus.
	*/
	void focus();

	/**
	* Sets whether or not pages should be rendered with a transparent background-color.
	*
	* @param	isTransparent	Whether or not to force the background-color as transparent.
	*/
	void setTransparent(bool isTransparent);

protected:
	WebView(int width, int height, bool isTransparent, bool enableAsyncRendering, int maxAsyncRenderPerSec, base::Thread* coreThread);
	~WebView();

	void startup();
	void setDirty(bool val = true);
	void setAsyncDirty(bool val = true);
	void setFinishRender();
	void setFinishShutdown();
	void setFinishGetContentText();
	void setFinishResize();

	void resolveJSValueFuture(int requestID, Awesomium::JSValue* result);
	void handleFutureJSValueCallback(const Awesomium::JSArguments& args);
	void nullifyFutureJSValueCallbacks();
	void handleCheckKeyboardFocus(bool isFocused);

	base::Thread* coreThread;
	WebViewProxy* viewProxy;
	WebViewWaitState* waitState;
	WebViewListener* listener;
	LockImpl* dirtinessLock;
	bool dirtiness, isKeyboardFocused;
	LockImpl* jsValueFutureMapLock;
	std::map<int, JSValueFutureImpl*> jsValueFutureMap;

	const bool enableAsyncRendering;

	friend class WebCore;
	friend class ::WebViewProxy;
	friend class ::FutureValueCallback;
	friend class ::CheckKeyboardFocusCallback;
};

}

#endif