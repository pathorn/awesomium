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
		widget->Paint(canvas, dirtyArea);
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
void PopupWidget::DidInvalidateRect(WebWidget* webwidget, const gfx::Rect& rect)
{
	gfx::Rect clientRect(windowRect.width(), windowRect.height());
	dirtyArea = clientRect.Intersect(dirtyArea.Union(rect));
	parent->invalidatePopups();
}

// Called when a region of the WebWidget, given by clip_rect, should be
// scrolled by the specified dx and dy amounts.
void PopupWidget::DidScrollRect(WebWidget* webwidget, int dx, int dy, const gfx::Rect& clip_rect)
{
	gfx::Size size = widget->GetSize();
	dirtyArea = gfx::Rect(size.width(), size.height());
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
void PopupWidget::GetWindowRect(WebWidget* webwidget, gfx::Rect* rect)
{
	*rect = windowRect;
}

// This method is called to re-position the WebWidget on the screen.  The given
// rect is in screen coordinates.  The implementation may choose to ignore
// this call or modify the given rect.  This method may be called before Show
// has been called.
// TODO(darin): this is more of a request; does this need to take effect
// synchronously?
void PopupWidget::SetWindowRect(WebWidget* webwidget, const gfx::Rect& rect)
{
	windowRect = rect;
	widget->Resize(gfx::Size(rect.width(), rect.height()));
	dirtyArea = dirtyArea.Union(gfx::Rect(rect.width(), rect.height()));
}

// Returns the rectangle of the window in which this WebWidget is embeded in.
void PopupWidget::GetRootWindowRect(WebWidget* webwidget, gfx::Rect* rect)
{
	parent->GetWindowRect(0, rect);
}

void PopupWidget::GetRootWindowResizerRect(WebWidget* webwidget, gfx::Rect* rect)
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

bool PopupWidget::IsHidden()
{
	return false;
}