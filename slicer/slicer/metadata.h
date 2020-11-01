#ifndef SLICER_METADATA_H
#define SLICER_METADATA_H

#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <vector>
#include <visibility.h>

namespace Slicer {
	template<typename Iter = std::array<std::string_view, 1>::const_iterator> class DLL_PUBLIC MetaData {
	public:
		using Value = std::string_view;

		constexpr MetaData() = default;

		// Flags
		[[nodiscard]] constexpr inline bool
		flagSet(std::string_view flag) const
		{
			return find(flag) != _end;
		}

		[[nodiscard]] constexpr inline bool
		flagNotSet(std::string_view flag) const
		{
			return find(flag) == _end;
		}

		// Values
		[[nodiscard]] constexpr std::optional<std::string_view>
		value(std::string_view prefix) const
		{
			for (auto mditr = _begin; mditr != _end; mditr++) {
				const std::string_view md {*mditr};
				if (md.substr(0, prefix.length()) == prefix) {
					return md.substr(prefix.length());
				}
			}
			return {};
		}

		[[nodiscard]] std::vector<std::string_view>
		values(std::string_view prefix) const
		{
			std::vector<std::string_view> mds;
			for (auto mditr = _begin; mditr != _end; mditr++) {
				const std::string_view md {*mditr};
				if (md.substr(0, prefix.length()) == prefix) {
					mds.push_back(md.substr(prefix.length()));
				}
			}
			return mds;
		}

		[[nodiscard]] constexpr auto
		begin() const
		{
			return _begin;
		}

		[[nodiscard]] constexpr auto
		end() const
		{
			return _end;
		}

	protected:
		Iter _begin {};
		Iter _end {};

		[[nodiscard]] constexpr Iter
		find(Value v) const
		{
			for (auto mdptr = _begin; mdptr != _end; mdptr++) {
				if (*mdptr == v) {
					return mdptr;
				}
			}
			return _end;
		}
	};

	template<std::size_t N> class DLL_PUBLIC MetaDataImpl : public MetaData<> {
	public:
		using Arr = std::array<Value, N>;
		constexpr inline explicit MetaDataImpl(Arr a) : arr(std::move(a))
		{
			_begin = arr.begin();
			_end = arr.end();
		}

		Arr arr;
	};
}

#endif
