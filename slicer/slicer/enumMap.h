#ifndef SLICER_ENUM_MAP_H
#define SLICER_ENUM_MAP_H

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
		constexpr inline const Node *
		find(const T & v) const
		{
			auto b = begin();
			const auto e = end();
			while (b != e) {
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
				++b;
			}
			return nullptr;
		}

		virtual constexpr const Node * begin() const = 0;
		virtual constexpr const Node * end() const = 0;
	};

	template<typename E, std::size_t N> class EnumMapImpl : public EnumMap<E> {
	public:
		using NodeType = typename EnumMap<E>::Node;
		template<std::size_t n> using Arr = std::array<NodeType, n>;

		constexpr const NodeType *
		begin() const override
		{
			return arr.begin();
		}

		constexpr const NodeType *
		end() const override
		{
			return arr.end();
		}

		inline constexpr EnumMapImpl(Arr<N> a) : arr(std::move(a)) { }
		const Arr<N> arr;
	};
}

#endif
