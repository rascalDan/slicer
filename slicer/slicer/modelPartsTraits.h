#pragma once

#include <Ice/Optional.h>
#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>

namespace Slicer {
	// Function traits helpers
	template<typename R, typename... Args> struct function_traits;

	template<typename R, typename... Args> struct function_traits<std::function<R(Args...)>> {
		template<size_t A> struct arg {
			using type = std::tuple_element_t<A, std::tuple<Args...>>;
		};
	};

	template<typename F> using callable_traits = function_traits<std::function<std::remove_pointer_t<F>>>;

	template<typename F, size_t A> using callable_param = typename callable_traits<F>::template arg<A>::type;

	// Converters that remove "optionalness".
	template<typename X> struct Coerce {
		using T = std::decay_t<X>;

		[[nodiscard]] inline constexpr T &
		operator()(T & x) const noexcept
		{
			return x;
		}

		[[nodiscard]] inline constexpr const T &
		operator()(const T & x) const noexcept
		{
			return x;
		}

		template<typename Y>
		[[nodiscard]] inline constexpr T &
		operator()(Ice::optional<Y> & x) const noexcept
		{
			if (!x) {
				x = Y();
			}
			return *x;
		}

		template<typename Y>
		[[nodiscard]] inline constexpr const T &
		operator()(const Ice::optional<Y> & x) const
		{
			return *x;
		}

		[[nodiscard]] inline constexpr static bool
		valueExists(const T &) noexcept
		{
			return true;
		}

		[[nodiscard]] inline constexpr static bool
		valueExists(const Ice::optional<T> & y) noexcept
		{
			return y.has_value();
		}
	};

	template<typename X> struct Coerce<Ice::optional<X>> {
		using T = std::decay_t<X>;

		[[nodiscard]] inline constexpr Ice::optional<T> &
		operator()(Ice::optional<T> & x) const noexcept
		{
			return x;
		}

		[[nodiscard]] inline constexpr const Ice::optional<T> &
		operator()(const Ice::optional<T> & x) const noexcept
		{
			return x;
		}

		template<typename Y>
		[[nodiscard]] inline constexpr Ice::optional<T>
		operator()(Y & y) const noexcept
		{
			return y;
		}

		[[nodiscard]] inline constexpr static bool
		valueExists(const T &) noexcept
		{
			return true;
		}

		[[nodiscard]] inline constexpr static bool
		valueExists(const Ice::optional<T> &) noexcept
		{
			return true;
		}
	};
}
