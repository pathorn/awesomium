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

#ifndef __WEBVIEWPROXY_H__
#define __WEBVIEWPROXY_H__

#include "RenderBuffer.h"
#include "PopupWidget.h"
#include "WebView.h"
#include "ClientObject.h"
#include <vector>
#include "webkit/glue/webview.h"
#include "webkit/glue/webframe.h"
#include "webkit/glue/weburlrequest.h"
#include "webkit/glue/webview_delegate.h"
#include "webkit/glue/webpreferences.h"
#include "webkit/glue/webcursor.h"
#include "base/gfx/platform_canvas.h"
#include "webkit/glue/webinputevent.h"
#include "SkCanvas.h"
#include "base/gfx/rect.h"
#include "base/message_loop.h"
#include "base/timer.h"
#include "base/lock.h"

class NavigationEntry;
class NavigationController;

class WebViewProxy : public WebViewDelegate
{
	int refCount;
	int width, height;
	gfx::Rect dirtyArea;
	Awesomium::RenderBuffer* renderBuffer;
	Awesomium::RenderBuffer* backBuffer;
	skia::PlatformCanvas* canvas;
	int mouseX, mouseY;
	struct { bool leftDown, middleDown, rightDown; } buttonState;	
	::WebView* view;
	Awesomium::WebView* parent;
	std::vector<PopupWidget*> popups;
	bool isPopupsDirty;
	bool needsPainting;
	ClientObject* clientObject;
	WebCursor curCursor;
	std::wstring curTooltip;
	LockImpl *renderBufferLock, *refCountLock;
	base::RepeatingTimer<WebViewProxy> renderTimer;
	bool isAsyncRenderDirty;
	int maxAsyncRenderPerSec;
	const bool enableAsyncRendering;
	bool isTransparent;
	GURL lastTargetURL;
	NavigationController* navController;
	int pageID, nextPageID;

	friend class NavigationController;

	void closeAllPopups();
	void handleMouseEvent(WebInputEvent::Type type, short buttonID);
	void overrideIFrameWindow(const std::wstring& frameName);
	bool navigate(NavigationEntry* entry, bool reload);
	void updateForCommittedLoad(WebFrame* frame, bool is_new_navigation);
	void updateURL(WebFrame* frame);
	void updateSessionHistory(WebFrame* frame);
public:

	WebViewProxy(int width, int height, bool isTransparent, bool enableAsyncRendering, int maxAsyncRenderPerSec, Awesomium::WebView* parent);

	~WebViewProxy();

	void asyncStartup();
	void asyncShutdown();

	void loadURL(const std::string& url, const std::wstring& frameName, const std::string& username, const std::string& password);
	void loadHTML(const std::string& html, const std::wstring& frameName);
	void loadFile(const std::string& file, const std::wstring& frameName);

	void goToHistoryOffset(int offset);

	void executeJavascript(const std::string& javascript, const std::wstring& frameName = std::wstring());
	void setProperty(const std::string& name, const Awesomium::JSValue& value);
	void setCallback(const std::string& name);

	void mayBeginRender();

	gfx::Rect render();

	void copyRenderBuffer(unsigned char* destination, int destRowSpan, int destDepth);

	void renderAsync();

	void renderSync(unsigned char* destination, int destRowSpan, int destDepth, Awesomium::Rect* renderedRect);

	void paint();

	void injectMouseMove(int x, int y);
	void injectMouseDown(short mouseButtonID);
	void injectMouseUp(short mouseButtonID);
	void injectMouseWheel(int scrollAmount);

#if defined(_WIN32)
	void injectKeyboardEvent(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
#elif defined(__APPLE__)
	void injectKeyboardEvent(NSEvent* keyboardEvent);
#endif

	void cut();
	void copy();
	void paste();

	void selectAll();
	void deselectAll();

	void getContentAsText(std::wstring* result, int maxChars);

	void zoomIn();
	void zoomOut();
	void resetZoom();

	void resize(int width, int height);

	void setTransparent(bool isTransparent);

	void invalidatePopups();

	void closePopup(PopupWidget* popup);

	void checkKeyboardFocus();

	void AddRef();
	void Release();

	/**
	* The following functions are inherited from WebViewDelegate
	*/

	::WebView* CreateWebView(::WebView* webview, bool user_gesture);

	WebWidget* CreatePopupWidget(::WebView* webview, bool focus_on_show);

	WebPluginDelegate* CreatePluginDelegate(::WebView* webview, const GURL& url, const std::string& mime_type,
		const std::string& clsid, std::string* actual_mime_type);

	void OnMissingPluginStatus(WebPluginDelegate* delegate, int status);

	void OpenURL(::WebView* webview, const GURL& url, WindowOpenDisposition disposition);

	void ReportFindInPageMatchCount(int count, int request_id, bool final_update);

	void ReportFindInPageSelection(int request_id, int active_match_ordinal, const gfx::Rect& selection_rect);

	const SkBitmap* GetPreloadedResourceBitmap(int resource_id);

	bool WasOpenedByUserGesture(::WebView* webview) const;

	void DidStartLoading(::WebView* webview);

	void DidStopLoading(::WebView* webview);

	void WindowObjectCleared(WebFrame* webframe);

	WindowOpenDisposition DispositionForNavigationAction(::WebView* webview, WebFrame* frame, 
		const WebRequest* request, WebNavigationType type, WindowOpenDisposition disposition, bool is_redirect);

	void DidStartProvisionalLoadForFrame(::WebView* webview, WebFrame* frame, NavigationGesture gesture);

	void DidReceiveProvisionalLoadServerRedirect(::WebView* webview, WebFrame* frame);

	void DidFailProvisionalLoadWithError(::WebView* webview, const WebError& error, WebFrame* frame);

	void LoadNavigationErrorPage(WebFrame* frame, const WebRequest* failed_request, const WebError& error,
		const std::string& html, bool replace);

	void DidCommitLoadForFrame(::WebView* webview, WebFrame* frame, bool is_new_navigation);

	void DidReceiveTitle(::WebView* webview, const std::wstring& title, WebFrame* frame);

	void DidFinishLoadForFrame(::WebView* webview, WebFrame* frame);

	void DidFailLoadWithError(::WebView* webview, const WebError& error, WebFrame* forFrame);

	void DidFinishDocumentLoadForFrame(::WebView* webview, WebFrame* frame);

	bool DidLoadResourceFromMemoryCache(::WebView* webview, const WebRequest& request, 
		const WebResponse& response, WebFrame* frame);

	void DidHandleOnloadEventsForFrame(::WebView* webview, WebFrame* frame);

	void DidChangeLocationWithinPageForFrame(::WebView* webview, WebFrame* frame, bool is_new_navigation);

	void DidReceiveIconForFrame(::WebView* webview, WebFrame* frame);

	void WillPerformClientRedirect(::WebView* webview, WebFrame* frame, const GURL& src_url, 
		const GURL& dest_url, unsigned int delay_seconds, unsigned int fire_date);

	void DidCancelClientRedirect(::WebView* webview, WebFrame* frame);

	void DidCompleteClientRedirect(::WebView* webview, WebFrame* frame, const GURL& source);

	void WillCloseFrame(::WebView* webview, WebFrame* frame);

	void AssignIdentifierToRequest(::WebView* webview, uint32 identifier, const WebRequest& request);

	void WillSendRequest(::WebView* webview, uint32 identifier, WebRequest* request);

	void DidFinishLoading(::WebView* webview, uint32 identifier);

	void DidFailLoadingWithError(::WebView* webview, uint32 identifier, const WebError& error);

	void AddMessageToConsole(::WebView* webview, const std::wstring& message, unsigned int line_no, 
		const std::wstring& source_id);

	void OnPasswordFormsSeen(::WebView* webview, const std::vector<PasswordForm>& forms);

	void OnUnloadListenerChanged(::WebView* webview, WebFrame* webframe);

	void ShowModalHTMLDialog(const GURL& url, int width, int height, const std::string& json_arguments, 
		std::string* json_retval);

	void RunJavaScriptAlert(::WebView* webview, const std::wstring& message);

	bool RunJavaScriptConfirm(::WebView* webview, const std::wstring& message);

	bool RunJavaScriptPrompt(::WebView* webview, const std::wstring& message, const std::wstring& default_value, 
		std::wstring* result);

	bool RunBeforeUnloadConfirm(::WebView* webview, const std::wstring& message);

	void UpdateTargetURL(::WebView* webview, const GURL& url);

	void RunFileChooser(const std::wstring& initial_filename, WebFileChooserCallback* file_chooser);

	void ShowContextMenu(::WebView* webview, ContextNode node, int x, int y, const GURL& link_url,
		const GURL& image_url, const GURL& page_url, const GURL& frame_url, const std::wstring& selection_text, 
		const std::wstring& misspelled_word, int edit_flags, const std::string& frame_encoding);

	void StartDragging(::WebView* webview, const WebDropData& drop_data);

	void TakeFocus(::WebView* webview, bool reverse);

	void JSOutOfMemory();

	bool ShouldBeginEditing(::WebView* webview, std::wstring range);

	bool ShouldEndEditing(::WebView* webview, std::wstring range);

	bool ShouldInsertNode(::WebView* webview, std::wstring node, std::wstring range, std::wstring action);

	bool ShouldInsertText(::WebView* webview, std::wstring text, std::wstring range, std::wstring action);

	bool ShouldChangeSelectedRange(::WebView* webview, std::wstring fromRange, std::wstring toRange, 
		std::wstring affinity, bool stillSelecting);

	bool ShouldDeleteRange(::WebView* webview, std::wstring range);

	bool ShouldApplyStyle(::WebView* webview, std::wstring style, std::wstring range);

	bool SmartInsertDeleteEnabled();
	void DidBeginEditing();
	void DidChangeSelection();
	void DidChangeContents();
	void DidEndEditing();

	void UserMetricsRecordAction(const std::wstring& action);
	void UserMetricsRecordComputedAction(const std::wstring& action);

	void DidDownloadImage(int id, const GURL& image_url, bool errored, const SkBitmap& image);

	enum ErrorPageType
	{
		DNS_ERROR,
		HTTP_404
	};

	GURL GetAlternateErrorPageURL(const GURL& failedURL, ErrorPageType error_type);

	WebHistoryItem* GetHistoryEntryAtOffset(int offset);

	void GoToEntryAtOffsetAsync(int offset);

	int GetHistoryBackListCount();
	int GetHistoryForwardListCount();

	void OnNavStateChanged(::WebView* webview);

	void SetTooltipText(::WebView* webview, const std::wstring& tooltip_text);

	void DownloadUrl(const GURL& url, const GURL& referrer);

	void SpellCheck(const std::wstring& word, int& misspell_location, int& misspell_length);

	void SetInputMethodState(bool enabled);

	void ScriptedPrint(WebFrame* frame);

	void WebInspectorOpened(int num_resources);

	void TransitionToCommittedForNewPage();

	gfx::NativeViewId GetContainingView(WebWidget* webwidget);

	void DidInvalidateRect(WebWidget* webwidget, const gfx::Rect& rect);

	void DidScrollRect(WebWidget* webwidget, int dx, int dy, const gfx::Rect& clip_rect);

	void Show(WebWidget* webwidget, WindowOpenDisposition disposition);

	void CloseWidgetSoon(WebWidget* webwidget);

	void Focus(WebWidget* webwidget);

	void Blur(WebWidget* webwidget);

	void SetCursor(WebWidget* webwidget, const WebCursor& cursor);

	void GetWindowRect(WebWidget* webwidget, gfx::Rect* rect);

	void SetWindowRect(WebWidget* webwidget, const gfx::Rect& rect);

	void GetRootWindowRect(WebWidget* webwidget, gfx::Rect* rect);

	void GetRootWindowResizerRect(WebWidget* webwidget, gfx::Rect* rect);

	void DidMove(WebWidget* webwidget, const WebPluginGeometry& move);

	void RunModal(WebWidget* webwidget);

	bool IsHidden();

};

#endif