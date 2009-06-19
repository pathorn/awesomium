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

#include "PopupWidget.h"
#include "WebViewProxy.h"
#include "WebSize.h"
#include "WebRect.h"
#include "WebScreenInfo.h"

PopupWidget::PopupWidget(WebViewProxy* parent) : parent(parent), refCount(0), canvas(0)
{
	widget = WebWidget::Create(this);
}

PopupWidget::~PopupWidget()
{
	if(canvas)
		delete canvas;

	widget->Close();
}

WebWidget* PopupWidget::getWidget()
{
	return widget;
}

void PopupWidget::renderToWebView(Awesomium::RenderBuffer* context, bool isParentTransparent)
{
	if(windowRect.IsEmpty())
		return;

	if(!canvas)
		canvas = new skia::PlatformCanvas(windowRect.width(), windowRect.height(), false);

	if(!dirtyArea.IsEmpty())
	{
		widget->Layout();
		widget->Paint(canvas, WebKit::WebRect(dirtyArea));
		dirtyArea = gfx::Rect();
	}

	const SkBitmap& sourceBitmap = canvas->getTopPlatformDevice().accessBitmap(false);
	SkAutoLockPixels sourceBitmapLock(sourceBitmap);

	context->copyArea((unsigned char*)sourceBitmap.getPixels(), sourceBitmap.rowBytes(), windowRect, isParentTransparent);

}

void PopupWidget::DidScrollWebView(int dx, int dy)
{
	if(windowRect.IsEmpty())
		return;

	windowRect.Offset(dx, dy);
}

bool PopupWidget::isVisible()
{
	return !windowRect.IsEmpty();
}

void PopupWidget::translatePointRelative(int& x, int& y)
{
	if(!windowRect.IsEmpty())
	{
		x -= windowRect.x();
		y -= windowRect.y();
	}
}

gfx::NativeViewId PopupWidget::GetContainingView(WebWidget* webwidget)
{
	return NULL;
}

// Called when a region of the WebWidget needs to be re-painted.
void PopupWidget::DidInvalidateRect(WebWidget* webwidget, const WebKit::WebRect& rect)
{
	gfx::Rect clientRect(windowRect.width(), windowRect.height());
	dirtyArea = clientRect.Intersect(dirtyArea.Union(gfx::Rect(rect)));
	parent->invalidatePopups();
}

// Called when a region of the WebWidget, given by clip_rect, should be
// scrolled by the specified dx and dy amounts.
void PopupWidget::DidScrollRect(WebWidget* webwidget, int dx, int dy, const WebKit::WebRect& clip_rect)
{
	WebKit::WebSize size = widget->GetSize();
	dirtyArea = gfx::Rect(size.width, size.height);
	parent->invalidatePopups();
}

// This method is called to instruct the window containing the WebWidget to
// show itself as the topmost window.  This method is only used after a
// successful call to CreateWebWidget.  |disposition| indicates how this new
// window should be displayed, but generally only means something for WebViews.
void PopupWidget::Show(WebWidget* webwidget, WindowOpenDisposition disposition)
{
}

// This method is called to instruct the window containing the WebWidget to
// close.  Note: This method should just be the trigger that causes the
// WebWidget to eventually close.  It should not actually be destroyed until
// after this call returns.
void PopupWidget::CloseWidgetSoon(WebWidget* webwidget)
{
	MessageLoop::current()->PostTask(FROM_HERE, NewRunnableMethod(parent, &WebViewProxy::closePopup, this));
}

// This method is called to focus the window containing the WebWidget so
// that it receives keyboard events.
void PopupWidget::Focus(WebWidget* webwidget) { widget->SetFocus(true); }

// This method is called to unfocus the window containing the WebWidget so that
// it no longer receives keyboard events.
void PopupWidget::Blur(WebWidget* webwidget) { widget->SetFocus(false); }

void PopupWidget::SetCursor(WebWidget* webwidget, const WebCursor& cursor) {}

// Returns the rectangle of the WebWidget in screen coordinates.
void PopupWidget::GetWindowRect(WebWidget* webwidget, WebKit::WebRect* rect)
{
	*rect = WebKit::WebRect(windowRect);
}

// This method is called to re-position the WebWidget on the screen.  The given
// rect is in screen coordinates.  The implementation may choose to ignore
// this call or modify the given rect.  This method may be called before Show
// has been called.
// TODO(darin): this is more of a request; does this need to take effect
// synchronously?
void PopupWidget::SetWindowRect(WebWidget* webwidget, const WebKit::WebRect& rect)
{
	windowRect = gfx::Rect(rect);
	widget->Resize(WebKit::WebSize(rect.width, rect.height));
	dirtyArea = dirtyArea.Union(gfx::Rect(rect.width, rect.height));
}

// Returns the rectangle of the window in which this WebWidget is embeded in.
void PopupWidget::GetRootWindowRect(WebWidget* webwidget, WebKit::WebRect* rect)
{
	parent->GetWindowRect(0, rect);
}

void PopupWidget::GetRootWindowResizerRect(WebWidget* webwidget, WebKit::WebRect* rect)
{
}

// Keeps track of the necessary window move for a plugin window that resulted
// from a scroll operation.  That way, all plugin windows can be moved at the
// same time as each other and the page.
void PopupWidget::DidMove(WebWidget* webwidget, const WebPluginGeometry& move)
{
}

// Suppress input events to other windows, and do not return until the widget
// is closed.  This is used to support |window.showModalDialog|.
void PopupWidget::RunModal(WebWidget* webwidget) {}

// Owners depend on the delegates living as long as they do, so we ref them.
void PopupWidget::AddRef()
{
	refCount++;
}

void PopupWidget::Release()
{
	refCount--;

	if(refCount == 0)
		delete this;
}

WebKit::WebScreenInfo PopupWidget::GetScreenInfo(WebWidget* webwidget) {
	WebKit::WebScreenInfo ret;
	ret.availableRect = WebKit::WebRect(0,0,1024,768); // PRHFIXME: Huge hack, hardcode some screen size here.
	ret.depth = 24;
	ret.isMonochrome = false;
	ret.depthPerComponent = 8;
	ret.rect = WebKit::WebRect(0,0,1024,768);
	return ret;
}

  // Used for displaying HTML popup menus on Mac OS X (other platforms will use
  // Show() above). |bounds| represents the positioning on the screen (in WebKit
  // coordinates, origin at the top left corner) of the button that will display
  // the menu. It will be used, along with |item_height| (which refers to the
  // size of each entry in the menu), to position the menu on the screen.
  // |selected_index| indicates the menu item that should be drawn as selected
  // when the menu initially is displayed. |items| contains information about
  // each of the entries in the popup menu, such as the type (separator, option,
  // group), the text representation and the item's enabled status.
void PopupWidget::ShowAsPopupWithItems(WebWidget *webwidget,const WebKit::WebRect &,int,int,const std::vector<WebMenuItem> &) {
	Show(webwidget, NEW_POPUP); // PRHFIXME: for now.
}

bool PopupWidget::IsHidden(WebWidget *)
{
	return false;
}