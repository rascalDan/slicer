#ifndef SLICER_ENUM_MAP_H
#define SLICER_ENUM_MAP_H

#include <array>
#include <string>
#include <string_view>

namespace Slicer {
	enum class EnumMapKey {
		Value,
		Name,
	};

	template<typename E> class EnumMap {
	public:
		struct Node {
			E value;
			std::string_view name;
			const std::string * nameStr;
		};

		template<EnumMapKey Key, typename T>
		[[nodiscard]] constexpr inline const Node *
		find(const T & v) const noexcept
		{
			for (auto b = begin; b != end; b++) {
				if constexpr (Key == EnumMapKey::Value) {
					if (b->value == v) {
						return b;
					}
				}
				else if constexpr (Key == EnumMapKey::Name) {
					if (b->name == v) {
						return b;
					}
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

		inline constexpr EnumMapImpl(Arr<N> a) : arr(std::move(a))
		{
			EnumMap<E>::begin = arr.begin();
			EnumMap<E>::end = arr.end();
		}
		const Arr<N> arr;
	};
}

#endif
