#pragma once

#include "modelParts.h"
#include <Ice/Optional.h>
#include <c++11Helpers.h>
#include <cstddef>
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
	template<typename T> struct isLocal {
		static constexpr bool value = false;
	};

	template<typename T> struct isLocal<::Ice::optional<T>> {
		static constexpr bool value = isLocal<T>::value;
	};

	template<typename T> struct isOptional {
		static constexpr bool value = false;
	};

	template<typename T> struct isOptional<::Ice::optional<T>> {
		static constexpr bool value = true;
	};

	DLL_PUBLIC bool optionalCaseEq(std::string_view a, std::string_view b, bool matchCase);

	template<typename T> class ModelPartForRoot : public ModelPartForRootBase {
	public:
		explicit ModelPartForRoot(T * o, ModelPartParam mp);

		[[nodiscard]] const std::string & GetRootName() const override;
		[[nodiscard]] bool HasValue() const override;
		void Write(::Ice::OutputStream &) const override;
		void Read(::Ice::InputStream &) override;

		static const std::string rootName;

	private:
		T * ModelObject;
	};

	class DLL_PUBLIC ModelPartForSimpleBase : public ModelPart {
	public:
		[[nodiscard]] bool HasValue() const override;
		[[nodiscard]] ModelPartType GetType() const override;
		static const ModelPartType type;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForSimple : public ModelPartForSimpleBase, protected ModelPartModel<T> {
	public:
		using element_type = T;

		using ModelPartModel<T>::ModelPartModel;

		void SetValue(ValueSource && s) override;
		bool GetValue(ValueTarget && s) override;
	};

	class DLL_PUBLIC ModelPartForConvertedBase : public ModelPart {
	public:
		[[nodiscard]] bool HasValue() const override;
		[[nodiscard]] ModelPartType GetType() const override;
		static const ModelPartType type;

	protected:
		[[noreturn]] void conversion_fail(std::string_view typeName);
		template<typename ET, typename MT, typename Conv>
		inline static bool tryConvertFrom(const ValueSource & vsp, MT * model, const Conv & conv);
		template<typename ET, typename MT> inline static bool tryConvertFrom(const ValueSource & vsp, MT * model);
		template<typename ET, typename MT, typename Conv>
		inline static TryConvertResult tryConvertTo(const ValueTarget & vsp, const MT * model, const Conv & conv);
		template<typename ET, typename MT>
		inline static TryConvertResult tryConvertTo(const ValueTarget & vsp, const MT * model);
	};

	template<typename T, typename M, T M::*MV>
	class ModelPartForConverted : public ModelPartForConvertedBase, protected ModelPartModel<T> {
	public:
		using element_type = T;

		using ModelPartModel<T>::ModelPartModel;

		void SetValue(ValueSource && s) override;
		bool GetValue(ValueTarget && s) override;
	};

	template<typename T, typename M, Ice::optional<T> M::*MV>
	class ModelPartForConverted<Ice::optional<T>, M, MV> :
		public ModelPartForConvertedBase,
		protected ModelPartModel<Ice::optional<T>> {
	public:
		using element_type = Ice::optional<T>;

		using ModelPartModel<Ice::optional<T>>::ModelPartModel;

		void SetValue(ValueSource && s) override;
		bool GetValue(ValueTarget && s) override;
		[[nodiscard]] bool HasValue() const override;
	};

	class DLL_PUBLIC ModelPartForOptionalBase : public ModelPart {
	public:
		void OnEachChild(const ChildHandler & ch) override;
		void Complete() override;
		bool OnAnonChild(const SubPartHandler &, const HookFilter & flt) override;
		bool OnChild(const SubPartHandler &, std::string_view name, const HookFilter & flt,
				MatchCase matchCase = MatchCase::Yes) override;
		void SetValue(ValueSource && s) override;
		bool GetValue(ValueTarget && s) override;
		[[nodiscard]] bool HasValue() const override;
		[[nodiscard]] bool IsOptional() const override;
		[[nodiscard]] const Metadata & GetMetadata() const override;

	protected:
		[[nodiscard]] virtual bool hasModel() const = 0;
		ModelPart * modelPart;
	};

	template<typename T>
	class ModelPartForOptional :
		public ModelPartForOptionalBase,
		protected ModelPartModel<Ice::optional<typename T::element_type>> {
	public:
		using element_type = Ice::optional<typename T::element_type>;
		explicit ModelPartForOptional(element_type * h);
		void Create() override;
		[[nodiscard]] ModelPartType GetType() const override;

	protected:
		[[nodiscard]] bool hasModel() const override;
		std::optional<T> modelPartOwner;
	};

	class DLL_PUBLIC ModelPartForComplexBase : public ModelPart {
	public:
		[[nodiscard]] ModelPartType GetType() const override;
		static const ModelPartType type;

	protected:
		const ClassRefBase * getSubclassRef(const std::string & name);

		static void registerClass(
				const std::string_view className, const std::optional<std::string_view> typeName, const ClassRefBase *);
		static void unregisterClass(const std::string_view className, const std::optional<std::string_view> typeName);
		static TypeId getTypeId(const std::string & id, const std::string_view className);

		static const std::string & ToExchangeTypeName(const std::string &);
		static std::string_view ToModelTypeName(const std::string &);
		[[noreturn]] static void throwIncorrectType(const std::string & name, const std::type_info & target);
		[[noreturn]] static void throwAbstractClassException(const std::type_info & target);
	};

	template<typename T> class Hooks;

	template<typename T> class ModelPartForComplex : public ModelPartForComplexBase {
	public:
		class HookBase;

		template<typename MT, typename MP, std::size_t = 0> class Hook;

		void OnEachChild(const ChildHandler & ch) override;

		bool OnAnonChild(const SubPartHandler &, const HookFilter & flt) override;
		bool OnChild(const SubPartHandler &, std::string_view name, const HookFilter & flt,
				MatchCase matchCase = MatchCase::Yes) override;

		[[nodiscard]] const Metadata & GetMetadata() const override;

		virtual T * GetModel() = 0;

	protected:
		template<typename R> bool OnChildFromRange(const SubPartHandler &, const R & range, const HookFilter & flt);

		static const Hooks<T> & hooks();
	};

	template<typename T>
	class ModelPartForClass : public ModelPartForComplex<T>, protected ModelPartModel<std::shared_ptr<T>> {
	public:
		using element_type = std::shared_ptr<T>;

		using ModelPartModel<element_type>::ModelPartModel;

		void Create() override;

		T * GetModel() override;

		void OnSubclass(const ModelPartHandler &, const std::string & name) override;

		[[nodiscard]] bool HasValue() const override;

		[[nodiscard]] TypeId GetTypeId() const override;

		[[nodiscard]] std::optional<std::string> GetTypeIdProperty() const override;

		static const std::string typeIdProperty;
		constinit static const std::string_view className;
		constinit static const std::optional<const std::string_view> typeName;

	private:
		static const ClassRefBase * const classref;
		static void registerClass() __attribute__((constructor(210)));
		static void unregisterClass() __attribute__((destructor(210)));
	};

	template<typename T> struct ClassRef {
		consteval ClassRef() = default;
		virtual ~ClassRef() = default;
		SPECIAL_MEMBERS_DEFAULT(ClassRef);

		virtual void onSubClass(std::shared_ptr<T> &, const ModelPartHandler &) const = 0;
	};

	template<typename T> class ModelPartForStruct : public ModelPartForComplex<T>, protected ModelPartModel<T> {
	public:
		using element_type = T;

		using ModelPartModel<T>::ModelPartModel;

		T * GetModel() override;

		[[nodiscard]] bool HasValue() const override;
	};

	class DLL_PUBLIC ModelPartForEnumBase : public ModelPart {
	public:
		[[nodiscard]] bool HasValue() const override;
		[[nodiscard]] ModelPartType GetType() const override;
		static const ModelPartType type;
	};
	template<typename T> class EnumMap;

	template<typename T> class ModelPartForEnum : public ModelPartForEnumBase, protected ModelPartModel<T> {
	public:
		using element_type = T;

		using ModelPartModel<T>::ModelPartModel;

		[[nodiscard]] const Metadata & GetMetadata() const override;

		void SetValue(ValueSource && s) override;

		bool GetValue(ValueTarget && s) override;

		static const Metadata metadata;
		static constexpr const EnumMap<T> & enumerations();
		DLL_PUBLIC static const std::string & lookup(T);
		DLL_PUBLIC static T lookup(std::string_view);
	};

	class DLL_PUBLIC ModelPartForSequenceBase : public ModelPart {
	public:
		[[nodiscard]] bool HasValue() const override;
		[[nodiscard]] ModelPartType GetType() const override;
		bool OnChild(const SubPartHandler &, std::string_view, const HookFilter &,
				MatchCase matchCase = MatchCase::Yes) override;
		[[nodiscard]] virtual const std::string & GetElementName() const = 0;

		static const ModelPartType type;
	};

	template<typename T> class ModelPartForSequence : public ModelPartForSequenceBase, protected ModelPartModel<T> {
	public:
		using element_type = T;

		using ModelPartModel<T>::ModelPartModel;

		void OnEachChild(const ChildHandler & ch) override;

		bool OnAnonChild(const SubPartHandler &, const HookFilter &) override;

		[[nodiscard]] const std::string & GetElementName() const override;

		[[nodiscard]] const Metadata & GetMetadata() const override;

		void OnContained(const ModelPartHandler &) override;

		static const Metadata metadata;
		static const std::string elementName;
	};

	template<typename T>
	class ModelPartForDictionaryElementInserter : public ModelPartForStruct<typename T::value_type> {
	public:
		explicit ModelPartForDictionaryElementInserter(T * d);

		void Complete() override;

		typename T::value_type value;

	private:
		T * dictionary;
	};

	class DLL_PUBLIC ModelPartForDictionaryBase : public ModelPart {
	public:
		[[nodiscard]] bool HasValue() const override;
		[[nodiscard]] ModelPartType GetType() const override;
		static const ModelPartType type;

	protected:
		[[noreturn]] static void throwIncorrectElementName(const std::string_view);
	};

	template<typename T> class ModelPartForDictionary : public ModelPartForDictionaryBase, protected ModelPartModel<T> {
	public:
		using element_type = T;

		using ModelPartModel<T>::ModelPartModel;

		void OnEachChild(const ChildHandler & ch) override;

		bool OnAnonChild(const SubPartHandler &, const HookFilter &) override;

		bool OnChild(const SubPartHandler &, std::string_view name, const HookFilter &,
				MatchCase matchCase = MatchCase::Yes) override;

		[[nodiscard]] const Metadata & GetMetadata() const override;

		void OnContained(const ModelPartHandler &) override;

		static const Metadata metadata;
		static const std::string pairName;
	};

	template<typename T> class Stream {
	public:
		using Consumer = std::function<void(const T &)>;
		using element_type = T;

		constexpr Stream() = default;
		virtual ~Stream() = default;
		SPECIAL_MEMBERS_DEFAULT(Stream);

		virtual void Produce(const Consumer & c) = 0;
	};

	class DLL_PUBLIC ModelPartForStreamBase : public ModelPart {
	public:
		[[nodiscard]] ModelPartType GetType() const override;
		[[nodiscard]] bool HasValue() const override;

		void OnContained(const ModelPartHandler &) override = 0;
		void OnEachChild(const ChildHandler & ch) override = 0;
	};

	template<typename T> class ModelPartForStream : public ModelPartForStreamBase, ModelPartModel<Stream<T>> {
	public:
		using ModelPartModel<Stream<T>>::ModelPartModel;

		void OnContained(const ModelPartHandler &) override;
		void OnEachChild(const ChildHandler & ch) override;
	};

	class DLL_PUBLIC ModelPartForStreamRootBase : public ModelPartForRootBase {
	public:
		using ModelPartForRootBase::ModelPartForRootBase;

		void Write(Ice::OutputStream &) const override;
		void Read(Ice::InputStream &) override;
		[[nodiscard]] bool HasValue() const override;
		void OnEachChild(const ChildHandler & ch) override;
		[[nodiscard]] const std::string & GetRootName() const override = 0;
	};

	template<typename T> class ModelPartForStreamRoot : public ModelPartForStreamRootBase {
	public:
		using ModelPartForStreamRootBase::ModelPartForStreamRootBase;

		[[nodiscard]] const std::string & GetRootName() const override;
	};
}
