#ifndef SLICER_METADATA_H
#define SLICER_METADATA_H

#include <algorithm>
#include <array>
#include <cstddef>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
#include <visibility.h>

namespace Slicer {
	template<bool FixedSize = true, typename Value = std::string_view> class DLL_PUBLIC MetaData {
	public:
		using Pair = std::pair<Value, std::string_view>;
		using PairView = std::pair<std::string_view, std::string_view>;
		template<size_t size = 0>
		using ContainerBase = std::conditional_t<FixedSize, std::array<Pair, size>, std::vector<Pair>>;
		using Iter = typename ContainerBase<>::const_iterator;

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
			remove_colons(prefix);
			for (const auto & md : *this) {
				// cppcheck-suppress useStlAlgorithm; (not constexpr)
				if (md.second == prefix) {
					return std::string_view(md.first).substr(prefix.length() + 1);
				}
			}
			return {};
		}

		[[nodiscard]] std::vector<std::string_view>
		values(std::string_view scope) const
		{
			remove_colons(scope);
			std::vector<std::string_view> mds;
			for (const auto & md : *this) {
				// cppcheck-suppress useStlAlgorithm; (not constexpr)
				if (in_scope(md.first, scope)) {
					mds.push_back(std::string_view(md.first).substr(scope.length() + 1));
				}
			}
			return mds;
		}

		[[nodiscard]] inline constexpr auto
		begin() const
		{
			return _begin;
		}

		[[nodiscard]] inline constexpr auto
		end() const
		{
			return _end;
		}

		[[nodiscard]] constexpr auto
		find(std::string_view v) const
		{
			for (const auto & md : *this) {
				// cppcheck-suppress useStlAlgorithm; (not constexpr)
				if (md.first == v) {
					return &md;
				}
			}
			return _end;
		}

		static constexpr inline auto
		remove_colons(std::string_view & s)
		{
			if (auto c = s.find_last_not_of(':'); c != std::string_view::npos) {
				s.remove_suffix(s.length() - 1 - c);
			}
			return s;
		}

		static constexpr inline auto
		in_scope(std::string_view md, std::string_view scope)
		{
			return ((md.length() == scope.length() || (md.length() > scope.length() && md[scope.length()] == ':'))
					&& md.compare(0, scope.length(), scope) == 0);
		}

	protected:
		Iter _begin {};
		Iter _end {};
	};

	template<std::size_t N> class DLL_PUBLIC MetaDataImpl : public MetaData<> {
	public:
		using Arr = ContainerBase<N>;

		constexpr inline explicit MetaDataImpl(const std::array<std::string_view, N> & a) :
			arr {[&a]() {
				Arr rtn;
				std::transform(a.begin(), a.end(), rtn.begin(), [](const auto & md) -> typename Arr::value_type {
					return {md, md.substr(0, md.rfind(':'))};
				});
				return rtn;
			}()}
		{
			_begin = arr.begin();
			_end = arr.end();
		}

		Arr arr;
	};
}

#endif
