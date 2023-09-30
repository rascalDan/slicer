#pragma once

#include "any_ptr.h"
#include "metadata.h"
#include <Ice/Config.h>
#include <c++11Helpers.h>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <visibility.h>

namespace Ice {
	class InputStream;
	class OutputStream;
}

namespace Slicer {
	template<typename T> class TValueTarget {
	public:
		constexpr TValueTarget() = default;
		virtual ~TValueTarget() = default;
		virtual void get(const T &) const = 0;
		SPECIAL_MEMBERS_DEFAULT(TValueTarget);
	};

	class ValueTarget :
		public TValueTarget<bool>,
		public TValueTarget<Ice::Byte>,
		public TValueTarget<Ice::Short>,
		public TValueTarget<Ice::Int>,
		public TValueTarget<Ice::Long>,
		public TValueTarget<Ice::Float>,
		public TValueTarget<Ice::Double>,
		public TValueTarget<std::string> {
	public:
		using TValueTarget<bool>::get;
		using TValueTarget<Ice::Byte>::get;
		using TValueTarget<Ice::Short>::get;
		using TValueTarget<Ice::Int>::get;
		using TValueTarget<Ice::Long>::get;
		using TValueTarget<Ice::Float>::get;
		using TValueTarget<Ice::Double>::get;
		using TValueTarget<std::string>::get;
	};

	template<typename T> class TValueSource {
	public:
		constexpr TValueSource() = default;
		virtual ~TValueSource() = default;
		virtual void set(T &) const = 0;
		SPECIAL_MEMBERS_DEFAULT(TValueSource);
	};

	class ValueSource :
		public TValueSource<bool>,
		public TValueSource<Ice::Byte>,
		public TValueSource<Ice::Short>,
		public TValueSource<Ice::Int>,
		public TValueSource<Ice::Long>,
		public TValueSource<Ice::Float>,
		public TValueSource<Ice::Double>,
		public TValueSource<std::string> {
	public:
		using TValueSource<bool>::set;
		using TValueSource<Ice::Byte>::set;
		using TValueSource<Ice::Short>::set;
		using TValueSource<Ice::Int>::set;
		using TValueSource<Ice::Long>::set;
		using TValueSource<Ice::Float>::set;
		using TValueSource<Ice::Double>::set;
		using TValueSource<std::string>::set;
	};

	class ModelPart;
	class ModelPartForRootBase;
	class HookCommon;

	struct ClassRefBase {
		constexpr ClassRefBase() = default;
		virtual ~ClassRefBase() = default;
		SPECIAL_MEMBERS_DEFAULT(ClassRefBase);
	};

	using ModelPartParam = any_ptr<ModelPart>;
	using ModelPartForRootParam = any_ptr<ModelPartForRootBase>;
	using TypeId = std::optional<std::string>;
	using Metadata = MetaData<>;
	using ChildHandler = std::function<void(const std::string &, ModelPartParam, const HookCommon *)>;
	using ModelPartHandler = std::function<void(ModelPartParam)>;
	using ModelPartRootHandler = std::function<void(ModelPartForRootParam)>;
	using SubPartHandler = std::function<void(ModelPartParam, const Metadata &)>;
	using HookFilter = std::function<bool(const HookCommon *)>;
	constexpr Metadata emptyMetadata;

	enum class ModelPartType {
		Null,
		Simple,
		Complex,
		Sequence,
		Dictionary,
	};

	enum class TryConvertResult {
		NoAction,
		NoValue,
		Value,
	};

	enum class MatchCase { Yes, No, No_Prelowered };

	class DLL_PUBLIC HookCommon {
	public:
		constexpr HookCommon(std::string_view n, std::string_view nl, const std::string * ns) :
			name(n), nameLower(nl), nameStr(ns)
		{
		}

		virtual ~HookCommon() = default;
		SPECIAL_MEMBERS_DEFAULT(HookCommon);

		[[nodiscard]] bool filter(const HookFilter & flt) const;
		void apply(const ChildHandler & ch, ModelPartParam modelPart) const;

		[[nodiscard]] virtual const Metadata & GetMetadata() const = 0;

		std::string_view name;
		std::string_view nameLower;
		const std::string * nameStr;
	};

	class DLL_PUBLIC ModelPart {
	public:
		ModelPart() = default;
		virtual ~ModelPart() = default;

		SPECIAL_MEMBERS_DEFAULT(ModelPart);

		template<typename MP> static void Make(typename MP::element_type * t, const ModelPartHandler &);
		template<typename T> static void CreateFor(T * t, const ModelPartHandler &);
		template<typename T> static void OnRootFor(T & t, const ModelPartRootHandler &);

		virtual void OnEachChild(const ChildHandler &);
		virtual bool OnAnonChild(const SubPartHandler &, const HookFilter & = HookFilter());
		virtual bool OnChild(const SubPartHandler &, std::string_view memberName, const HookFilter & = HookFilter(),
				MatchCase matchCase = MatchCase::Yes);
		virtual void OnSubclass(const ModelPartHandler &, const std::string &);
		[[nodiscard]] virtual TypeId GetTypeId() const;
		[[nodiscard]] virtual std::optional<std::string> GetTypeIdProperty() const;
		[[nodiscard]] virtual ModelPartType GetType() const = 0;
		virtual void Create();
		virtual void Complete();
		virtual void SetValue(ValueSource &&);
		virtual bool GetValue(ValueTarget &&);
		[[nodiscard]] virtual bool HasValue() const = 0;
		[[nodiscard]] virtual const Metadata & GetMetadata() const;
		[[nodiscard]] virtual bool IsOptional() const;
		virtual void OnContained(const ModelPartHandler &);

	protected:
		static std::string demangle(const char * mangled);
	};

	template<typename T> class DLL_PUBLIC ModelPartModel {
	public:
		explicit ModelPartModel(T * m = nullptr) : Model(m) { }

		T * Model;
	};

	class DLL_PUBLIC ModelPartForRootBase : public ModelPart {
	public:
		explicit ModelPartForRootBase(ModelPartParam mp);

		[[nodiscard]] virtual const std::string & GetRootName() const = 0;
		bool OnAnonChild(const SubPartHandler &, const HookFilter &) override;
		bool OnChild(const SubPartHandler &, std::string_view name, const HookFilter &,
				MatchCase matchCase = MatchCase::Yes) override;
		void OnEachChild(const ChildHandler & ch) override;
		[[nodiscard]] ModelPartType GetType() const override;
		[[nodiscard]] bool IsOptional() const override;
		virtual void Write(::Ice::OutputStream &) const = 0;
		virtual void Read(::Ice::InputStream &) = 0;
		void OnContained(const ModelPartHandler &) override;

		ModelPartParam mp;

	protected:
		[[noreturn]] static void throwLocalTypeException(const std::type_info &);
	};
}
