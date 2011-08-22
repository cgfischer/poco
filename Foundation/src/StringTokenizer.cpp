//
// StringTokenizer.cpp
//
// $Id: //poco/svn/Foundation/src/StringTokenizer.cpp#2 $
//
// Library: Foundation
// Package: Core
// Module:  StringTokenizer
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#include "Poco/StringTokenizer.h"
#include "Poco/Ascii.h"


namespace Poco {


StringTokenizer::StringTokenizer(const std::string& str, const std::string& separators, int options)
{
	std::string::const_iterator it1 = str.begin();
	std::string::const_iterator it2;
	std::string::const_iterator it3;
	std::string::const_iterator end = str.end();
	
	while (it1 != end)
	{
		if (options & TOK_TRIM)
		{
			while (it1 != end && Ascii::isSpace(*it1)) ++it1;
		}
		it2 = it1;
		while (it2 != end && separators.find(*it2) == std::string::npos) ++it2;
		it3 = it2;
		if (it3 != it1 && (options & TOK_TRIM))
		{
			--it3;
			while (it3 != it1 && Ascii::isSpace(*it3)) --it3;
			if (!Ascii::isSpace(*it3)) ++it3;
		}
		if (options & TOK_IGNORE_EMPTY)
		{
			if (it3 != it1)
				_tokens.push_back(std::string(it1, it3));
		}
		else
		{
			_tokens.push_back(std::string(it1, it3));
		}
		it1 = it2;
		if (it1 != end) ++it1;
	}
}


StringTokenizer::~StringTokenizer()
{
}


bool StringTokenizer::has(const std::string& token) const
{
	Iterator it = begin();
	Iterator stop = end();
	for (; it != stop; ++it)
		if (*it == token) return true;

	return false;
}


std::size_t StringTokenizer::find(const std::string& token, std::size_t pos) const
{
	Iterator it = begin();
	Iterator stop = end();
	for (it += pos; it != stop; ++it)
		if (*it == token) return it - begin();

	throw NotFoundException(token);
}


std::size_t StringTokenizer::replace(const std::string& oldToken, const std::string& newToken, std::size_t pos)
{
	std::size_t count = 0;
	TokenVec::iterator it = _tokens.begin();
	TokenVec::iterator stop = _tokens.end();
	for (it += pos; it != stop; ++it)
	{
		if (*it == oldToken)
		{
			*it = newToken;
			++count;
		}
	}

	return count;
}


std::size_t StringTokenizer::count(const std::string& token) const
{
	std::size_t cnt = 0;
	Iterator it = begin();
	Iterator stop = end();
	for (; it != stop; ++it)
		if (*it == token) ++cnt;

	return cnt;
}


} // namespace Poco
