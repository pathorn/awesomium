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

#include "../include/WebView.h"
#include "WebViewProxy.h"
#include "WebCore.h"
#include "WebViewEvent.h"

#include "base/string_util.h"
#include "base/waitable_event.h"
#include "base/thread.h"
#include "base/lock.h"

class WebViewWaitState
{
public:
	base::WaitableEvent renderEvent, shutdownEvent, getContentTextEvent, jsValueFutureEvent, resizeEvent;

	WebViewWaitState() : renderEvent(false, false), shutdownEvent(false, false), getContentTextEvent(false, false),
		jsValueFutureEvent(false, false), resizeEvent(false, false)
	{
	}
};

class JSValueFutureImpl
{
public:
	Awesomium::JSValue* value;
	base::WaitableEvent resolveFutureEvent;

	JSValueFutureImpl() : value(0), resolveFutureEvent(false, false)
	{
	}

	~JSValueFutureImpl()
	{
		if(value)
			delete value;
	}
};

Awesomium::Rect::Rect() : x(0), y(0), width(0), height(0)
{
}

Awesomium::Rect::Rect(int x, int y, int width, int height) : x(x), y(y), width(width), height(height)
{
}

bool Awesomium::Rect::isEmpty() const
{
	return !x && !y && !width && !height;
}

Awesomium::WebView::WebView(int width, int height, bool isTransparent, bool enableAsyncRendering, int maxAsyncRenderPerSec, base::Thread* coreThread)
: coreThread(coreThread), listener(0), dirtiness(false), isKeyboardFocused(false), enableAsyncRendering(enableAsyncRendering)
{
	viewProxy = new WebViewProxy(width, height, isTransparent, enableAsyncRendering, maxAsyncRenderPerSec, this);
	viewProxy->AddRef();

	waitState = new WebViewWaitState();
	dirtinessLock = new LockImpl();
	jsValueFutureMapLock = new LockImpl();

	LOG(INFO) << "A new WebView has been created.";
}

Awesomium::WebView::~WebView()
{
	coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::asyncShutdown));
	waitState->shutdownEvent.Wait();
	viewProxy->Release();

	delete jsValueFutureMapLock;
	delete dirtinessLock;
	delete waitState;

	WebCore::Get().removeWebView(this);

	LOG(INFO) << "A WebView has been destroyed.";
}

void Awesomium::WebView::startup()
{
	coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::asyncStartup));
}

void Awesomium::WebView::destroy()
{
	delete this;
}

void Awesomium::WebView::setListener(Awesomium::WebViewListener* listener)
{
	this->listener = listener;
}

Awesomium::WebViewListener* Awesomium::WebView::getListener()
{
	return listener;
}

void Awesomium::WebView::loadURL(const std::string& url, const std::wstring& frameName, const std::string& username, const std::string& password)
{
	coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::loadURL, url, frameName, username, password));
}

void Awesomium::WebView::loadHTML(const std::string& html, const std::wstring& frameName)
{
	coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::loadHTML, html, frameName));
}

void Awesomium::WebView::loadFile(const std::string& file, const std::wstring& frameName)
{
	coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::loadFile, file, frameName));
}

void Awesomium::WebView::goToHistoryOffset(int offset)
{
	coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::goToHistoryOffset, offset));
}

void Awesomium::WebView::refresh()
{
	coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::refresh));
}

void Awesomium::WebView::executeJavascript(const std::string& javascript, const std::wstring& frameName)
{
	coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::executeJavascript, javascript, frameName));
}

Awesomium::FutureJSValue Awesomium::WebView::executeJavascriptWithResult(const std::string& javascript, const std::wstring& frameName)
{
	static int requestIDCounter = 0;

	FutureJSValue futureValue;
	futureValue.init(this, ++requestIDCounter);

	jsValueFutureMapLock->Lock();
	jsValueFutureMap[requestIDCounter] = new JSValueFutureImpl();
	jsValueFutureMapLock->Unlock();

	std::stringstream stream;
	stream << "Client.____futureValue(" << requestIDCounter << ", eval(\"";
	
	for(std::string::const_iterator i = javascript.begin(); i != javascript.end(); i++)
	{
		if((*i) == '"')
			stream << "\\\"";
		else if((*i) == '\\')
			stream << "\\\\";
		else
			stream << (*i);
	}

	stream << "\"))";

	executeJavascript(stream.str(), frameName);

	return futureValue;
}

void Awesomium::WebView::setProperty(const std::string& name, const JSValue& value)
{
	coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::setProperty, name, value));
}

void Awesomium::WebView::setCallback(const std::string& name)
{
	coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::setCallback, name));
}

bool Awesomium::WebView::isDirty()
{
	bool result = true;

	if(enableAsyncRendering)
	{
		dirtinessLock->Lock();
		result = dirtiness;
		dirtinessLock->Unlock();
	}
	else if(dirtinessLock->Try())
	{
		result = dirtiness;
		dirtinessLock->Unlock();
	}

	return result;
}

void Awesomium::WebView::render(unsigned char* destination, int destRowSpan, int destDepth, Awesomium::Rect* renderedRect)
{
	if(enableAsyncRendering)
	{
		viewProxy->copyRenderBuffer(destination, destRowSpan, destDepth);
	}
	else
	{
		coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::renderSync, destination, destRowSpan, destDepth, renderedRect));
		waitState->renderEvent.Wait();
	}
}

void Awesomium::WebView::injectMouseMove(int x, int y)
{
	coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::injectMouseMove, x, y));
}

void Awesomium::WebView::injectMouseDown(Awesomium::MouseButton button)
{
	coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::injectMouseDown, (short)button));
}

void Awesomium::WebView::injectMouseUp(Awesomium::MouseButton button)
{
	coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::injectMouseUp, (short)button));
}

void Awesomium::WebView::injectMouseWheelXY(int scrollAmountX, int scrollAmountY)
{
	coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::injectMouseWheel, scrollAmountX, scrollAmountY));
}

void Awesomium::WebView::injectKeyEvent(bool press, int modifiers, int windowsCode, int nativeCode)
{
	coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::injectKeyEvent, press, modifiers, windowsCode, nativeCode));
}

void Awesomium::WebView::injectTextEvent(std::wstring text)
{
	// Webkit uses utf-16...
	string16 output = WideToUTF16(text);
	coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::injectTextEvent, output));
}

#if defined(_WIN32)
void Awesomium::WebView::injectKeyboardEvent(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::injectKeyboardEvent, hwnd, message, wparam, lparam));
}

#elif defined(__APPLE__)
void Awesomium::WebView::injectKeyboardEvent(NSEvent* keyboardEvent)
{
	coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::injectKeyboardEvent, keyboardEvent));
}

#endif

void Awesomium::WebView::cut()
{
	coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::cut));
}

void Awesomium::WebView::copy()
{
	coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::copy));
}

void Awesomium::WebView::paste()
{
	coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::paste));
}

void Awesomium::WebView::selectAll()
{
	coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::selectAll));
}

void Awesomium::WebView::deselectAll()
{
	coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::deselectAll));
}

void Awesomium::WebView::getContentAsText(std::wstring& result, int maxChars)
{
	coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::getContentAsText, &result, maxChars));
	waitState->getContentTextEvent.Wait();
}

void Awesomium::WebView::zoomIn()
{
	coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::zoomIn));
}

void Awesomium::WebView::zoomOut()
{
	coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::zoomOut));
}

void Awesomium::WebView::resetZoom()
{
	coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::resetZoom));
}

void Awesomium::WebView::resize(int width, int height)
{
	coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::resize, width, height));
	waitState->resizeEvent.Wait();
}

void Awesomium::WebView::focus()
{
	executeJavascript("window.focus()");
}

void Awesomium::WebView::unfocus()
{
	executeJavascript("window.blur()");
}

void Awesomium::WebView::setTransparent(bool isTransparent)
{
	coreThread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(viewProxy, &WebViewProxy::setTransparent, isTransparent));
}

void Awesomium::WebView::setDirty(bool val)
{
	if(enableAsyncRendering)
		return;

	dirtinessLock->Lock();
	dirtiness = val;
	dirtinessLock->Unlock();
}

void Awesomium::WebView::setAsyncDirty(bool val)
{
	dirtinessLock->Lock();
	dirtiness = val;
	dirtinessLock->Unlock();
}

void Awesomium::WebView::setFinishRender()
{
	dirtiness = false;
	waitState->renderEvent.Signal();
}

void Awesomium::WebView::setFinishShutdown()
{
	waitState->shutdownEvent.Signal();
}

void Awesomium::WebView::setFinishGetContentText()
{
	waitState->getContentTextEvent.Signal();
}

void Awesomium::WebView::setFinishResize()
{
	waitState->resizeEvent.Signal();
}

void Awesomium::WebView::resolveJSValueFuture(int requestID, Awesomium::JSValue* result)
{
	std::map<int, JSValueFutureImpl*>::iterator i;

	jsValueFutureMapLock->Lock();

	i = jsValueFutureMap.find(requestID);

	if(i != jsValueFutureMap.end())
	{
		JSValueFutureImpl* futureImpl = (*i).second;

		if(!futureImpl->value)
		{
			jsValueFutureMapLock->Unlock();
			futureImpl->resolveFutureEvent.Wait();
			jsValueFutureMapLock->Lock();
		}

		if(!futureImpl->value)
		{
			// Try again with a timed wait
			jsValueFutureMapLock->Unlock();
			futureImpl->resolveFutureEvent.TimedWait(base::TimeDelta::FromMilliseconds(300));
			jsValueFutureMapLock->Lock();

			if(!futureImpl->value)
			{
				*result = Awesomium::JSValue();
				delete futureImpl;
				jsValueFutureMap.erase(i);
				jsValueFutureMapLock->Unlock();
				return;
			}
		}

		*result = *futureImpl->value;
		delete futureImpl;
		jsValueFutureMap.erase(i);
	}
	else
	{
		*result = Awesomium::JSValue();
	}

	jsValueFutureMapLock->Unlock();
}

void Awesomium::WebView::handleFutureJSValueCallback(const Awesomium::JSArguments& args)
{
	if(args.size() != 2)
		return;

	int requestID = args[0].toInteger();

	std::map<int, JSValueFutureImpl*>::iterator i;

	jsValueFutureMapLock->Lock();

	i = jsValueFutureMap.find(requestID);

	if(i == jsValueFutureMap.end())
		return;

	JSValueFutureImpl* futureImpl = (*i).second;

	if(!futureImpl->value)
	{
		futureImpl->value = new Awesomium::JSValue();
		*futureImpl->value = args[1];
	}

	jsValueFutureMapLock->Unlock();

	if(!futureImpl->resolveFutureEvent.IsSignaled())
		futureImpl->resolveFutureEvent.Signal();
}

void Awesomium::WebView::nullifyFutureJSValueCallbacks()
{
	jsValueFutureMapLock->Lock();

	for(std::map<int, JSValueFutureImpl*>::iterator i = jsValueFutureMap.begin(); i != jsValueFutureMap.end(); i++)
	{
		if(!i->second->value)
		{
			i->second->value = new Awesomium::JSValue();
			i->second->resolveFutureEvent.Signal();
		}
	}

	jsValueFutureMapLock->Unlock();
}

void Awesomium::WebView::handleCheckKeyboardFocus(bool isFocused)
{
	if(isKeyboardFocused != isFocused)
	{
		isKeyboardFocused = isFocused;
		Awesomium::WebCore::Get().queueEvent(new WebViewEvents::ChangeKeyboardFocus(this, isFocused));
	}
}
