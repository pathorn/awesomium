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

#ifndef __WINDOWLESS_PLUGIN_H__
#define __WINDOWLESS_PLUGIN_H__

#if defined(_WIN32) || defined(__APPLE__)

#include "webkit/glue/webplugin_delegate.h"
#include "webkit/glue/webplugin.h"
#include "webkit/glue/plugins/plugin_instance.h"
#include "webkit/glue/plugins/plugin_lib.h"
#include "webkit/glue/plugins/plugin_stream_url.h"
#include "webkit/glue/webinputevent.h"
#include "base/message_loop.h"
#include "base/timer.h"
#include "WebCore.h"

class WindowlessPlugin : public WebPluginDelegate
{
public:
	WebPlugin* plugin;
	scoped_refptr<NPAPI::PluginInstance> pluginInstance;
	std::string pluginURL;
	NPWindow window;
	bool needsSetWindow;
	gfx::NativeDrawingContext hdc;
#if defined(__APPLE__)
	NP_CGContext npCgContext;
	base::RepeatingTimer<WindowlessPlugin> pluginUpdateTimer;
#endif
	
	WindowlessPlugin(NPAPI::PluginInstance *pluginInstance) : hdc(0), needsSetWindow(false)
	{
		this->pluginInstance = pluginInstance;
		pluginInstance->set_use_mozilla_user_agent();
		window.type = NPWindowTypeDrawable;
#if defined(__APPLE__)
		npCgContext.window = 0;
#endif
	}

	~WindowlessPlugin()
	{
		destroyInstance();
#if defined(__APPLE__)
		if(npCgContext.window)
			DisposeWindow(npCgContext.window);
#endif
		
		LOG(INFO) << "A WindowlessPlugin has been destroyed";
	}

	void destroyInstance()
	{
		if(pluginInstance && pluginInstance->npp()->ndata)
		{
			pluginInstance->CloseStreams();
			pluginInstance->NPP_Destroy();
			pluginInstance->set_web_plugin(0);
			pluginInstance->AddRef();
			MessageLoop::current()->ReleaseSoon(FROM_HERE, pluginInstance.get());
			pluginInstance = 0;
		}
	}

	static WindowlessPlugin* Create(const FilePath& path, const std::string& mime_type)
	{
		scoped_refptr<NPAPI::PluginLib> plugin = NPAPI::PluginLib::CreatePluginLib(path);

		if(!plugin.get())
			return 0;

		NPError rv = plugin->NP_Initialize();
		if(rv != NPERR_NO_ERROR)
			return 0;

		scoped_refptr<NPAPI::PluginInstance> instance = plugin->CreateInstance(mime_type);

		LOG(INFO) << "A WindowlessPlugin has been created";

		return new WindowlessPlugin(instance.get());
	}

	// Initializes the plugin implementation with the given (UTF8) arguments.
	// Note that the lifetime of WebPlugin must be longer than this delegate.
	// If this function returns false the plugin isn't started and shouldn't be
	// called again.  If this method succeeds, then the WebPlugin is valid until
	// PluginDestroyed is called.
	// The load_manually parameter if true indicates that the plugin data would 
	// be passed from webkit. if false indicates that the plugin should download 
	// the data. This also controls whether the plugin is instantiated as a full 
	// page plugin (NP_FULL) or embedded (NP_EMBED)
	bool Initialize(const GURL& url, char** argn, char** argv, int argc, WebPlugin* plugin, bool load_manually)
	{
		this->plugin = plugin;
		pluginInstance->set_web_plugin(plugin);
		
		/**
		* We need to enforce that the Flash plugin is always initialized with a 'wmode' of 'opaque' or 'transparent'
		* because those are the two windowless modes Flash accepts. We do this by copying the passed parameters into
		* our own structure, modifying/appending the wmode, and passing the result to PluginInstance::Start.
		*/

		char** keys = new char*[argc+1];
		char** vals = new char*[argc+1];

		bool hasNoWMode = true;

		for(int i = 0;; i++)
		{
			if(i == argc)
			{
				if(hasNoWMode)
				{
					keys[i] = "wmode";
					vals[i] = "opaque";
				}
				else
				{
					keys[i] = vals[i] = "";
				}
				break;
			}
			else
			{
				keys[i] = argn[i];
				vals[i] = argv[i];
			}

			if(strcmp(keys[i], "wmode") == 0)
			{
				if(strcmp(vals[i], "opaque") != 0 && strcmp(vals[i], "transparent") != 0)
					vals[i] = "opaque";

				hasNoWMode = false;
			}
		}

		//for(int i = 0; i < argc+1; i++)
		//	printf("init %s : %s\n", keys[i], vals[i]);
		
		pluginInstance->set_windowless(true);
#if defined(_WIN32)
		pluginInstance->set_window_handle(0);
#endif
		plugin->SetWindow(0, 0);

		NPAPI::PluginInstance* oldInstance = NPAPI::PluginInstance::SetInitializingInstance(pluginInstance);
		bool startResult = pluginInstance->Start(url, keys, vals, argc+1, load_manually);

		NPAPI::PluginInstance::SetInitializingInstance(oldInstance);
		pluginURL = url.spec();
		
		delete[] keys;
		delete[] vals;

		if(!startResult)
			LOG(WARNING) << "Failed to start the plugin instance of a WindowlessPlugin, loaded with URL: " << url.spec();
		
#if defined(__APPLE__)
		if(startResult)
			pluginUpdateTimer.Start(base::TimeDelta::FromMilliseconds(5), this, &WindowlessPlugin::updatePlugin);
#endif

		return startResult;
	}

	// Called when the WebPlugin is being destroyed.  This is a signal to the
	// delegate that it should tear-down the plugin implementation and not call
	// methods on the WebPlugin again.
	void PluginDestroyed()
	{
		Awesomium::WebCore::Get().purgePluginMessages();
		delete this;
	}

	// Update the geometry of the plugin.  This is a request to move the plugin,
	// relative to its containing window, to the coords given by window_rect.
	// Its contents should be clipped to the coords given by clip_rect, which are
	// relative to the origin of the plugin window.
	void UpdateGeometry(const gfx::Rect& window_rect, const gfx::Rect& clip_rect)
	{
		window.x = window_rect.x();
		window.y = window_rect.y();
		window.width = window_rect.width();
		window.height = window_rect.height();
		window.clipRect.left = clip_rect.x();
		window.clipRect.top = clip_rect.y();
		window.clipRect.right = clip_rect.x() + clip_rect.width();
		window.clipRect.bottom = clip_rect.y() + clip_rect.height();

		needsSetWindow = true;

#if defined(_WIN32)
		WINDOWPOS win_pos = {0};
		win_pos.x = window_rect.x();
		win_pos.y = window_rect.y();
		win_pos.cx = window_rect.width();
		win_pos.cy = window_rect.height();

		NPEvent pos_changed_event;
		pos_changed_event.event = WM_WINDOWPOSCHANGED;
		pos_changed_event.wParam = 0;
		pos_changed_event.lParam = PtrToUlong(&win_pos);

		pluginInstance->NPP_HandleEvent(&pos_changed_event);
#elif defined(__APPLE__)
		if(npCgContext.window)
			return;
		
		Rect bounds;
		bounds.top = window.y;
		bounds.left = window.x;
		bounds.bottom = window.y + window.height;
		bounds.right = window.x + window.width;
		
		CreateNewWindow(kDocumentWindowClass, 0, &bounds, &npCgContext.window);
#endif
	}

	// Tells the plugin to paint the damaged rect.  The HDC is only used for
	// windowless plugins.
	void Paint(gfx::NativeDrawingContext hdc, const gfx::Rect& rect)
	{
#if defined(_WIN32)
		window.window = (void*)hdc;
#elif defined(__APPLE__)
		CGContextSaveGState(hdc);
		CGContextTranslateCTM(hdc, window.x, window.y);
		npCgContext.context = hdc;
		//npCgContext.window = 0;
		
		/*Rect bounds;
		bounds.top = window.y;
		bounds.left = window.x;
		bounds.bottom = window.y + window.height;
		bounds.right = window.x + window.width;*/
		
		//CreateNewWindow(kDocumentWindowClass, 0, &bounds, &cgCtxt.window);
		
		window.window = (void*)&npCgContext;
#endif

		if(needsSetWindow)
		{
			pluginInstance->NPP_SetWindow(&window);
			needsSetWindow = false;
		}
		
#if defined(_WIN32)
		RECT damage_rect_win;
		damage_rect_win.left   = rect.x();  // + window_rect_.x();
		damage_rect_win.top    = rect.y();  // + window_rect_.y();
		damage_rect_win.right  = damage_rect_win.left + rect.width();
		damage_rect_win.bottom = damage_rect_win.top + rect.height();

		NPEvent paint_event;
		paint_event.event = WM_PAINT;
		// NOTE: NPAPI is not 64bit safe.  It puts pointers into 32bit values.
		paint_event.wParam = PtrToUlong(hdc);
		paint_event.lParam = PtrToUlong(&damage_rect_win);
		pluginInstance->NPP_HandleEvent(&paint_event);
		
#elif defined(__APPLE__)
		
		EventRecord event;
		event.what = updateEvt;
		event.message = (long unsigned int)hdc;
		event.when = TickCount();
		event.where.h = 0;
		event.where.v = 0;
		event.modifiers = 0;
		
		//NPEvent paint_event;
		pluginInstance->NPP_HandleEvent(&event);
		
		CGContextRestoreGState(hdc);
#endif
	}

	// Tells the plugin to print itself.
	void Print(gfx::NativeDrawingContext hdc)
	{
	}

	// Informs the plugin that it now has focus.
	void SetFocus()
	{
#if defined(_WIN32)
		NPEvent focusEvent;
		focusEvent.event = WM_SETFOCUS;
		focusEvent.wParam = 0;
		focusEvent.lParam = 0;

		pluginInstance->NPP_HandleEvent(&focusEvent);
#endif
	}

	// For windowless plugins, gives them a user event like mouse/keyboard.
	// Returns whether the event was handled.
	bool HandleEvent(NPEvent* event, WebCursor* cursor)
	{
		return pluginInstance->NPP_HandleEvent(event) != 0;
	}

	// Gets the NPObject associated with the plugin for scripting.
	NPObject* GetPluginScriptableObject()
	{
		return pluginInstance->GetPluginScriptableObject();
	}

	// Receives notification about a resource load that the plugin initiated
	// for a frame.
	void DidFinishLoadWithReason(NPReason reason)
	{
		pluginInstance->DidFinishLoadWithReason(reason);
	}
  
	// Returns the process id of the process that is running the plugin.
	int GetProcessId()
	{
#if defined(_WIN32)
		return ::GetCurrentProcessId();
#else
		return getpid();
#endif
	}

	// Returns the window handle for this plugin if it's a windowed plugin,
	// or NULL otherwise.
	gfx::NativeView GetWindowHandle() { return 0; }

	void FlushGeometryUpdates() {}

	// The result of the script execution is returned via this function.
	void SendJavaScriptStream(const std::string& url, const std::wstring& result,  bool success, bool notify_needed, int notify_data)
	{
		pluginInstance->SendJavaScriptStream(url, result, success, notify_needed, notify_data);
	}

	// Receives notification about data being available. 
	void DidReceiveManualResponse(const std::string& url, const std::string& mime_type, const std::string& headers, uint32 expected_length, uint32 last_modified)
	{
		pluginInstance->DidReceiveManualResponse(url, mime_type, headers, expected_length, last_modified);
	}

	// Receives the data.
	void DidReceiveManualData(const char* buffer, int length)
	{
		pluginInstance->DidReceiveManualData(buffer, length);
	}

	// Indicates end of data load.
	void DidFinishManualLoading()
	{
		pluginInstance->DidFinishManualLoading();
	}

	// Indicates a failure in data receipt.
	void DidManualLoadFail()
	{
		pluginInstance->DidManualLoadFail();
	}

	// Only Available after Initialize is called.
	FilePath GetPluginPath()
	{
		return pluginInstance->plugin_lib()->plugin_info().path;
	}

	// Only Supported when the plugin is the default plugin.
	void InstallMissingPlugin() {}

	// Creates a WebPluginResourceClient instance and returns the same.
	WebPluginResourceClient* CreateResourceClient(int resource_id, const std::string &url, bool notify_needed, void *notify_data, void* existing_stream)
	{
		if(existing_stream)
		{
			NPAPI::PluginStream* plugin_stream = reinterpret_cast<NPAPI::PluginStream*>(existing_stream);
			plugin_stream->CancelRequest();

			return plugin_stream->AsResourceClient();
		}

		if(notify_needed)
			pluginInstance->SetURLLoadData(GURL(url.c_str()), notify_data);

		return pluginInstance->CreateStream(resource_id, url, "", notify_needed, notify_data);
	}

	// Notifies the delegate about a Get/Post URL request getting routed
	void URLRequestRouted(const std::string&url, bool notify_needed, void* notify_data)
	{
		if(notify_needed)
			pluginInstance->SetURLLoadData(GURL(url.c_str()), notify_data);
	}
	
	void updatePlugin()
	{
#if defined(__APPLE__)
		static EventRecord event;
		event.what = nullEvent;
		event.message = 0;
		event.when = TickCount();
		event.where.h = 0;
		event.where.v = 0;
		event.modifiers = 0;
		
		pluginInstance->NPP_HandleEvent(&event);
		
		event.when = TickCount();
		pluginInstance->NPP_HandleEvent(&event);
		
		event.when = TickCount();
		pluginInstance->NPP_HandleEvent(&event);
		
		event.when = TickCount();
		pluginInstance->NPP_HandleEvent(&event);
#endif
	}
};

#endif

#endif