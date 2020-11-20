// str_view.h
// Copyright (c) 2020, zhiayang
// Licensed under the Apache License Version 2.0.

#pragma once

#include <string>
#include <string_view>

namespace pcombs
{
	struct str_view : public std::string_view
	{
		str_view()                          : std::string_view("") { }
		str_view(std::string&& s)           : std::string_view(std::move(s)) { }
		str_view(std::string_view&& s)      : std::string_view(std::move(s)) { }
		str_view(const std::string& s)      : std::string_view(s) { }
		str_view(const std::string_view& s) : std::string_view(s) { }
		str_view(const char* s)             : std::string_view(s) { }
		str_view(const char* s, size_t l)   : std::string_view(s, l) { }

		std::string_view sv() const   { return *this; }
		str_view drop(size_t n) const { return (this->size() > n ? this->substr(n) : ""); }
		str_view take(size_t n) const { return (this->size() > n ? this->substr(0, n) : *this); }
		str_view take_last(size_t n) const { return (this->size() > n ? this->substr(this->size() - n) : *this); }
		str_view drop_last(size_t n) const { return (this->size() > n ? this->substr(0, this->size() - n) : *this); }
		str_view substr(size_t pos = 0, size_t cnt = -1) const { return str_view(std::string_view::substr(pos, cnt)); }

		str_view& remove_prefix(size_t n) { std::string_view::remove_prefix(n); return *this; }
		str_view& remove_suffix(size_t n) { std::string_view::remove_suffix(n); return *this; }

		str_view trim_front(bool newlines = false) const
		{
			auto ret = *this;
			while(ret.size() > 0 && (ret[0] == ' ' || ret[0] == '\t' || (newlines && (ret[0] == '\r' || ret[0] == '\n'))))
				ret.remove_prefix(1);
			return ret;
		}

		str_view trim_back(bool newlines = false) const
		{
			auto ret = *this;
			while(ret.size() > 0 && (ret.back() == ' ' || ret.back() == '\t' || (newlines && (ret.back() == '\r' || ret.back() == '\n'))))
				ret.remove_suffix(1);
			return ret;
		}

		str_view trim(bool newlines = false) const
		{
			return this->trim_front(newlines).trim_back(newlines);
		}

		std::string str() const { return std::string(*this); }
	};
}
