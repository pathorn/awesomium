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

#ifndef __POPUP_WIDGET_H__
#define __POPUP_WIDGET_H__

#include <string>
#include "base/basictypes.h"
#include "webkit/glue/webwidget.h"
#include "webkit/glue/webwidget_delegate.h"
#include "base/gfx/platform_canvas.h"
#include "WebInputEvent.h"
#include "base/gfx/rect.h"
#include "RenderBuffer.h"

class WebViewProxy;

class PopupWidget : public WebWidgetDelegate
{
	int refCount;
	WebWidget* widget;
	WebViewProxy* parent;
	skia::PlatformCanvas* canvas;
	gfx::Rect dirtyArea;
	gfx::Rect windowRect;
public:
	PopupWidget(WebViewProxy* parent);
	~PopupWidget();

	WebWidget* getWidget();

	void renderToWebView(Awesomium::RenderBuffer* context, bool isParentTransparent);

	void DidScrollWebView(int dx, int dy);

	bool isVisible();

	void translatePointRelative(int& x, int& y);

	/**
	* The following methods are inherited from WebWidgetDelegate
	*/

	gfx::NativeViewId GetContainingView(WebWidget* webwidget);

	// Called when a region of the WebWidget needs to be re-painted.
	void DidInvalidateRect(WebWidget* webwidget, const WebKit::WebRect& rect);

	// Called when a region of the WebWidget, given by clip_rect, should be
	// scrolled by the specified dx and dy amounts.
	void DidScrollRect(WebWidget* webwidget, int dx, int dy, const WebKit::WebRect& clip_rect);

	// This method is called to instruct the window containing the WebWidget to
	// show itself as the topmost window.  This method is only used after a
	// successful call to CreateWebWidget.  |disposition| indicates how this new
	// window should be displayed, but generally only means something for WebViews.
	void Show(WebWidget* webwidget, WindowOpenDisposition disposition);

	// This method is called to instruct the window containing the WebWidget to
	// close.  Note: This method should just be the trigger that causes the
	// WebWidget to eventually close.  It should not actually be destroyed until
	// after this call returns.
	void CloseWidgetSoon(WebWidget* webwidget);

	// This method is called to focus the window containing the WebWidget so
	// that it receives keyboard events.
	void Focus(WebWidget* webwidget);

	// This method is called to unfocus the window containing the WebWidget so that
	// it no longer receives keyboard events.
	void Blur(WebWidget* webwidget);

	void SetCursor(WebWidget* webwidget, const WebCursor& cursor);

	// Returns the rectangle of the WebWidget in screen coordinates.
	void GetWindowRect(WebWidget* webwidget, WebKit::WebRect* rect);

	// This method is called to re-position the WebWidget on the screen.  The given
	// rect is in screen coordinates.  The implementation may choose to ignore
	// this call or modify the given rect.  This method may be called before Show
	// has been called.
	// TODO(darin): this is more of a request; does this need to take effect
	// synchronously?
	void SetWindowRect(WebWidget* webwidget, const WebKit::WebRect& rect);

	// Returns the rectangle of the window in which this WebWidget is embeded in.
	void GetRootWindowRect(WebWidget* webwidget, WebKit::WebRect* rect);

	// Returns the resizer rectangle of the window this WebWidget is in. This
	// is used on Mac to determine if a scrollbar is over the in-window resize
	// area at the bottom right corner.
	void GetRootWindowResizerRect(WebWidget* webwidget, WebKit::WebRect* rect);

	// Keeps track of the necessary window move for a plugin window that resulted
	// from a scroll operation.  That way, all plugin windows can be moved at the
	// same time as each other and the page.
	void DidMove(WebWidget* webwidget, const WebPluginGeometry& move);

	// Suppress input events to other windows, and do not return until the widget
	// is closed.  This is used to support |window.showModalDialog|.
	void RunModal(WebWidget* webwidget);

	// Owners depend on the delegates living as long as they do, so we ref them.
	void AddRef();
	void Release();

	WebKit::WebScreenInfo GetScreenInfo(WebWidget* webwidget);
	bool IsHidden(WebWidget *);
	void ShowAsPopupWithItems(WebWidget *,const WebKit::WebRect &,int,int,const std::vector<WebMenuItem> &);
};

#endif
