#ifndef SLICER_HOOK_MAP_H
#define SLICER_HOOK_MAP_H

#include "modelParts.h"
#include <array>
#include <boost/algorithm/string/case_conv.hpp>

namespace Slicer {
	template<typename T> class ModelPartForComplex;
	template<typename T> class Hooks {
	public:
		using HookPtr = const typename ModelPartForComplex<T>::HookBase *;

		template<typename K> class eq;

		template<typename K> class iter : public std::iterator<std::bidirectional_iterator_tag, HookPtr> {
		public:
			iter(const eq<K> * const r, const HookPtr * c) : range(r), cur(c)
			{
				operator++(); // move to first match
			}

			HookPtr
			operator*() const
			{
				return *cur;
			}

			void
			operator++()
			{
				for (; cur != range->e && (*cur)->*(range->name) != range->key; cur++) { }
			}

			bool
			operator!=(const iter & other) const
			{
				return cur != other.cur;
			}

		private:
			const eq<K> * const range;
			const HookPtr * cur;
		};

		template<typename K> class eq {
		public:
			iter<K>
			begin() const
			{
				return {this, b};
			}

			iter<K>
			end() const
			{
				return {this, e};
			}

			K key;
			std::string_view HookCommon::*name;
			const HookPtr *b, *e;
		};

		template<typename K>
		inline eq<K>
		equal_range(const K & k, bool matchCase) const
		{
			return {matchCase ? k : boost::algorithm::to_lower_copy(k),
					matchCase ? &HookCommon::name : &HookCommon::nameLower, begin(), end()};
		}

		virtual constexpr const HookPtr * begin() const = 0;
		virtual constexpr const HookPtr * end() const = 0;
	};

	template<typename T, std::size_t N> class HooksImpl : public Hooks<T> {
	public:
		using HookPtr = typename Hooks<T>::HookPtr;
		template<std::size_t n> using Arr = std::array<HookPtr, n>;

		inline constexpr HooksImpl(Arr<N> a) : arr(std::move(a)) { }

		constexpr const HookPtr *
		begin() const override
		{
			return arr.begin();
		}

		constexpr const HookPtr *
		end() const override
		{
			return arr.end();
		}

	private:
		const Arr<N> arr;
	};
}

#endif
