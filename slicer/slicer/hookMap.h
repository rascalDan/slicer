#ifndef SLICER_HOOK_MAP_H
#define SLICER_HOOK_MAP_H

#include "modelParts.h"
#include <array>
#include <visibility.h>

namespace Slicer {
	void DLL_PUBLIC to_lower(std::string s);

	template<typename T> class ModelPartForComplex;
	template<typename T> class Hooks {
	public:
		using HookPtr = const typename ModelPartForComplex<T>::HookBase *;

		template<typename K> class eq;

		template<typename K> class iter : public std::iterator<std::bidirectional_iterator_tag, HookPtr> {
		public:
			[[nodiscard]] constexpr inline iter(const eq<K> * const r, const HookPtr * c) : range(r), cur(c)
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

			const eq<K> * const range;
			const HookPtr * cur;
		};

		template<typename K> class eq {
		public:
			[[nodiscard]] constexpr inline iter<K>
			begin() const
			{
				return {this, b};
			}

			[[nodiscard]] constexpr inline iter<K>
			end() const
			{
				return {this, e};
			}

			K key;
			std::string_view HookCommon::*name;
			const HookPtr *b, *e;
		};

		template<typename K>
		[[nodiscard]] constexpr inline eq<K>
		equal_range(K && k) const
		{
			return {std::forward<K>(k), &HookCommon::name, _begin, _end};
		}

		template<typename K>
		[[nodiscard]] constexpr inline eq<K>
		equal_range_lower(K && k) const
		{
			return {std::forward<K>(k), &HookCommon::nameLower, _begin, _end};
		}

		template<typename K>
		[[nodiscard]] inline auto
		equal_range_nocase(const K & k) const
		{
			std::string i {k};
			to_lower(i);
			return equal_range_lower(std::move(i));
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

#endif
