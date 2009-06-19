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

#include "JSValue.h"
#include "WebCore.h"
#include <sstream>

template<class NumberType>
inline NumberType stringToNumber(const std::string& numberString)
{
	if(numberString.substr(0, 4).compare("true") == 0) return 1;
	else if(numberString.substr(0, 4).compare("false") == 0) return 0;

	std::istringstream converter(numberString);

	NumberType result;
	return (converter >> result).fail() ? 0 : result;
}

template<class NumberType>
inline std::string numberToString(const NumberType &number)
{
	std::ostringstream converter;

	return (converter << number).fail() ? "" : converter.str();
}

using namespace Awesomium;
using namespace Impl;

JSValue::JSValue()
{
	varValue.type = VariantType_NULL;
}

JSValue::JSValue(bool value)
{
	varValue.type = VariantType_BOOLEAN;
	varValue.numericValue.booleanValue = value;
}

JSValue::JSValue(int value)
{
	varValue.type = VariantType_INTEGER;
	varValue.numericValue.integerValue = value;
}

JSValue::JSValue(double value)
{
	varValue.type = VariantType_DOUBLE;
	varValue.numericValue.doubleValue = value;
}

JSValue::JSValue(const char* value)
{
	varValue.type = VariantType_STRING;
	varValue.stringValue = value;
}

JSValue::JSValue(const std::string& value)
{
	varValue.type = VariantType_STRING;
	varValue.stringValue = value;
}

bool JSValue::isBoolean() const
{
	return varValue.type == VariantType_BOOLEAN;
}

bool JSValue::isInteger() const
{
	return varValue.type == VariantType_INTEGER;
}

bool JSValue::isDouble() const
{
	return varValue.type == VariantType_DOUBLE;
}

bool JSValue::isNumber() const
{
	return varValue.type == VariantType_INTEGER || varValue.type == VariantType_DOUBLE;
}

bool JSValue::isString() const
{
	return varValue.type == VariantType_STRING;
}

bool JSValue::isNull() const
{
	return varValue.type == VariantType_NULL;
}

const std::string& JSValue::toString() const
{
	if(isString())
		return varValue.stringValue;

	static std::string tempResult;

	if(isInteger())
		tempResult = numberToString<int>(varValue.numericValue.integerValue);
	else if(isDouble())
		tempResult = numberToString<double>(varValue.numericValue.doubleValue);
	else if(isBoolean())
		tempResult = numberToString<bool>(varValue.numericValue.booleanValue);
	else
		tempResult = "";

	return tempResult;
}

int JSValue::toInteger() const
{
	if(isString())
		return stringToNumber<int>(varValue.stringValue);
	else if(isInteger())
		return varValue.numericValue.integerValue;
	else if(isDouble())
		return (int)varValue.numericValue.doubleValue;
	else if(isBoolean())
		return (int)varValue.numericValue.booleanValue;
	else
		return 0;
}

double JSValue::toDouble() const
{
	if(isString())
		return stringToNumber<double>(varValue.stringValue);
	else if(isInteger())
		return (double)varValue.numericValue.integerValue;
	else if(isDouble())
		return varValue.numericValue.doubleValue;
	else if(isBoolean())
		return (double)varValue.numericValue.booleanValue;
	else
		return 0;
}

bool JSValue::toBoolean() const
{
	if(isString())
		return stringToNumber<bool>(varValue.stringValue);
	else if(isInteger())
		return !!varValue.numericValue.integerValue;
	else if(isDouble())
		return !!static_cast<int>(varValue.numericValue.doubleValue);
	else if(isBoolean())
		return varValue.numericValue.booleanValue;
	else
		return false;
}

FutureJSValue::FutureJSValue() : source(0), requestID(0)
{
}

FutureJSValue::~FutureJSValue()
{
}

void FutureJSValue::init(WebView* source, int requestID)
{
	this->source = source;
	this->requestID = requestID;
}

const JSValue& FutureJSValue::get()
{
	if(requestID)
	{
		Awesomium::WebCore::Get().resolveJSValueFuture(source, requestID, &value);
		requestID = 0;
	}
	
	return value;
}
