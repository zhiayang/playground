// result.h
// Copyright (c) 2020, zhiayang
// Licensed under the Apache License Version 2.0.

#pragma once

#include <string>
#include <optional>

namespace pcombs
{
	template <typename T, typename E = std::string>
	struct Result
	{
		static_assert(!std::is_reference_v<T>);
		static_assert(!std::is_reference_v<E>);

	private:
		static constexpr int STATE_NONE = 0;
		static constexpr int STATE_VAL  = 1;
		static constexpr int STATE_ERR  = 2;

		Result() : state(STATE_NONE) { }

	public:
		// Result() : state(0) { }
		~Result()
		{
		}

		Result(const Result& other)
		{
			this->state = other.state;
			if(this->state == STATE_VAL) this->val = other.val;
			if(this->state == STATE_ERR) this->err = other.err;
		}

		Result(Result&& other)
		{
			this->state = other.state;
			other.state = STATE_NONE;

			if(this->state == STATE_VAL) this->val = std::move(other.val);
			if(this->state == STATE_ERR) this->err = std::move(other.err);
		}

		Result& operator=(const Result& other)
		{
			if(this != &other)
			{
				this->state = other.state;
				if(this->state == STATE_VAL) this->val = other.val;
				if(this->state == STATE_ERR) this->err = other.err;
			}
			return *this;
		}

		Result& operator=(Result&& other)
		{
			if(this != &other)
			{
				this->state = other.state;
				other.state = STATE_NONE;

				if(this->state == STATE_VAL) this->val = std::move(other.val);
				if(this->state == STATE_ERR) this->err = std::move(other.err);
			}
			return *this;
		}

		T* operator -> () { assert(this->state == STATE_VAL); return &this->val; }
		const T* operator -> () const { assert(this->state == STATE_VAL); return &this->val; }

		operator bool() const { return this->state == STATE_VAL; }
		bool has_value() const { return this->state == STATE_VAL; }

		const T& unwrap() const { assert(this->state == STATE_VAL); return this->val; }
		const E& error() const { assert(this->state == STATE_ERR); return this->err; }

		T& unwrap() { assert(this->state == STATE_VAL); return this->val; }
		E& error() { assert(this->state == STATE_ERR); return this->err; }

		using result_type = T;
		using error_type = E;

		static Result of(std::optional<T> opt, const E& err)
		{
			if(opt.has_value()) return Result<T, E>(opt.value());
			else                return Result<T, E>(err);
		}

		static Result of_value(const T& x)  { auto r = Result<T>(); r.state = STATE_VAL; r.val = x; return r; }
		static Result of_value(T&& x)       { auto r = Result<T>(); r.state = STATE_VAL; r.val = std::move(x); return r; }

		static Result of_error(const E& x)  { auto r = Result<T>(); r.state = STATE_ERR; r.err = x; return r; }
		static Result of_error(E&& x)       { auto r = Result<T>(); r.state = STATE_ERR; r.err = std::move(x); return r; }

		// we use this in the parser a lot, so why not lmao
		template <typename U, typename = std::enable_if_t<
			std::is_pointer_v<T> && std::is_pointer_v<U>
			&& std::is_base_of_v<std::remove_pointer_t<U>, std::remove_pointer_t<T>>
		>>
		operator Result<U, E> () const
		{
			if(state == STATE_VAL)  return Result<U, E>(this->val);
			if(state == STATE_ERR)  return Result<U, E>(this->err);

			abort();
		}

	private:
		// 0 = schrodinger -- no error, no value.
		// 1 = valid
		// 2 = error
		int state = 0;
		T val;
		E err;
	};
}
