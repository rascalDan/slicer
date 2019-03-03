#ifndef SLICER_MODELPARTSTYPES_H
#define SLICER_MODELPARTSTYPES_H

#include "modelParts.h"

namespace Slicer {
	template<typename T>
	struct isLocal {
		static constexpr bool value = false;
	};

	DLL_PUBLIC bool optionalCaseEq(const std::string & a, const std::string & b, bool matchCase);

	template<typename T>
	class DLL_PUBLIC ModelPartForRoot : public ModelPartForRootBase {
		public:
			ModelPartForRoot(T * o);

			const std::string & GetRootName() const override;
			virtual bool HasValue() const override;
			void Write(::Ice::OutputStream &) const override;
			void Read(::Ice::InputStream &) override;

			static const std::string rootName;

		private:
			T * ModelObject;
	};

	class DLL_PUBLIC ModelPartForSimpleBase : public ModelPart {
		public:
			virtual void OnEachChild(const ChildHandler &) override;
			virtual ChildRef GetAnonChildRef(const HookFilter &) override;
			virtual ChildRef GetChildRef(const std::string &, const HookFilter &, bool matchCase = true) override;
			virtual bool HasValue() const override;
			virtual ModelPartType GetType() const override;
			static const ModelPartType type;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForSimple : public ModelPartForSimpleBase, protected ModelPartModel<T> {
		public:
			typedef T element_type;

			ModelPartForSimple(T * h);

			virtual void SetValue(ValueSource && s) override;
			virtual bool GetValue(ValueTarget && s) override;
	};

	class DLL_PUBLIC ModelPartForConvertedBase : public ModelPart {
		public:
			virtual void OnEachChild(const ChildHandler &) override;
			virtual ChildRef GetAnonChildRef(const HookFilter &) override;
			virtual ChildRef GetChildRef(const std::string &, const HookFilter &, bool matchCase = true) override;
			virtual bool HasValue() const override;
			virtual ModelPartType GetType() const override;
			static const ModelPartType type;

		protected:
			template<typename ET, typename MT, typename Conv>
			inline static bool tryConvertFrom(ValueSource & vsp, MT * model, const Conv & conv);
			template<typename ET, typename MT>
			inline static bool tryConvertFrom(ValueSource & vsp, MT * model);
			template<typename ET, typename MT, typename Conv>
			inline static TryConvertResult tryConvertTo(ValueTarget & vsp, const MT * model, const Conv & conv);
			template<typename ET, typename MT>
			inline static TryConvertResult tryConvertTo(ValueTarget & vsp, const MT * model);
	};

	template<typename T, typename M, T M::* MV>
	class DLL_PUBLIC ModelPartForConverted : public ModelPartForConvertedBase, protected ModelPartModel<T> {
		public:
			typedef T element_type;

			ModelPartForConverted(T * h);

			virtual void SetValue(ValueSource && s) override;
			virtual bool GetValue(ValueTarget && s) override;
	};

	template<typename T, typename M, Ice::optional<T> M::* MV>
	class DLL_PUBLIC ModelPartForConverted<Ice::optional<T>, M, MV> : public ModelPartForConvertedBase, protected ModelPartModel<Ice::optional<T>> {
		public:
			typedef Ice::optional<T> element_type;

			ModelPartForConverted(Ice::optional<T> * h);

			virtual void SetValue(ValueSource && s) override;
			virtual bool GetValue(ValueTarget && s) override;
			virtual bool HasValue() const override;
	};

	class DLL_PUBLIC ModelPartForOptionalBase : public ModelPart {
		public:
			virtual void OnEachChild(const ChildHandler & ch) override;
			virtual void Complete() override;
			virtual ChildRef GetAnonChildRef(const HookFilter & flt) override;
			virtual ChildRef GetChildRef(const std::string & name, const HookFilter & flt, bool matchCase = true) override;
			virtual void SetValue(ValueSource && s) override;
			virtual bool HasValue() const override;
			virtual bool IsOptional() const override;
			virtual const Metadata & GetMetadata() const override;

		protected:
			virtual bool hasModel() const = 0;
			ModelPartPtr modelPart;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForOptional : public ModelPartForOptionalBase, protected ModelPartModel<Ice::optional<typename T::element_type> > {
		public:
			ModelPartForOptional(Ice::optional< typename T::element_type > * h);
			virtual void Create() override;
			virtual bool GetValue(ValueTarget && s) override;
			virtual ModelPartType GetType() const override;

		protected:
			virtual bool hasModel() const override;
	};

	class DLL_PUBLIC ModelPartForComplexBase : public ModelPart {
		public:
			virtual ModelPartType GetType() const override;
			static const ModelPartType type;

		protected:
			ModelPartPtr getSubclassModelPart(const std::string & name, void * m);

			static void registerClass(const std::string & className, const std::string * typeName, const ClassRef &);
			static void unregisterClass(const std::string & className, const std::string * typeName);
			static TypeId GetTypeId(const std::string & id, const std::string & className);
			static std::string demangle(const char * mangled);

			static const std::string & ToExchangeTypeName(const std::string &);
			static const std::string & ToModelTypeName(const std::string &);
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForComplex : public ModelPartForComplexBase {
		public:
			class DLL_PRIVATE HookBase;
			typedef std::unique_ptr<HookBase> HookPtr;

			template <typename MT, typename MP>
			class DLL_PRIVATE Hook;

			template <typename MT, typename MP>
			class DLL_PRIVATE HookMetadata;

			virtual void OnEachChild(const ChildHandler & ch) override;

			virtual ChildRef GetAnonChildRef(const HookFilter & flt) override;
			ChildRef GetChildRef(const std::string & name, const HookFilter & flt, bool matchCase = true) override;

			virtual const Metadata & GetMetadata() const override;

			virtual T * GetModel() = 0;

		protected:
			template<typename R>
			DLL_PRIVATE ChildRef GetChildRefFromRange(const R & range, const HookFilter & flt);

			class DLL_PRIVATE Hooks;

			template<typename H, typename ... P>
			static void addHook(Hooks &, const P & ...);

			static const Hooks hooks;
			static const Metadata metadata;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForClass : public ModelPartForComplex<T>, protected ModelPartModel<std::shared_ptr<T> > {
		public:
			typedef std::shared_ptr<T> element_type;

			ModelPartForClass(element_type * h);

			virtual void Create() override;

			T * GetModel() override;

			virtual ModelPartPtr GetSubclassModelPart(const std::string & name) override;

			virtual bool HasValue() const override;

			virtual TypeId GetTypeId() const override;
			template<typename dummy = T>
			const std::string & getTypeId(typename std::enable_if<std::is_base_of<Ice::Object, dummy>::value>::type * = nullptr) const;
			template<typename dummy = T>
			std::string getTypeId(typename std::enable_if<!std::is_base_of<Ice::Object, dummy>::value>::type * = nullptr) const;

			virtual Ice::optional<std::string> GetTypeIdProperty() const override;

			static const std::string typeIdProperty;
			static const std::string * className;
			static const std::string * typeName;

			static ModelPartPtr CreateModelPart(void *);

		private:
			static void initClassName();
			static void deleteClassName();
			static void registerClass() __attribute__ ((constructor(210)));
			static void unregisterClass() __attribute__ ((destructor(210)));
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForStruct : public ModelPartForComplex<T>, protected ModelPartModel<T> {
		public:
			typedef T element_type;

			ModelPartForStruct(T * o);

			T * GetModel() override;

			virtual bool HasValue() const override;
	};

	class DLL_PUBLIC ModelPartForEnumBase : public ModelPart {
		public:
			virtual void OnEachChild(const ChildHandler &) override;
			ChildRef GetAnonChildRef(const HookFilter &) override;
			ChildRef GetChildRef(const std::string &, const HookFilter &, bool matchCase = true) override;
			virtual bool HasValue() const override;
			virtual ModelPartType GetType() const override;
			static const ModelPartType type;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForEnum : public ModelPartForEnumBase, protected ModelPartModel<T> {
		public:
			typedef T element_type;
			class DLL_PRIVATE Enumerations;

			ModelPartForEnum(T * s);

			virtual const Metadata & GetMetadata() const override;

			virtual void SetValue(ValueSource && s) override;

			virtual bool GetValue(ValueTarget && s) override;

			static const Metadata metadata;
			static const Enumerations enumerations;
			DLL_PUBLIC static const std::string & lookup(T);
			DLL_PUBLIC static T lookup(const std::string_view &);
	};

	class DLL_PUBLIC ModelPartForSequenceBase : public ModelPart {
		public:
			virtual bool HasValue() const override;
			virtual ModelPartType GetType() const override;
			static const ModelPartType type;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForSequence : public ModelPartForSequenceBase, protected ModelPartModel<T> {
		public:
			typedef T element_type;

			ModelPartForSequence(T * s);

			virtual void OnEachChild(const ChildHandler & ch) override;

			ChildRef GetAnonChildRef(const HookFilter &) override;

			ChildRef GetChildRef(const std::string &, const HookFilter &, bool matchCase = true) override;

			virtual const Metadata & GetMetadata() const override;

			virtual ModelPartPtr GetContainedModelPart() override;

			static const Metadata metadata;
			static const std::string elementName;

		private:
			ModelPartPtr elementModelPart(typename T::value_type &) const;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForDictionaryElementInserter : public ModelPartForStruct<typename T::value_type> {
		public:
			ModelPartForDictionaryElementInserter(T * d);

			virtual void Complete() override;

			typename T::value_type value;

		private:
			T * dictionary;
	};

	class DLL_PUBLIC ModelPartForDictionaryBase : public ModelPart {
		public:
			virtual bool HasValue() const override;
			virtual ModelPartType GetType() const override;
			static const ModelPartType type;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForDictionary : public ModelPartForDictionaryBase, protected ModelPartModel<T> {
		public:
			typedef T element_type;

			ModelPartForDictionary(T * d);

			virtual void OnEachChild(const ChildHandler & ch) override;

			ChildRef GetAnonChildRef(const HookFilter &) override;

			ChildRef GetChildRef(const std::string & name, const HookFilter &, bool matchCase = true) override;

			virtual const Metadata & GetMetadata() const override;

			virtual ModelPartPtr GetContainedModelPart() override;

			static const Metadata metadata;
			static const std::string pairName;
	};

	template<typename T>
	class DLL_PUBLIC Stream {
		public:
			typedef std::function<void(const T &)> Consumer;
			typedef T element_type;

			virtual void Produce(const Consumer & c) = 0;
	};

	class DLL_PUBLIC ModelPartForStreamBase : public ModelPart {
		public:
			virtual ModelPartType GetType() const override;
			virtual bool HasValue() const override;
			virtual ChildRef GetAnonChildRef(const HookFilter &) override;
			virtual ChildRef GetChildRef(const std::string &, const HookFilter &, bool matchCase = true) override;

			virtual ModelPartPtr GetContainedModelPart() override = 0;
			virtual void OnEachChild(const ChildHandler & ch) override = 0;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForStream : public ModelPartForStreamBase, ModelPartModel<Stream<T>> {
		public:
			ModelPartForStream(Stream<T> * s);

			virtual ModelPartPtr GetContainedModelPart() override;
			virtual void OnEachChild(const ChildHandler & ch) override;
	};

	class DLL_PUBLIC ModelPartForStreamRootBase : public ModelPartForRootBase {
		public:
			ModelPartForStreamRootBase(const ModelPartPtr & mp);

			virtual void Write(Ice::OutputStream&) const override;
			virtual void Read(Ice::InputStream&) override;
			virtual bool HasValue() const override;
			virtual void OnEachChild(const ChildHandler & ch) override;
			virtual const std::string & GetRootName() const override = 0;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForStreamRoot : public ModelPartForStreamRootBase {
		public:
			ModelPartForStreamRoot(Stream<T> * s);

			virtual const std::string & GetRootName() const override;

		private:
			Stream<T> * stream;
	};
}

#endif


