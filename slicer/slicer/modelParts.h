#ifndef SLICER_MODELPARTS_H
#define SLICER_MODELPARTS_H

#include <Ice/Config.h>
#include <functional>
#include <list>
#include <optional>
#include <stdexcept>
#include <vector>
#include <visibility.h>

namespace Ice {
	class InputStream;
	class OutputStream;
}

namespace Slicer {
	template<typename T> class TValueTarget {
	public:
		virtual void get(const T &) const = 0;
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
		virtual void set(T &) const = 0;
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

	using ModelPartPtr = std::shared_ptr<ModelPart>;
	using ModelPartForRootPtr = std::shared_ptr<ModelPartForRootBase>;
	using TypeId = std::optional<std::string>;
	using ChildHandler = std::function<void(const std::string &, ModelPartPtr, const HookCommon *)>;
	using ClassRef = std::function<ModelPartPtr(void *)>;
	using HookFilter = std::function<bool(const HookCommon *)>;
	using Metadata = std::list<std::string>;
	DLL_PUBLIC extern const Metadata emptyMetadata;

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

	class DLL_PUBLIC ChildRef {
	public:
		explicit ChildRef();
		explicit ChildRef(ModelPartPtr);
		explicit ChildRef(ModelPartPtr, const Metadata &);

		[[nodiscard]] ModelPartPtr Child() const;
		[[nodiscard]] const Metadata & ChildMetaData() const;
		explicit operator bool() const;

	private:
		ModelPartPtr mpp;
		const Metadata & mdr;
	};

	class DLL_PUBLIC HookCommon {
	public:
		constexpr HookCommon(std::string_view n, std::string_view nl, const std::string * ns) :
			name(n), nameLower(nl), nameStr(ns)
		{
		}

		bool filter(const HookFilter & flt) const;
		void apply(const ChildHandler & ch, const ModelPartPtr & modelPart) const;

		[[nodiscard]] virtual const Metadata & GetMetadata() const = 0;

		std::string_view name;
		std::string_view nameLower;
		const std::string * nameStr;
	};

	struct case_less {
		template<typename A, typename B>
		inline bool
		operator()(const A & a, const B & b) const
		{
			const auto cmp = strncasecmp(a.data(), b.data(), std::min(a.length(), b.length()));
			return (cmp < 0) || (!cmp && a.length() < b.length());
		}
	};

	class DLL_PUBLIC ModelPart : public std::enable_shared_from_this<ModelPart> {
	public:
		ModelPart() = default;
		ModelPart(const ModelPart &) = delete;
		ModelPart(ModelPart &&) = delete;

		virtual ~ModelPart() = default;

		ModelPart & operator=(const ModelPart &) = delete;
		ModelPart & operator=(ModelPart &&) = delete;

		template<typename T> static ModelPartPtr CreateFor();
		template<typename T> static ModelPartPtr CreateFor(T & t);
		template<typename T> static ModelPartForRootPtr CreateRootFor(T & t);

		virtual void OnEachChild(const ChildHandler &) = 0;
		ModelPartPtr GetAnonChild(const HookFilter & = HookFilter());
		ModelPartPtr GetChild(const std::string & memberName, const HookFilter & = HookFilter());
		virtual ChildRef GetAnonChildRef(const HookFilter & = HookFilter()) = 0;
		virtual ChildRef GetChildRef(
				const std::string & memberName, const HookFilter & = HookFilter(), bool matchCase = true)
				= 0;
		virtual ModelPartPtr GetSubclassModelPart(const std::string &);
		virtual TypeId GetTypeId() const;
		virtual std::optional<std::string> GetTypeIdProperty() const;
		virtual ModelPartType GetType() const = 0;
		virtual void Create();
		virtual void Complete();
		virtual void SetValue(ValueSource &&);
		virtual bool GetValue(ValueTarget &&);
		virtual bool HasValue() const = 0;
		virtual const Metadata & GetMetadata() const;
		virtual bool IsOptional() const;
		virtual ModelPartPtr GetContainedModelPart();
	};

	template<typename T> class DLL_PUBLIC ModelPartModel {
	protected:
		explicit ModelPartModel() : Model(nullptr) { }
		explicit ModelPartModel(T * m) : Model(m) { }
		T * Model;
	};

	class DLL_PUBLIC ModelPartForRootBase : public ModelPart {
	public:
		explicit ModelPartForRootBase(ModelPartPtr mp);

		virtual const std::string & GetRootName() const = 0;
		ChildRef GetAnonChildRef(const HookFilter &) override;
		ChildRef GetChildRef(const std::string & name, const HookFilter &, bool matchCase = true) override;
		void OnEachChild(const ChildHandler & ch) override;
		ModelPartType GetType() const override;
		bool IsOptional() const override;
		virtual void Write(::Ice::OutputStream &) const = 0;
		virtual void Read(::Ice::InputStream &) = 0;
		ModelPartPtr GetContainedModelPart() override;

		ModelPartPtr mp;
	};
}

#endif
