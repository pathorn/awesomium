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

#include "WebCore.h"
#include "WebCoreProxy.h"
#include "WebViewEvent.h"
#include "base/lock.h"
#include "base/thread.h"
#include "base/at_exit.h"
#include "base/path_service.h"
#include "base/file_util.h"
#include "base/message_loop.h"

Awesomium::WebCore* Awesomium::WebCore::instance = 0;
static MessageLoop* messageLoop = 0;

namespace Awesomium {

WebCore::WebCore(LogLevel level, bool enablePlugins, PixelFormat pixelFormat ) : pluginsEnabled(enablePlugins), pixelFormat(pixelFormat)
{
	assert(!instance);
	instance = this;

	logOpen = level != LOG_NONE;

	atExitMgr = new base::AtExitManager();
	
	Impl::initCommandLine();

	if(logOpen)
	{
		FilePath log_filename;
		PathService::Get(base::DIR_EXE, &log_filename);
		log_filename = log_filename.Append(FILE_PATH_LITERAL("awesomium.log"));
		logging::InitLogging(log_filename.value().c_str(),
			level == LOG_VERBOSE ? logging::LOG_TO_BOTH_FILE_AND_SYSTEM_DEBUG_LOG : logging::LOG_ONLY_TO_FILE,
			logging::LOCK_LOG_FILE, logging::DELETE_OLD_LOG_FILE);
	}

	if(level == LOG_NORMAL)
		logging::SetMinLogLevel(logging::LOG_WARNING);
	else if(level == LOG_VERBOSE)
		logging::SetMinLogLevel(logging::LOG_INFO);
	else if(level == LOG_NONE)
		logging::SetMinLogLevel(logging::LOG_NUM_SEVERITIES);

	eventQueueLock = new Lock();
	baseDirLock = new Lock();
	customResponsePageLock = new Lock();
	
	messageLoop = new MessageLoop();

	LOG(INFO) << "Creating the core thread.";
	coreThread = new base::Thread("CoreThread");
#if defined(_WIN32)
	// An internal message loop type of UI seems to be required on
	// Windows for proper clipboard functionality
	coreThread->StartWithOptions(base::Thread::Options(MessageLoop::TYPE_UI, 0));
#else
	coreThread->Start();
#endif

	LOG(INFO) << "Creating the WebCore.";
	coreProxy = new WebCoreProxy(coreThread, pluginsEnabled);
	Impl::initWebCorePlatform();
	coreProxy->AddRef();
	coreProxy->startup();
}

WebCore::~WebCore()
{
	assert(instance);

	if(views.size())
	{
		std::vector<WebView*> viewsToDestroy(views);

		for(std::vector<WebView*>::iterator i = viewsToDestroy.begin(); i != viewsToDestroy.end();)
		{
			(*i)->destroy();
			i = viewsToDestroy.erase(i);
		}
	}
	
	messageLoop->RunAllPending();
	delete messageLoop;

	LOG(INFO) << "Releasing the WebCore soon.";
	coreThread->message_loop()->ReleaseSoon(FROM_HERE, coreProxy);
	LOG(INFO) << "Destroying the core thread.";
	delete coreThread;
	LOG(INFO) << "The core thread has been destroyed.";
	delete customResponsePageLock;
	delete eventQueueLock;
	delete baseDirLock;

	LOG(INFO) << "The WebCore has been shutdown.";

	if(logOpen)
		logging::CloseLogFile();

	delete atExitMgr;

	instance = 0;
}

WebCore& WebCore::Get()
{
	assert(instance);

	return *instance;
}

WebCore* WebCore::GetPointer()
{
	return instance;
}

void WebCore::setBaseDirectory(const std::string& baseDirectory)
{
	AutoLock autoBaseDirLock(*baseDirLock);

	this->baseDirectory = baseDirectory;

	if(this->baseDirectory.size())
		if(this->baseDirectory[this->baseDirectory.size() - 1] != '\\')
			this->baseDirectory += "\\";
}

Awesomium::WebView* WebCore::createWebView(int width, int height, bool isTransparent, bool enableAsyncRendering, int maxAsyncRenderPerSec)
{
	Awesomium::WebView* view = new Awesomium::WebView(width, height, isTransparent, enableAsyncRendering, maxAsyncRenderPerSec, coreThread);

	views.push_back(view);

	view->startup();

	return view;
}

void WebCore::setCustomResponsePage(int statusCode, const std::string& filePath)
{
	AutoLock autoCustomResponsePageLock(*customResponsePageLock);

	customResponsePageMap[statusCode] = filePath;
}

void WebCore::update()
{
	messageLoop->RunAllPending();
	
	AutoLock autoQueueLock(*eventQueueLock);

	while(!eventQueue.empty())
	{
		WebViewEvent* event = eventQueue.front();
		event->run();
		delete event;

		eventQueue.pop();
	}
}

const std::string& WebCore::getBaseDirectory() const
{
	AutoLock autoBaseDirLock(*baseDirLock);

	return baseDirectory;
}

PixelFormat WebCore::getPixelFormat() const
{
	return pixelFormat;
}

bool WebCore::arePluginsEnabled() const
{
	return pluginsEnabled;
}

void WebCore::pause()
{
	coreProxy->pause();
}

void WebCore::resume()
{
	coreProxy->resume();
}

void WebCore::queueEvent(WebViewEvent* event)
{
	AutoLock autoQueueLock(*eventQueueLock);

	eventQueue.push(event);
}

void WebCore::removeWebView(WebView* view)
{
	// Parse event queue in case one of the events references this view
	update();

	for(std::vector<WebView*>::iterator i = views.begin(); i != views.end(); i++)
	{
		if((*i) == view)
		{
			views.erase(i);
			break;
		}
	}
}

void WebCore::purgePluginMessages()
{
	if(pluginsEnabled)
		coreProxy->purgePluginMessages();
}

void WebCore::resolveJSValueFuture(WebView* view, int requestID, JSValue* result)
{
	for(std::vector<WebView*>::iterator i = views.begin(); i != views.end(); i++)
	{
		if((*i) == view)
		{
			view->resolveJSValueFuture(requestID, result);
			return;
		}
	}
}

void WebCore::getCustomResponsePage(int statusCode, std::string& filePathResult)
{
	AutoLock autoCustomResponsePageLock(*customResponsePageLock);

	std::map<int, std::string>::iterator i = customResponsePageMap.find(statusCode);

	if(i != customResponsePageMap.end())
		filePathResult = i->second;
	else
		filePathResult = "";
}

}

