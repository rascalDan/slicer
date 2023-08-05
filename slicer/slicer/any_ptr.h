#pragma once

#include <memory>
#include <vector>

namespace Slicer {
	template<typename T> struct any_ptr {
		// cppcheck-suppress noExplicitConstructor
		// NOLINTNEXTLINE(hicpp-explicit-conversions)
		inline constexpr any_ptr(T * p) noexcept : ptr {p} { }

		template<typename S>
			requires requires(S p) { p.get(); }
		// cppcheck-suppress noExplicitConstructor
		// NOLINTNEXTLINE(hicpp-explicit-conversions)
		inline constexpr any_ptr(const S & p) noexcept : ptr {p.get()}
		{
		}

		inline constexpr auto
		get() const noexcept
		{
			return ptr;
		}

		inline constexpr auto
		operator->() const noexcept
		{
			return ptr;
		}

		inline constexpr auto &
		operator*() const noexcept
		{
			return *ptr;
		}

		inline constexpr explicit operator bool() const noexcept
		{
			return ptr;
		}

		inline constexpr bool
		operator!() const noexcept
		{
			return !ptr;
		}

	private:
		T * ptr;
	};
}
