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

#ifndef __WEBCORE_H__
#define __WEBCORE_H__

#include "WebView.h"
#include <queue>
#include <vector>
#include <string>

namespace base { class Thread; class AtExitManager; }
class WebCoreProxy;
class WebViewEvent;
class WindowlessPlugin;
class NamedCallback;
std::string GetDataResource(int id);
class Lock;

namespace Awesomium {

/**
* An enumeration of the three verbosity settings for the Awesomium Log.
*/
enum LogLevel
{
	LOG_NONE,		// No log is created
	LOG_NORMAL,		// Logs only errors
	LOG_VERBOSE		// Logs everything
};

/**
* An enumeration of the two output pixel formats that WebView::render will use.
*/
enum PixelFormat
{
	PF_BGRA,	// BGRA byte ordering [Blue, Green, Red, Alpha]
	PF_RGBA		// RGBA byte ordering [Red, Green, Blue, Alpha]
};

/**
* The WebCore singleton manages the creation of WebViews, the internal worker thread,
* and various other global states that are required to embed Chromium.
*/
class _OSMExport WebCore
{
public:
	/**
	* Instantiates the WebCore singleton (you can access it later with
	* WebCore::Get or GetPointer).
	*
	* @param	level	The logging level to use (default is LOG_NORMAL).
	*
	* @param	enablePlugins	Whether or not to enable embedded plugins.
	*
	* @param	pixelFormat		The pixel-format/byte-ordering to use when rendering WebViews.
	*/
	WebCore(LogLevel level = LOG_NORMAL, bool enablePlugins = true, PixelFormat pixelFormat = PF_BGRA);

	/**
	* Destroys the WebCore singleton. (Also destroys any lingering WebViews)
	*/
	~WebCore();

	/**
	* Retrieves the WebCore singleton.
	*
	* @note	This will assert if the singleton is not instantiated.
	*
	* @return	Returns a reference to the instance.
	*/
	static WebCore& Get();

	/**
	* Retrieves the WebCore singleton.
	*
	* @return	Returns a pointer to the instance.
	*/
    static WebCore* GetPointer();

	/**
	* Sets the base directory.
	*
	* @param	baseDirectory	The absolute path to your base directory. The base directory is a 
	*							location that holds all of your local assets. It will be used for 
	*							WebView::loadFile and WebView::loadHTML (to resolve relative URL's).
	*/
	void setBaseDirectory(const std::string& baseDirectory);

	/**
	* Creates a new WebView.
	*
	* @param	width	The width of the WebView in pixels.
	* @param	height	The height of the WebView in pixels.
	*
	* @param	isTransparent	Whether or not the background of a WebView should be rendered as transparent.
	*
	* @param	enableAsyncRendering	Enables fully-asynchronous rendering, see the note below.
	* @param	maxAsyncRenderPerSec	The maximum times per second this WebView should asynchronously render.
	*
	* @note		When asynchronous rendering is enabled, all rendering takes place on another thread asynchronously.
	*			The benefit of this behavior is that you may see a marked performance increase on machines with
	*			multi-core processors (especially when a WebView has high-animation content).
	*
	* @return	Returns a pointer to the created WebView.
	*/
	WebView* createWebView(int width, int height, bool isTransparent = false, bool enableAsyncRendering = false, int maxAsyncRenderPerSec = 70);

	/**
	* Sets a custom response page to use when a WebView encounters a certain
	* HTML status code from the server (such as '404 - File not found').
	*
	* @param	statusCode	The status code this response page should be associated with.
	*						See <http://en.wikipedia.org/wiki/List_of_HTTP_status_codes>
	*
	* @param	filePath	The local page to load as a response, should be a path relative to the base directory.
	*/
	void setCustomResponsePage(int statusCode, const std::string& filePath);

	/**
	* Updates the WebCore and allows it to conduct various operations such as the propagation
	* of bound JS callbacks and the invocation of any queued listener events.
	*/
	void update();

	/**
	* Retrieves the base directory.
	*
	* @return	Returns the current base directory.
	*/
	const std::string& getBaseDirectory() const;

	/**
	* Retrieves the pixel format being used.
	*/
	PixelFormat getPixelFormat() const;

	/**
	* Returns whether or not plugins are enabled.
	*/
	bool arePluginsEnabled() const;
	
	/**
	* Pauses the internal thread of the Awesomium WebCore.
	* 
	* @note	The pause and resume functions were added as
	*		a temporary workaround for threading issues with
	*		Flash plugins on the Mac OSX platform. You should
	*		call WebCore::pause right before handling the
	*		message loop in your main thread (usually via
	*		SDL_PollEvents) and immediately call resume after.
	*/
	void pause();
	
	/**
	* Resumes the internal thread of the Awesomium WebCore.
	*
	* @note	See WebCore::pause.
	*/
	void resume();

	friend class WebView;
	friend class FutureJSValue;
	friend class ::WebCoreProxy;
	friend class ::WebViewProxy;
	friend class ::NamedCallback;
	friend class ::WindowlessPlugin;
	friend std::string GetDataResource(int id);

protected:
	static WebCore* instance;
	base::Thread* coreThread;
	WebCoreProxy* coreProxy;
	base::AtExitManager* atExitMgr;
	std::vector<WebView*> views;
	std::queue<WebViewEvent*> eventQueue;
	std::map<int, std::string> customResponsePageMap;
	std::string baseDirectory;
	bool logOpen;
	bool pluginsEnabled;
	const PixelFormat pixelFormat;
	Lock *eventQueueLock, *baseDirLock, *customResponsePageLock;

	void queueEvent(WebViewEvent* event);
	void removeWebView(WebView* view);

	void purgePluginMessages();

	void resolveJSValueFuture(WebView* view, int requestID, JSValue* result);

	void getCustomResponsePage(int statusCode, std::string& filePathResult);
};

}

#endif