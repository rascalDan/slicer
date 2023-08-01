#ifndef SLICER_ENUM_MAP_H
#define SLICER_ENUM_MAP_H

#include <array>
#include <cstddef>
#include <string>
#include <string_view>

namespace Slicer {
	template<typename E> class EnumMap {
	public:
		struct Node {
			E value {};
			std::string_view name {};
			const std::string * nameStr {};
		};

		[[nodiscard]] constexpr inline const Node *
		find(std::string_view v) const noexcept
		{
			for (auto b = begin; b != end; b++) {
				if (b->name == v) {
					return b;
				}
			}
			return nullptr;
		}

		[[nodiscard]] constexpr inline const Node *
		find(E v) const noexcept
		{
			for (auto b = begin; b != end; b++) {
				if (b->value == v) {
					return b;
				}
			}
			return nullptr;
		}

	protected:
		const Node * begin {};
		const Node * end {};
	};

	template<typename E, std::size_t N> class EnumMapImpl : public EnumMap<E> {
	public:
		using NodeType = typename EnumMap<E>::Node;
		template<std::size_t n> using Arr = std::array<NodeType, n>;

		inline constexpr explicit EnumMapImpl(Arr<N> a) : arr(std::move(a))
		{
			EnumMap<E>::begin = arr.begin();
			EnumMap<E>::end = arr.end();
		}

		const Arr<N> arr;
	};
}

#endif
