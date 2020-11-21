// test_json.cpp
// Copyright (c) 2020, zhiayang
// Licensed under the Apache License Version 2.0.

#include <map>
#include <vector>
#include <fstream>
#include <sstream>

#include "zpr.h"
#include "pcombs.h"

using namespace pcombs;

static bool is_digit(char c) { return '0' <= c && c <= '9'; }
static auto match(char x) { return [x](char c) { return c == x; }; };

struct value_t
{
	value_t() : m_type(0) { }
	explicit value_t(bool x) : m_type(T_BOOL), m_bool(x) { }
	explicit value_t(double x) : m_type(T_NUMBER), m_number(x) { }
	explicit value_t(std::string x) : m_type(T_STRING), m_string(std::move(x)) { }
	explicit value_t(std::vector<value_t> x) : m_type(T_LIST), m_list(std::move(x)) { }
	explicit value_t(std::map<std::string, value_t> x) : m_type(T_OBJECT), m_object(std::move(x)) { }

	static constexpr int T_NULL     = 0;
	static constexpr int T_BOOL     = 1;
	static constexpr int T_NUMBER   = 2;
	static constexpr int T_STRING   = 3;
	static constexpr int T_LIST     = 4;
	static constexpr int T_OBJECT   = 5;

	int m_type = 0;
	bool m_bool;
	double m_number;
	std::string m_string;
	std::vector<value_t> m_list;
	std::map<std::string, value_t> m_object;
};

namespace zpr
{
	template <>
	struct print_formatter<value_t>
	{
		template <typename Cb>
		void print(const value_t& x, Cb&& cb, format_args args)
		{
			switch(x.m_type)
			{
				case value_t::T_NULL:   zpr::cprint(cb, "null"); return;
				case value_t::T_BOOL:   zpr::cprint(cb, "{}", x.m_bool); return;
				case value_t::T_NUMBER: zpr::cprint(cb, "{}", x.m_number); return;
				case value_t::T_STRING: zpr::cprint(cb, "{}", x.m_string); return;
				case value_t::T_LIST:   cb(zpr::sprint("{}", x.m_list).c_str()); return;
				case value_t::T_OBJECT: cb(zpr::sprint("{}", x.m_object).c_str()); return;
			}
		}
	};
}



Parser<value_t> value;

// auto list = map<std::vector<value_t>


int main(int argc, char** argv)
{
	if(argc < 2)
	{
		zpr::fprintln(stderr, "expected input");
		exit(1);
	}

	std::stringstream buffer; buffer << std::ifstream(argv[1]).rdbuf();
	auto test = buffer.str();

	using std::pair;

	using list_t = std::vector<value_t>;
	using object_t = std::map<std::string, value_t>;

	// super fucking scuffed.
	Parser<value_t> value;
	Parser<value_t> elements;
	Parser<value_t> members;

	auto whitespace = *(str("\x0A") | str("\x0D") | str("\x20") | str("\x09"));

	auto null = map<std::string, value_t>(str("null"), [](auto) { return value_t(); });

	auto boolean = map<std::string, value_t>(
		str("true") | str("false"), [](auto x) { return value_t(x == "true" ? true : false); }
	);


	auto nmbr_p = -str("-")
				+ (str("0") | str("1") | str("2") | str("3") | str("4")
					| str("5") | str("6") | str("7") | str("8") | str("9"))
				+ take_while(is_digit)
				+ -(str(".") + take_while(is_digit));

	auto number = map<std::string, value_t>(nmbr_p, [](auto x) { return value_t(std::stod(x)); });

	auto string = map<std::string, value_t>(
		str("\"") >> take_until(match('"')) << str("\""),
		[](auto x) { return value_t(std::move(x)); }
	);

	auto element = whitespace >> ref(value) << whitespace;
	elements = map<pair<value_t, list_t>, value_t>(element & *(str(",") >> element),
			 	[](auto p) {
			 		p.second.insert(p.second.begin(), p.first);
			 		return value_t(p.second);
			 	})
			 | map<value_t, value_t>(element, [](auto x) { return value_t(std::vector { std::move(x) }); });

	auto member = (whitespace >> string << whitespace << str(":")) & element;
	members = map<pair<pair<value_t, value_t>, value_t>, value_t>((whitespace >> member << str(",") << whitespace) & ref(members),
				[](auto p) {
					p.second.m_object.insert({ p.first.first.m_string, p.first.second });
					return p.second;
				})
			 | map<pair<value_t, value_t>, value_t>(member, [](auto x) { return value_t(object_t { { x.first.m_string, x.second } }); });

	auto list = map<std::string, value_t>(str("[") >> whitespace >> str("]"), [](auto) -> auto {
					return value_t(list_t { });
				}) | (str("[") >> elements << str("]"));

	auto object = map<std::string, value_t>(str("{") >> whitespace >> str("}"), [](auto) { return value_t(object_t { }); })
				| (str("{") >> members << str("}"));

	value = object | list | string | number | boolean | null;

	auto json = element;


	auto x = json.run(test);
	zpr::println("{}", x ? x->first : value_t());
}
