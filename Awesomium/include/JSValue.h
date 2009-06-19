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

#ifndef __JSVALUE_H__
#define __JSVALUE_H__

#include <string>
#include <vector>
#include "PlatformUtils.h"

namespace Impl {

typedef enum {
    VariantType_NULL,
	VariantType_BOOLEAN,
	VariantType_INTEGER,
	VariantType_DOUBLE,
	VariantType_STRING
} VariantType;

struct VariantValue
{
	VariantType type;

	std::string stringValue;

	union {
		bool booleanValue;
		int integerValue;
		double doubleValue;
	} numericValue;
};

}

namespace Awesomium 
{

class WebView;

/**
* JSValue is a class that represents a Javascript value. It can be initialized from
* and converted to several types: boolean, integer, double, string
*/
class _OSMExport JSValue
{
	Impl::VariantValue varValue;
public:
	/// Creates a null JSValue.
	JSValue();

	/// Creates a JSValue initialized with a boolean.
	JSValue(bool value);

	/// Creates a JSValue initialized with an integer.
	JSValue(int value);

	/// Creates a JSValue initialized with a double.
	JSValue(double value);

	/// Creates a JSValue initialized with a string.
	JSValue(const char* value);

	/// Creates a JSValue initialized with a string.
	JSValue(const std::string& value);

	/// Returns whether or not this JSValue is a boolean.
	bool isBoolean() const;

	/// Returns whether or not this JSValue is an integer.
	bool isInteger() const;

	/// Returns whether or not this JSValue is a double.
	bool isDouble() const;

	/// Returns whether or not this JSValue is a number (integer or double).
	bool isNumber() const;

	/// Returns whether or not this JSValue is a string.
	bool isString() const;

	/// Returns whether or not this JSValue is null.
	bool isNull() const;

	/**
	* Returns this JSValue as a string (converting if necessary).
	*
	* @note	If this JSValue is not a string, the returned reference
	*		is only valid until the next call to JSValue::toString.
	*/
	const std::string& toString() const;

	/// Returns this JSValue as an integer (converting if necessary).
	int toInteger() const;

	/// Returns this JSValue as a double (converting if necessary).
	double toDouble() const;

	/// Returns this JSValue as a boolean (converting if necessary).
	bool toBoolean() const;
};

typedef std::vector<JSValue> JSArguments;

/**
* FutureJSValue is a special wrapper around a JSValue that allows
* asynchronous retrieval of the actual value at a later time.
*
* If you are unfamiliar with the concept of a 'Future', please see:
* <http://en.wikipedia.org/wiki/Futures_and_promises>
*/
class _OSMExport FutureJSValue
{
public:
	FutureJSValue();
	~FutureJSValue();

	/**
	* If the internal JSValue has been computed, immediately returns
	* the value, else, blocks the calling thread until it has.
	*/
	const JSValue& get();

protected:
	void init(WebView* source, int requestID);

	JSValue value;
	WebView* source;
	int requestID;

	friend class WebView;
};

}

#endif