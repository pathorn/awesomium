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

#ifndef __NavigationController_H__
#define __NavigationController_H__

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/linked_ptr.h"
#include "base/ref_counted.h"
#include "googleurl/src/gurl.h"
#include "WebURLRequest.h"
#include "WebHistoryItem.h"
#include "WebViewProxy.h"
#include "WebDataSource.h"

/**
* This NavigationController is largely based on Chromium's TestShell implementation
* and has been extended to accomodate navigations with credentials and HTML-string only
* navigations.
*/

using WebKit::WebDataSource;

// Associated with browser-initated navigations to hold tracking data.
class NavigationExtraRequestData : public WebDataSource::ExtraData {
 public:
  NavigationExtraRequestData(int32 pending_page_id)
	  : WebDataSource::ExtraData(),
        pending_page_id(pending_page_id),
        request_committed(false) {}

  // Contains the page_id for this navigation or -1 if there is none yet.
  int32 pending_page_id;

  // True if we have already processed the "DidCommitLoad" event for this
  // request.  Used by session history.
  bool request_committed;
};

// Stores one back/forward navigation state for the test shell.
class NavigationEntry {
 public:
	NavigationEntry() : page_id_(-1) {}

	NavigationEntry(int page_id, const GURL& url, const std::wstring& title, const std::wstring& target_frame,
		const std::string& authUsername = "", const std::string& authPassword = "")
	: page_id_(page_id),
	  url_(url),
	  title_(title),
	  target_frame_(target_frame),
	  authUsername(authUsername), authPassword(authPassword) {}

	NavigationEntry(int page_id, const std::string& htmlString, const GURL& url, const std::wstring& target_frame)
		: page_id_(page_id),
		htmlString(htmlString),
		url_(url),
		target_frame_(target_frame){}

	// Virtual to allow test_shell to extend the class.
	~NavigationEntry() {}

	// Set / Get the URI
	void SetURL(const GURL& url) { url_ = url; }
	const GURL& GetURL() const { return url_; }

	// Set / Get the title
	void SetTitle(const std::wstring& a_title) { title_ = a_title; }
	const std::wstring& GetTitle() const { return title_; }

	// Set / Get opaque state.
	// WARNING: This state is saved to the database and used to restore previous
	// states. If you use write a custom TabContents and provide your own
	// state make sure you have the ability to modify the format in the future
	// while being able to deal with older versions.
	void SetContentItem(const WebKit::WebHistoryItem& item)
	{
//		cached_history_item_ = NULL;  // invalidate our cached item
		historyitem_ = item;
	}

	const WebKit::WebHistoryItem& GetContentItem() const { return historyitem_; }

	// Get the page id corresponding to the tab's state.
	void SetPageID(int page_id) { page_id_ = page_id; }
	int32 GetPageID() const { return page_id_; }

	const std::wstring& GetTargetFrame() const { return target_frame_; }

	void GetAuthorizationCredentials(std::string& username, std::string& password)
	{
		username = authUsername;
		password = authPassword;
	}

	const std::string& GetHTMLString() const { return htmlString; }

private:
	// Describes the current page that the tab represents. This is not relevant
	// for all tab contents types.
	int32 page_id_;

	GURL url_;
	std::wstring title_;
	WebKit::WebHistoryItem historyitem_;
	std::string authUsername, authPassword;
	std::string htmlString;

//	mutable scoped_refptr<WebKit::WebHistoryItem> cached_history_item_;

	std::wstring target_frame_;

	DISALLOW_COPY_AND_ASSIGN(NavigationEntry);
};

// Test shell's NavigationController.  The goal is to be as close to the Chrome
// version as possible.
class NavigationController {
public:
	NavigationController(WebViewProxy* webView)
    : pending_entry_(NULL),
      last_committed_entry_index_(-1),
      pending_entry_index_(-1),
      parentWebView(webView),
      max_page_id_(-1) {}

	~NavigationController() 
	{
		DiscardPendingEntry();
	}

	void Reset() 
	{
		entries_.clear();
		DiscardPendingEntry();

		last_committed_entry_index_ = -1;
	}

	// Causes the controller to reload the current (or pending) entry.
	void Reload() 
	{
	  // Base the navigation on where we are now...
	  int current_index = GetCurrentEntryIndex();

	  // If we are no where, then we can't reload.  TODO(darin): We should add a
	  // CanReload method.
	  if (current_index == -1)
		return;

	  DiscardPendingEntry();

	  pending_entry_index_ = current_index;
	  NavigateToPendingEntry(true);
	}

	// Causes the controller to go to the specified offset from current.  Does
	// nothing if out of bounds.
	void GoToOffset(int offset) 
	{
	  int index = last_committed_entry_index_ + offset;
	  if (index < 0 || index >= GetEntryCount())
		return;

	  GoToIndex(index);
	}

	// Causes the controller to go to the specified index.
	void GoToIndex(int index) 
	{
	  DCHECK(index >= 0);
	  DCHECK(index < static_cast<int>(entries_.size()));

	  DiscardPendingEntry();

	  pending_entry_index_ = index;
	  NavigateToPendingEntry(false);
	}

	// Causes the controller to load the specified entry.  The controller
	// assumes ownership of the entry.
	// NOTE: Do not pass an entry that the controller already owns!
	void LoadEntry(NavigationEntry* entry) 
	{
	  // When navigating to a new page, we don't know for sure if we will actually
	  // end up leaving the current page.  The new page load could for example
	  // result in a download or a 'no content' response (e.g., a mailto: URL).
	  DiscardPendingEntry();
	  pending_entry_ = entry;
	  NavigateToPendingEntry(false);
	}

	// Returns the last committed entry, which may be null if there are no
	// committed entries.
	NavigationEntry* GetLastCommittedEntry() const 
	{
	  if (last_committed_entry_index_ == -1)
		return NULL;
	  return entries_[last_committed_entry_index_].get();
	}

	// Returns the number of entries in the NavigationControllerBase, excluding
	// the pending entry if there is one.
	int GetEntryCount() const 
	{
		return static_cast<int>(entries_.size());
	}

	// Returns the active entry, which is the pending entry if a navigation is in
	// progress or the last committed entry otherwise.  NOTE: This can be NULL!!
	//
	// If you are trying to get the current state of the NavigationControllerBase,
	// this is the method you will typically want to call.
	NavigationEntry* GetActiveEntry() const 
	{
	  NavigationEntry* entry = pending_entry_;
	  if (!entry)
		entry = GetLastCommittedEntry();
	  return entry;
	}

	// Returns the index from which we would go back/forward or reload.  This is
	// the last_committed_entry_index_ if pending_entry_index_ is -1.  Otherwise,
	// it is the pending_entry_index_.
	int GetCurrentEntryIndex() const 
	{
	  if (pending_entry_index_ != -1)
		return pending_entry_index_;
	  return last_committed_entry_index_;
	}

	// Returns the entry at the specified offset from current.  Returns NULL
	// if out of bounds.
	NavigationEntry* GetEntryAtOffset(int offset) const 
	{
	  int index = last_committed_entry_index_ + offset;
	  if (index < 0 || index >= GetEntryCount())
		return NULL;

	  return entries_[index].get();
	}

	// Return the entry with the corresponding type and page_id, or NULL if
	// not found.
	NavigationEntry* GetEntryWithPageID(int32 page_id) const 
	{
	  int index = GetEntryIndexWithPageID(page_id);
	  return (index != -1) ? entries_[index].get() : NULL;
	}

	// Returns the index of the last committed entry.
	int GetLastCommittedEntryIndex() const 
	{
		return last_committed_entry_index_;
	}

	// Used to inform us of a navigation being committed for a tab. We will take
	// ownership of the entry. Any entry located forward to the current entry will
	// be deleted. The new entry becomes the current entry.
	void DidNavigateToEntry(NavigationEntry* entry) 
	{
	  // If the entry is that of a page with PageID larger than any this Tab has
	  // seen before, then consider it a new navigation.
	  if (entry->GetPageID() > GetMaxPageID()) {
		InsertEntry(entry);
		return;
	  }

	  // Otherwise, we just need to update an existing entry with matching PageID.
	  // If the existing entry corresponds to the entry which is pending, then we
	  // must update the current entry index accordingly.  When navigating to the
	  // same URL, a new PageID is not created.

	  int existing_entry_index = GetEntryIndexWithPageID(entry->GetPageID());
	  NavigationEntry* existing_entry = (existing_entry_index != -1) ?
		  entries_[existing_entry_index].get() : NULL;
	  if (!existing_entry) {
		// No existing entry, then simply ignore this navigation!
		DLOG(WARNING) << "ignoring navigation for page: " << entry->GetPageID();
	  } else if (existing_entry == pending_entry_) {
		// The given entry might provide a new URL... e.g., navigating back to a
		// page in session history could have resulted in a new client redirect.
		existing_entry->SetURL(entry->GetURL());
		existing_entry->SetContentItem(entry->GetContentItem());
		last_committed_entry_index_ = pending_entry_index_;
		pending_entry_index_ = -1;
		pending_entry_ = NULL;
	  } else if (pending_entry_ && pending_entry_->GetPageID() == -1 &&
				 pending_entry_->GetURL() == existing_entry->GetURL()) {
		// Not a new navigation
		DiscardPendingEntry();
	  } else {
		// The given entry might provide a new URL... e.g., navigating to a page
		// might result in a client redirect, which should override the URL of the
		// existing entry.
		existing_entry->SetURL(entry->GetURL());
		existing_entry->SetContentItem(entry->GetContentItem());

		// The navigation could have been issued by the renderer, so be sure that
		// we update our current index.
		last_committed_entry_index_ = existing_entry_index;
	  }

	  delete entry;
	  UpdateMaxPageID();
	}

	// Used to inform us to discard its pending entry.
	void DiscardPendingEntry()
	{
	  if (pending_entry_index_ == -1)
		delete pending_entry_;
	  pending_entry_ = NULL;
	  pending_entry_index_ = -1;
	}

	private:
	// Inserts an entry after the current position, removing all entries after it.
	// The new entry will become the active one.
	void InsertEntry(NavigationEntry* entry) 
	{
	  DiscardPendingEntry();

	  // Prune any entry which are in front of the current entry
	  int current_size = static_cast<int>(entries_.size());
	  if (current_size > 0) {
		while (last_committed_entry_index_ < (current_size - 1)) {
		  entries_.pop_back();
		  current_size--;
		}
	  }

	  entries_.push_back(linked_ptr<NavigationEntry>(entry));
	  last_committed_entry_index_ = static_cast<int>(entries_.size()) - 1;
	  UpdateMaxPageID();
	}

	int GetMaxPageID() const { return max_page_id_; }

	void NavigateToPendingEntry(bool reload) 
	{
	  // For session history navigations only the pending_entry_index_ is set.
	  if (!pending_entry_) {
		DCHECK(pending_entry_index_ != -1);
		pending_entry_ = entries_[pending_entry_index_].get();
	  }

	  if (parentWebView->navigate(pending_entry_, reload)) {
		// Note: this is redundant if navigation completed synchronously because
		// DidNavigateToEntry call this as well.
		//xxx ajs UpdateMaxPageID();
	  } else {
		DiscardPendingEntry();
	  }
	}

	// Return the index of the entry with the corresponding type and page_id,
	// or -1 if not found.
	int GetEntryIndexWithPageID(int32 page_id) const
	{
	  for (int i = static_cast<int>(entries_.size())-1; i >= 0; --i) {
		if (entries_[i]->GetPageID() == page_id)
		  return i;
	  }
	  return -1;
	}

	// Updates the max page ID with that of the given entry, if is larger.
	void UpdateMaxPageID() 
	{
	  NavigationEntry* entry = GetActiveEntry();
	  if (entry)
		max_page_id_ = std::max(max_page_id_, entry->GetPageID());
	}


	// List of NavigationEntry for this tab
	typedef std::vector< linked_ptr<NavigationEntry> > NavigationEntryList;
	typedef NavigationEntryList::iterator NavigationEntryListIterator;
	NavigationEntryList entries_;

	// An entry we haven't gotten a response for yet.  This will be discarded
	// when we navigate again.  It's used only so we know what the currently
	// displayed tab is.
	NavigationEntry* pending_entry_;

	// currently visible entry
	int last_committed_entry_index_;

	// index of pending entry if it is in entries_, or -1 if pending_entry_ is a
	// new entry (created by LoadURL).
	int pending_entry_index_;

	WebViewProxy* parentWebView;
	int max_page_id_;

	DISALLOW_EVIL_CONSTRUCTORS(NavigationController);
};

#endif
