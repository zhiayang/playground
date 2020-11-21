// pcombs.h
// Copyright (c) 2020, zhiayang
// Licensed under the Apache License Version 2.0.

#pragma once

#include <string>
#include <vector>
#include <utility>
#include <optional>
#include <string_view>
#include <type_traits>

#include "zpr.h"
#include "result.h"
#include "str_view.h"

namespace pcombs
{
	template <typename T>
	using ParserResult = Result<std::pair<T, str_view>>;

	template <typename T>
	ParserResult<T> Ok(T x, str_view i)
	{
		return ParserResult<std::remove_reference_t<T>>::of_value(
			std::make_pair<std::remove_reference_t<T>, str_view>(std::move(x), std::move(i))
		);
	}

	template <typename T>
	ParserResult<T> Err(const std::string& e)
	{
		return ParserResult<T>::of_error(e);
	}


	template <typename T>
	struct Parser
	{
		Parser() { }

		template <typename Fn>
		Parser(Fn fn) : m_fn(fn) { }

		Parser(const Parser& p) = default;
		Parser& operator = (const Parser& p) = default;

		ParserResult<T> run(str_view input) const { return m_fn(std::move(input)); }

	private:
		std::function<ParserResult<T> (str_view input)> m_fn;
	};


	template <typename A, typename B>
	Parser<std::pair<A, B>> operator & (Parser<A> pa, Parser<B> pb)
	{
		using R = std::pair<A, B>;
		return [pa = std::move(pa), pb = std::move(pb)](str_view input) -> ParserResult<R> {
			auto a = pa.run(input);
			if(!a) return Err<R>(a.error());

			auto b = pb.run(a->second);
			if(!b) return Err<R>(b.error());

			return Ok(std::make_pair(a->first, b->first), b->second);
		};
	}

	template <typename A, typename B>
	Parser<A> operator << (Parser<A> pa, Parser<B> pb)
	{
		return [pa = std::move(pa), pb = std::move(pb)](str_view input) -> ParserResult<A> {
			auto a = pa.run(input);
			if(!a) return Err<A>(a.error());

			auto b = pb.run(a->second);
			if(!b) return Err<A>(b.error());

			return Ok<A>(std::move(a->first), b->second);
		};
	}

	template <typename A, typename B>
	Parser<B> operator >> (Parser<A> pa, Parser<B> pb)
	{
		return [pa = std::move(pa), pb = std::move(pb)](str_view input) -> ParserResult<B> {
			auto a = pa.run(input);
			if(!a) return Err<B>(a.error());

			return pb.run(a->second);
		};
	}

	template <typename A>
	Parser<A> operator | (Parser<A> pa, Parser<A> pb)
	{
		return [pa = std::move(pa), pb = std::move(pb)](str_view input) -> ParserResult<A> {
			if(auto a = pa.run(input); a)
				return Ok<A>(a->first, a->second);

			else if(auto b = pb.run(input); b)
				return Ok<A>(b->first, b->second);

			else
				return Err<A>(zpr::sprint("{} or {}", a.error(), b.error()));
		};
	}

	// this is the unary one
	template <typename A>
	Parser<std::vector<A>> operator * (Parser<A> p)
	{
		return [p = std::move(p)](str_view input) -> ParserResult<std::vector<A>> {
			std::vector<A> list;
			while(true)
			{
				if(auto x = p.run(input); x)
				{
					input = x->second;
					list.push_back(std::move(x->first));
				}
				else
				{
					break;
				}
			}

			return Ok(std::move(list), input);
		};
	}

	// also the unary one.
	template <typename A>
	Parser<std::vector<A>> operator + (Parser<A> p)
	{
		// return p & *p;
		using R = std::vector<A>;
		return [p = std::move(p)](str_view input) -> ParserResult<R> {
			std::vector<A> list;
			auto fst = p.run(input);
			if(!fst) return Err<R>(fst.error());

			list.push_back(std::move(fst->first));

			auto b = (*p).run(fst->second);
			if(!b) return Err<R>(b.error());

			list.insert(list.end(), b->first.begin(), b->first.end());
			return Ok(std::move(list), b->second);
		};
	}

	template <typename A>
	Parser<std::optional<A>> operator ~ (Parser<A> p)
	{
		using R = std::optional<A>;
		return [p = std::move(p)](str_view input) -> ParserResult<R> {
			auto fst = p.run(input);
			if(fst) return Ok<R>(std::move(fst->first), fst->second);
			else    return Ok<R>({ }, input);
		};
	}

	// this is a special case of & that concatenates strings.
	inline Parser<std::string> operator + (Parser<std::string> pa, Parser<std::string> pb)
	{
		using R = std::string;
		return [pa = std::move(pa), pb = std::move(pb)](str_view input) -> ParserResult<R> {
			auto a = pa.run(input);
			if(!a) return Err<R>(a.error());

			auto b = pb.run(a->second);
			if(!b) return Err<R>(b.error());

			return Ok<R>(a->first + b->first, b->second);
		};
	}

	// this is a special case of ~ that returns empty string instead of optional.
	inline Parser<std::string> operator- (Parser<std::string> p)
	{
		using R = std::string;
		return [p = std::move(p)](str_view input) -> ParserResult<R> {
			auto x = p.run(input);
			if(!x)  return Ok<R>("", input);
			else    return Ok<R>(x->first, x->second);
		};
	}


	// template <typename T, typename R, typename Fn>
	// Parser<R> map(Parser<T> p, Fn&& fn)

	template <typename T, typename R>
	Parser<R> map(Parser<T> p, std::function<R (T)> fn)
	{
		return [p = std::move(p), fn = std::move(fn)](str_view input) -> ParserResult<R> {
			auto x = p.run(input);
			if(!x) return Err<R>(x.error());

			return Ok(fn(x->first), x->second);
		};
	}

	template <typename T>
	Parser<T> ref(Parser<T>& p)
	{
		return [&p](str_view input) -> ParserResult<T> {
			return p.run(input);
		};
	}



	inline Parser<std::string> str(str_view s)
	{
		return [s](str_view input) -> ParserResult<std::string> {
			if(input.find(s) == 0)  return Ok(std::string(s), input.drop(s.size()));
			else                    return Err<std::string>(zpr::sprint("expected '{}'", s));
		};
	}

	inline Parser<std::string> any_char()
	{
		using R = std::string;
		return [](str_view input) -> ParserResult<R> {
			if(input.size() > 0)    return Ok<R>(input.take(1).str(), input.drop(1));
			else                    return Err<R>("unexpected end of input");
		};
	}

	template <typename Fn>
	Parser<std::string> take_while(Fn&& pred)
	{
		return [pred = std::move(pred)](str_view input) -> ParserResult<std::string> {
			size_t i = 0;
			while(i < input.size())
			{
				if(!pred(input[i]))
					break;

				i++;
			}

			return Ok(input.take(i).str(), input.drop(i));
		};
	}

	template <typename Fn>
	Parser<std::string> take_until(Fn&& pred)
	{
		return take_while([pred](char c) -> bool { return !pred(c); });
	}

	inline Parser<std::string> whitespace()
	{
		return take_while([](char c) -> bool { return c == ' ' || c == '\t'; });
	}

	inline Parser<std::string> newline()
	{
		return str("\n") | str("\r\n");
	}
}











namespace zpr
{
	template <>
	struct print_formatter<pcombs::str_view>
	{
		template <typename Cb>
		void print(pcombs::str_view x, Cb&& cb, format_args args)
		{
			cb(x.data(), x.size());
		}
	};

	template <typename T>
	struct print_formatter<pcombs::Result<T>>
	{
		template <typename Cb>
		void print(const pcombs::Result<T>& x, Cb&& cb, format_args args)
		{
			if(x)   cb(zpr::sprint("Ok({})", x.unwrap()));
			else    cb(zpr::sprint("Err({})", x.error()));
		}
	};

	template <typename T>
	struct print_formatter<std::optional<T>>
	{
		template <typename Cb>
		void print(std::optional<T> x, Cb&& cb, format_args args)
		{
			if(x)   cb(zpr::sprint("Some({})", x.value()));
			else    cb(zpr::sprint("None"));
		}
	};
}
