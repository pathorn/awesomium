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
#include "WebPopupMenu.h"
#include "WebViewProxy.h"
#include "WebSize.h"
#include "WebRect.h"
#include "WebScreenInfo.h"
#include "skia/ext/platform_canvas.h"

inline WebKit::WebCanvas *SkiaCanvasToWebCanvas (skia::PlatformCanvas *canvas) {
#ifdef WEBKIT_USING_CG
    return canvas->getTopPlatformDevice().GetBitmapContext();
#else
    return canvas;
#endif
}

PopupWidget::PopupWidget(WebViewProxy* parent) : parent(parent), canvas(0)
{
	widget = WebKit::WebPopupMenu::create(this);
}

PopupWidget::~PopupWidget()
{
	if(canvas)
		delete canvas;

	widget->close();
}

WebWidget* PopupWidget::getWidget()
{
	return widget;
}

void PopupWidget::renderToWebView(Awesomium::RenderBuffer* context, bool isParentTransparent)
{
	if(mWindowRect.IsEmpty())
		return;

	if(!canvas)
		canvas = new skia::PlatformCanvas(mWindowRect.width(), mWindowRect.height(), false);

	if(!dirtyArea.IsEmpty())
	{
		widget->layout();
		widget->paint(SkiaCanvasToWebCanvas(canvas), WebKit::WebRect(dirtyArea));
		dirtyArea = gfx::Rect();
	}

	const SkBitmap& sourceBitmap = canvas->getTopPlatformDevice().accessBitmap(false);
	SkAutoLockPixels sourceBitmapLock(sourceBitmap);

	context->copyArea((unsigned char*)sourceBitmap.getPixels(), sourceBitmap.rowBytes(), mWindowRect, isParentTransparent);

}

void PopupWidget::didScrollWebView(int dx, int dy)
{
	if(mWindowRect.IsEmpty())
		return;

	mWindowRect.Offset(dx, dy);
}

bool PopupWidget::isVisible()
{
	return !mWindowRect.IsEmpty();
}

void PopupWidget::translatePointRelative(int& x, int& y)
{
	if(!mWindowRect.IsEmpty())
	{
		x -= mWindowRect.x();
		y -= mWindowRect.y();
	}
}

// Called when a region of the WebWidget needs to be re-painted.
void PopupWidget::didInvalidateRect(const WebKit::WebRect& rect)
{
	gfx::Rect clientRect(mWindowRect.width(), mWindowRect.height());
	dirtyArea = clientRect.Intersect(dirtyArea.Union(gfx::Rect(rect)));
	parent->invalidatePopups();
}

// Called when a region of the WebWidget, given by clip_rect, should be
// scrolled by the specified dx and dy amounts.
void PopupWidget::didScrollRect(int dx, int dy, const WebKit::WebRect& clip_rect)
{
	WebKit::WebSize size = widget->size();
	dirtyArea = gfx::Rect(size.width, size.height);
	parent->invalidatePopups();
}

// This method is called to instruct the window containing the WebWidget to
// show itself as the topmost window.  This method is only used after a
// successful call to CreateWebWidget.  |disposition| indicates how this new
// window should be displayed, but generally only means something for WebViews.
void PopupWidget::show(WebKit::WebNavigationPolicy policy)
{
}

// This method is called to instruct the window containing the WebWidget to
// close.  Note: This method should just be the trigger that causes the
// WebWidget to eventually close.  It should not actually be destroyed until
// after this call returns.
void PopupWidget::closeWidgetSoon()
{
	MessageLoop::current()->PostTask(FROM_HERE, NewRunnableMethod(parent, &WebViewProxy::closePopup, this));
}

// This method is called to focus the window containing the WebWidget so
// that it receives keyboard events.
void PopupWidget::didFocus() { widget->setFocus(true); }

// This method is called to unfocus the window containing the WebWidget so that
// it no longer receives keyboard events.
void PopupWidget::didBlur() { widget->setFocus(false); }

void PopupWidget::didChangeCursor(const WebKit::WebCursorInfo& cursor) {
}

// Returns the rectangle of the WebWidget in screen coordinates.
WebKit::WebRect PopupWidget::windowRect()
{
	return WebKit::WebRect(mWindowRect);
}

// This method is called to re-position the WebWidget on the screen.  The given
// rect is in screen coordinates.  The implementation may choose to ignore
// this call or modify the given rect.  This method may be called before Show
// has been called.
// TODO(darin): this is more of a request; does this need to take effect
// synchronously?
void PopupWidget::setWindowRect(const WebKit::WebRect& rect)
{
	mWindowRect = gfx::Rect(rect);
	widget->resize(WebKit::WebSize(rect.width, rect.height));
	dirtyArea = dirtyArea.Union(gfx::Rect(rect.width, rect.height));
}

// Returns the rectangle of the window in which this WebWidget is embeded in.
WebKit::WebRect PopupWidget::rootWindowRect()
{
	return parent->windowRect();
}

WebKit::WebRect PopupWidget::windowResizerRect()
{
	return windowRect();
}

// Suppress input events to other windows, and do not return until the widget
// is closed.  This is used to support |window.showModalDialog|.
void PopupWidget::runModal() {}

WebKit::WebScreenInfo PopupWidget::screenInfo() {
	WebKit::WebScreenInfo ret;
	ret.availableRect = WebKit::WebRect(0,0,1024,768); // PRHFIXME: Huge hack, hardcode some screen size here.
	ret.depth = 24;
	ret.isMonochrome = false;
	ret.depthPerComponent = 8;
	ret.rect = WebKit::WebRect(0,0,1024,768);
	return ret;
}
