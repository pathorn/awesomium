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

#include "base/logging.h"
#include "base/path_service.h"
#include "base/file_util.h"
#include "net/base/mime_util.h"
#include "webkit/glue/webkit_glue.h"
#include "webkit/glue/plugins/plugin_list.h"
#include "googleurl/src/gurl.h"
#include "WebCore.h"
#include "base/clipboard.h"
#include "base/lazy_instance.h"
#include "googleurl/src/gurl.h"
#include "webkit/glue/scoped_clipboard_writer_glue.h"
#include "SkBitmap.h"
#if defined(_WIN32)
//#include "webkit/glue/webkit_resources.h"
#include <atlbase.h>
#endif
#include "base/string16.h"
#include "base/string_util.h"
#include "webkit/glue/screen_info.h"

#if defined(_WIN32)
#pragma warning( disable: 4996 )
#endif

std::wstring stringToWide(const std::string &stringToConvert)
{
	size_t size = mbstowcs(0, stringToConvert.c_str(), 0) + 1;
	wchar_t *temp = new wchar_t[size];
	mbstowcs(temp, stringToConvert.c_str(), size);
	std::wstring result(temp);
	delete[] temp;
	return result;
}

namespace webkit_glue
{

bool HistoryContains(const char16* url, int url_len, const char* document_host, int document_host_len, 
	bool is_dns_prefetch_enabled)
{
	return false;
}

void PrefetchDns(const std::string& hostname) {}

void PrecacheUrl(const char16* url, int url_length) {}

void AppendToLog(const char* file, int line, const char* msg)
{
	logging::LogMessage(file, line).stream() << msg;
}
/*
bool GetMimeTypeFromExtension(const std::wstring &ext, std::string *mime_type)
{
	return net::GetMimeTypeFromExtension(ext, mime_type);
}

bool GetMimeTypeFromFile(const std::wstring &file_path, std::string *mime_type)
{
	return net::GetMimeTypeFromFile(file_path, mime_type);
}

bool GetPreferredExtensionForMimeType(const std::string& mime_type, std::wstring* ext)
{
	return net::GetPreferredExtensionForMimeType(mime_type, ext);
}
*/
string16 GetLocalizedString(int message_id)
{
	// TODO - Replace hard-coded strings with dynamic localization.
	// The following definitions are derived from webkit_strings_en-US.rc

	switch(message_id)
	{
	case 12372: return UTF8ToUTF16("This is a searchable index. Enter search keywords: ");
	case 12373: return UTF8ToUTF16("Submit");
	case 12374: return UTF8ToUTF16("Submit");
	case 12375: return UTF8ToUTF16("Reset");
	case 12376: return UTF8ToUTF16("Choose File");
	case 12377: return UTF8ToUTF16("No file chosen");
	case 12378: return UTF8ToUTF16("Drag file here");
	case 12379: return UTF8ToUTF16("No recent searches");
	case 12380: return UTF8ToUTF16("Recent Searches");
	case 12381: return UTF8ToUTF16("Clear Recent Searches");
	case 12382: return UTF8ToUTF16("%s%dx%d");
	case 12383: return UTF8ToUTF16("web area");
	case 12384: return UTF8ToUTF16("link");
	case 12385: return UTF8ToUTF16("list marker");
	case 12386: return UTF8ToUTF16("image map");
	case 12387: return UTF8ToUTF16("heading");
	case 12388: return UTF8ToUTF16("press");
	case 12389: return UTF8ToUTF16("select");
	case 12390: return UTF8ToUTF16("activate");
	case 12391: return UTF8ToUTF16("uncheck");
	case 12392: return UTF8ToUTF16("check");
	case 12393: return UTF8ToUTF16("jump");
	case 12394: return UTF8ToUTF16("2048 (High Grade)");
	case 12395: return UTF8ToUTF16("1024 (Medium Grade)");
	case 12396: return UTF8ToUTF16("$1 plugin is not installed");
	case 12397: return UTF8ToUTF16("The required plugin is not installed");
	case 12398: return UTF8ToUTF16("Click here to download plugin");
	case 12399: return UTF8ToUTF16("After installing the plugin, click here to refresh");
	case 12400: return UTF8ToUTF16("No plugin available to display this content");
	case 12401: return UTF8ToUTF16("Downloading plugin...");
	case 12402: return UTF8ToUTF16("Get Plugin");
	case 12403: return UTF8ToUTF16("Cancel");
	case 12404: return UTF8ToUTF16("$1 plugin needed");
	case 12405: return UTF8ToUTF16("Additional plugin needed");
	case 12406: return UTF8ToUTF16("Please confirm that you would like to install the $1 plugin. You should only install plugins that you trust.");
	case 12407: return UTF8ToUTF16("Please confirm that you would like to install this plugin. You should only install plugins that you trust.");
	case 12408: return UTF8ToUTF16("Install");
	case 12409: return UTF8ToUTF16("Cancel");
	case 12410: return UTF8ToUTF16("Failed to install plugin from $1");
	case 12411: return UTF8ToUTF16("Plugin installation failed");
	default: return UTF8ToUTF16("????");
	};

	/*
	const ATLSTRINGRESOURCEIMAGE* image = AtlGetStringResourceImage(_AtlBaseModule.GetModuleInstance(), message_id);

	if (!image) 
		return L"????";
	else
		return std::wstring(image->achString, image->nLength);
	*/
}

std::string GetDataResource(int resource_id)
{
	/*
#if defined(__APPLE__)
	return "";
#else
	switch(resource_id)
	{
	case IDR_BROKENIMAGE:
		{
			static std::string broken_image_data;
			
			if(broken_image_data.empty())
			{
				std::wstring path = stringToWide(Awesomium::WebCore::Get().getBaseDirectory());
				file_util::AppendToPath(&path, L"missingImage.gif");
				bool success = file_util::ReadFileToString(path, &broken_image_data);
				if(!success)
					LOG(ERROR) << "Couldn't find missingImage.gif in the base directory. " <<
						"This file is needed for broken image placeholders.";
			}

			return broken_image_data;
		}
	case IDR_FEED_PREVIEW:
		{
			// It is necessary to return a feed preview template that contains
			// a {{URL}} substring where the feed URL should go; see the code 
			// that computes feed previews in feed_preview.cc:MakeFeedPreview. 
			// This fixes issue #932714.    
			//

			return std::string("Feed preview for {{URL}}");
		}
	default:
		return "";
	}
#endif
	*/

	return "";
}

#if defined(WIN32)
HCURSOR LoadCursor(int cursor_id)
{
	return NULL;
}
#endif

bool GetApplicationDirectory(std::wstring *path)
{
	return PathService::Get(base::DIR_EXE, path);
}

GURL GetInspectorURL()
{
	return GURL();
}

std::string GetUIResourceProtocol()
{
	return "";
}

bool GetExeDirectory(std::wstring *path)
{
	return PathService::Get(base::DIR_EXE, path);
}

bool SpellCheckWord(const wchar_t* word, int word_len, int* misspelling_start, int* misspelling_len)
{
	// Report all words being correctly spelled.
	*misspelling_start = 0;
	*misspelling_len = 0;
	return true;
}

bool GetPlugins(bool refresh, std::vector<WebPluginInfo>* plugins)
{
//#if defined(WIN32)
	return NPAPI::PluginList::Singleton()->GetPlugins(refresh, plugins);
//#else
//	return false;
//#endif
}

bool IsPluginRunningInRendererProcess()
{
	/**
	* The plugin is actually running in the renderer process however we return
	* false here as a workaround to the rare event where a plugin is destroyed
	* and then immediately created. The issue is that if we return true and there
	* are no active instances using the plugin library, the lib uses deferred
	* destruction (posts a message to the loop). This causes a rather nasty
	* error if a plugin instance is immediately created after the
	* post to invoke deferred destruction of the lib is made.
	*/

	return false;
}

#if defined(WIN32)
bool EnsureFontLoaded(HFONT font)
{
	return true;
}

MONITORINFOEX GetMonitorInfoForWindow(HWND window)
{
	HMONITOR monitor = MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFOEX monitorInfo;
	monitorInfo.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfo(monitor, &monitorInfo);
	return monitorInfo;
}

bool DownloadUrl(const std::string& url, HWND caller_window)
{
	return false;
}
#endif
	
ScreenInfo GetScreenInfo(gfx::NativeViewId window)
{
	return GetScreenInfoHelper(gfx::NativeViewFromId(window));
}

bool GetPluginFinderURL(std::string* plugin_finder_url)
{
	return false;
}

bool IsDefaultPluginEnabled()
{
	return false;
}

std::wstring GetWebKitLocale()
{
	return L"en-US";
}

uint64 VisitedLinkHash(const char* canonical_url, size_t length)
{
  return 0;
}

bool IsLinkVisited(uint64 link_hash)
{
  return false;
}

#if defined(WIN32)
bool IsMediaPlayerAvailable()
{
	return false;
}

base::LazyInstance<Clipboard> clipboard(base::LINKER_INITIALIZED);

Clipboard* ClipboardGetClipboard() {
  return clipboard.Pointer();
}

bool ClipboardIsFormatAvailable(const Clipboard::FormatType& format) {
  return ClipboardGetClipboard()->IsFormatAvailable(format);
}

void ClipboardReadText(string16* result) {
  ClipboardGetClipboard()->ReadText(result);
}

void ClipboardReadAsciiText(std::string* result) {
  ClipboardGetClipboard()->ReadAsciiText(result);
}

void ClipboardReadHTML(string16* markup, GURL* url) {
  std::string url_str;
  ClipboardGetClipboard()->ReadHTML(markup, url ? &url_str : NULL);
  if (url)
    *url = GURL(url_str);
}
#endif
}  // namespace webkit_glue

#if defined(_WIN32)
#if defined(OS_WIN)
void ScopedClipboardWriterGlue::WriteBitmapFromPixels(
    const void* pixels, const gfx::Size& size) {
  ScopedClipboardWriter::WriteBitmapFromPixels(pixels, size);
}
#endif

ScopedClipboardWriterGlue::~ScopedClipboardWriterGlue() {
}
#endif