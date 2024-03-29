#pragma once

#include "modelParts.h"
#include <array>
#include <cstddef>
#include <iterator>
#include <string>
#include <string_view>
#include <visibility.h>

namespace Slicer {
	void DLL_PUBLIC to_lower(std::string & s);
	[[nodiscard]] std::string DLL_PUBLIC to_lower_copy(const std::string & s);

	template<typename T> class ModelPartForComplex; // IWYU pragma: keep

	template<typename T> class Hooks {
	public:
		using HookPtr = const typename ModelPartForComplex<T>::HookBase *;

		class eq;

		class iter {
		public:
			using iterator_category = std::bidirectional_iterator_tag;
			using value_type = HookPtr;
			using difference_type = std::ptrdiff_t;
			using pointer = HookPtr *;
			using reference = HookPtr &;

			[[nodiscard]] constexpr inline explicit iter(const eq * const r) : range(r), cur(r->e) { }

			[[nodiscard]] constexpr inline iter(const eq * const r, const HookPtr * c) : range(r), cur(c)
			{
				moveMatch();
			}

			[[nodiscard]] constexpr inline HookPtr
			operator*() const
			{
				return *cur;
			}

			[[nodiscard]] constexpr inline HookPtr
			operator->() const
			{
				return *cur;
			}

			constexpr inline void
			operator++()
			{
				if (cur++ != range->e) {
					moveMatch();
				}
			}

			[[nodiscard]] constexpr inline iter
			operator+(std::size_t n) const
			{
				auto i {*this};
				while (n--) {
					++i;
				}
				return i;
			}

			[[nodiscard]] constexpr inline bool
			operator!=(const iter & other) const
			{
				return cur != other.cur;
			}

			[[nodiscard]] constexpr inline bool
			operator==(const iter & other) const
			{
				return cur == other.cur;
			}

		private:
			constexpr void
			moveMatch()
			{
				while (cur != range->e && (*cur)->*(range->name) != range->key) {
					cur++;
				}
			}

			const eq * const range;
			const HookPtr * cur;
		};

		class eq {
		public:
			[[nodiscard]] constexpr inline iter
			begin() const
			{
				return iter {this, b};
			}

			[[nodiscard]] constexpr inline iter
			end() const
			{
				return iter {this};
			}

			std::string_view key {};
			std::string_view HookCommon::*name {};
			const HookPtr *b {}, *e {};
		};

		[[nodiscard]] constexpr inline eq
		equal_range(std::string_view k) const
		{
			return {k, &HookCommon::name, _begin, _end};
		}

		[[nodiscard]] constexpr inline eq
		equal_range_lower(std::string_view k) const
		{
			return {k, &HookCommon::nameLower, _begin, _end};
		}

		[[nodiscard]] constexpr inline auto
		begin() const
		{
			return _begin;
		}

		[[nodiscard]] constexpr inline auto
		end() const
		{
			return _end;
		}

	protected:
		const HookPtr * _begin {};
		const HookPtr * _end {};
	};

	template<typename T, std::size_t N> class HooksImpl : public Hooks<T> {
	public:
		using HookPtr = typename Hooks<T>::HookPtr;
		template<std::size_t n> using Arr = std::array<HookPtr, n>;

		inline constexpr explicit HooksImpl(Arr<N> a) : arr(std::move(a))
		{
			Hooks<T>::_begin = arr.begin();
			Hooks<T>::_end = arr.end();
		}

		const Arr<N> arr;
	};
}
