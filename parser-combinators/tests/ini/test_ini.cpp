// test_ini.cpp
// Copyright (c) 2020, zhiayang
// Licensed under the Apache License Version 2.0.

#include <fstream>
#include <sstream>

#include "zpr.h"
#include "pcombs.h"

using namespace pcombs;

using Key = std::string;
using Value = std::string;

using Pair = std::pair<Key, Value>;
using Section = std::pair<std::string, std::vector<Pair>>;
using Ini = std::vector<Section>;


auto key = take_while([](char c) {
	return c != '=' && (c == '_' || ('0' <= c && c <= '9') || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'));
});

auto value = take_while([](char c) {
	return c == '_' || ('0' <= c && c <= '9') || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
});

auto pair = (
	(whitespace() >> key << whitespace() << str("=")) &
	(whitespace() >> value << whitespace())
) << *str("\n");

auto section = ((str("[") >> take_until([](char c) { return c == ']'; }) << str("]")) << +str("\n")) & *pair;

auto ini = *(section << *str("\n"));

int main(int argc, char** argv)
{
	if(argc < 2)
	{
		zpr::fprintln(stderr, "expected input");
		exit(1);
	}

	std::string test;
	auto file = std::ifstream(argv[1]);
	std::stringstream buffer; buffer << file.rdbuf();
	test = buffer.str();

	zpr::println("{}", ini.run(test));
}
