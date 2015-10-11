#ifndef SLICER_MODELPARTS_H
#define SLICER_MODELPARTS_H

#include <Ice/Config.h>
#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>
#include <IceUtil/Optional.h>
#include <Ice/Handle.h>
#include <Slice/Parser.h>
#include <stdexcept>
#include <boost/function.hpp>
#include <boost/bimap.hpp>
#include <vector>
#include <visibility.h>

namespace Slicer {
	// This allows IceUtil::Handle to play nicely with boost::things
	template <class T>
	T *
	get_pointer(const IceUtil::Handle<T> & p)
	{
		return p.get();
	}

	class DLL_PUBLIC IncorrectElementName : public std::invalid_argument {
		public:
			IncorrectElementName(const std::string & n);
	};

	class DLL_PUBLIC UnknownType : public std::invalid_argument {
		public:
			UnknownType(const std::string & n);
	};

	class DLL_PUBLIC InvalidEnumerationValue : public std::invalid_argument {
		public:
			InvalidEnumerationValue(const std::string & n, const std::string & e);
			InvalidEnumerationValue(::Ice::Int n, const std::string & e);
	};

	template <typename T>
	class TValueTarget {
		public:
			virtual void get(const T &) const = 0;
	};
	class ValueTarget : public IceUtil::Shared,
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
	typedef IceUtil::Handle<ValueTarget> ValueTargetPtr;

	template <typename T>
	class TValueSource {
		public:
			virtual void set(T &) const = 0;
	};
	class ValueSource : public IceUtil::Shared,
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
	typedef IceUtil::Handle<ValueSource> ValueSourcePtr;

	class ModelPart;
	class HookCommon;

	typedef IceUtil::Handle<ModelPart> ModelPartPtr;
	typedef IceUtil::Handle<HookCommon> HookCommonPtr;
	typedef IceUtil::Optional<std::string> TypeId;

	typedef boost::function<void(const std::string &, ModelPartPtr, HookCommonPtr)> ChildHandler;

	typedef boost::function<ModelPartPtr(void *)> ClassRef;
	typedef boost::function<bool(HookCommonPtr)> HookFilter;
	typedef std::map<std::string, ClassRef> ClassRefMap;
	DLL_PUBLIC ClassRefMap * & classRefMap();
	typedef boost::bimap<std::string, std::string> ClassNameMap;
	DLL_PUBLIC ClassNameMap * & classNameMap();
	typedef std::list<std::string> Metadata;
	enum ModelPartType {
		mpt_Null,
		mpt_Simple,
		mpt_Complex,
		mpt_Sequence,
		mpt_Dictionary,
	};

	class HookCommon : public IceUtil::Shared {
		public:
			virtual std::string PartName() const = 0;

			virtual const Metadata & GetMetadata() const = 0;
	};

#define templateMODELPARTFOR(Type) \
	template <class T> ModelPartPtr DLL_PUBLIC ModelPartFor(Type & t); \
	template <class T> ModelPartPtr DLL_PUBLIC ModelPartFor(Type * t);
#define MODELPARTFOR(Type) \
	ModelPartPtr DLL_PUBLIC ModelPartFor(Type & t); \
	ModelPartPtr DLL_PUBLIC ModelPartFor(Type * t);
	templateMODELPARTFOR(IceInternal::Handle<T>);
	templateMODELPARTFOR(std::vector<T>);
	templateMODELPARTFOR(std::list<T>);
	template <class K, class V> ModelPartPtr DLL_PUBLIC ModelPartFor(std::map<K, V> & t);
	template <class K, class V> ModelPartPtr DLL_PUBLIC ModelPartFor(std::map<K, V> * t);
	MODELPARTFOR(std::string);
	MODELPARTFOR(bool);
	MODELPARTFOR(Ice::Float);
	MODELPARTFOR(Ice::Double);
	MODELPARTFOR(Ice::Byte);
	MODELPARTFOR(Ice::Short);
	MODELPARTFOR(Ice::Int);
	MODELPARTFOR(Ice::Long);
	templateMODELPARTFOR(T);
#undef templateMODELPARTFOR
#undef MODELPARTFOR

	class ChildRef : public IceUtil::Shared {
		public:
			virtual ModelPartPtr Child() const = 0;
			virtual const Metadata & ChildMetaData() const = 0;
	};
	typedef IceUtil::Handle<ChildRef> ChildRefPtr;
	class DLL_PUBLIC ImplicitChildRef : public ChildRef {
		public:
			ImplicitChildRef(ModelPartPtr);

			ModelPartPtr Child() const;
			const Metadata & ChildMetaData() const;

		private:
			ModelPartPtr mpp;
	};
	class DLL_PUBLIC MemberChildRef : public ChildRef {
		public:
			MemberChildRef(ModelPartPtr, const Metadata &);

			ModelPartPtr Child() const;
			const Metadata & ChildMetaData() const;

		private:
			ModelPartPtr mpp;
			const Metadata & mdr;
	};

	class DLL_PUBLIC ModelPart : public IceUtil::Shared {
		public:
			virtual ~ModelPart() = default;

			virtual void OnEachChild(const ChildHandler &) = 0;
			ModelPartPtr GetAnonChild(const HookFilter & = HookFilter());
			ModelPartPtr GetChild(const std::string & memberName, const HookFilter & = HookFilter());
			virtual ChildRefPtr GetAnonChildRef(const HookFilter & = HookFilter()) = 0;
			virtual ChildRefPtr GetChildRef(const std::string & memberName, const HookFilter & = HookFilter()) = 0;
			virtual ModelPartPtr GetSubclassModelPart(const std::string &);
			virtual TypeId GetTypeId() const;
			virtual IceUtil::Optional<std::string> GetTypeIdProperty() const;
			virtual ModelPartType GetType() const = 0;
			virtual void Create();
			virtual void Complete();
			virtual void SetValue(ValueSourcePtr);
			virtual void GetValue(ValueTargetPtr);
			virtual bool HasValue() const = 0;
			virtual const Metadata & GetMetadata() const;

			static const std::string & ToExchangeTypeName(const std::string &);
			static const std::string & ToModelTypeName(const std::string &);
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForSimple : public ModelPart {
		public:
			typedef T element_type;

			ModelPartForSimple(T & h) :
				Member(h)
			{
			}
			ModelPartForSimple(T * h) :
				Member(*h)
			{
			}
			virtual void OnEachChild(const ChildHandler &) { }
			virtual ChildRefPtr GetAnonChildRef(const HookFilter &) override { return NULL; }
			virtual ChildRefPtr GetChildRef(const std::string &, const HookFilter &) override { return NULL; }
			virtual void SetValue(ValueSourcePtr s) override { s->set(Member); }
			virtual void GetValue(ValueTargetPtr s) override { s->get(Member); }
			virtual bool HasValue() const override { return true; }
			virtual ModelPartType GetType() const { return mpt_Simple; }

		private:
			T & Member;
	};

	template<typename T, typename M, T M::* MV>
	class DLL_PUBLIC ModelPartForConverted : public ModelPart {
		public:
			typedef T element_type;

			ModelPartForConverted(T & h) :
				Member(h)
			{
			}
			ModelPartForConverted(T * h) :
				Member(*h)
			{
			}
			virtual void OnEachChild(const ChildHandler &) { }
			virtual ChildRefPtr GetAnonChildRef(const HookFilter &) override { return NULL; }
			virtual ChildRefPtr GetChildRef(const std::string &, const HookFilter &) override { return NULL; }
			virtual void SetValue(ValueSourcePtr s) override;
			virtual void GetValue(ValueTargetPtr s) override;
			virtual bool HasValue() const override { return true; }
			virtual ModelPartType GetType() const { return mpt_Simple; }

		private:
			T & Member;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForOptional : public ModelPart {
		public:
			ModelPartForOptional(IceUtil::Optional< typename T::element_type > & h) :
				OptionalMember(h)
			{
				if (OptionalMember) {
					modelPart = new T(*OptionalMember);
				}
			}
			ModelPartForOptional(IceUtil::Optional< typename T::element_type > * h) :
				OptionalMember(*h)
			{
				if (OptionalMember) {
					modelPart = new T(*OptionalMember);
				}
			}
			virtual void OnEachChild(const ChildHandler & ch) override
			{
				if (OptionalMember) {
					modelPart->OnEachChild(ch);
				}
			}
			virtual void Complete() override
			{
				if (OptionalMember) {
					modelPart->Complete();
				}
			}
			virtual void Create() override
			{
				if (!OptionalMember) {
					OptionalMember = typename T::element_type();
					modelPart = new T(*OptionalMember);
					modelPart->Create();
				}
			}
			virtual ChildRefPtr GetAnonChildRef(const HookFilter & flt) override
			{
				if (OptionalMember) {
					return modelPart->GetAnonChildRef(flt);
				}
				return NULL;
			}
			virtual ChildRefPtr GetChildRef(const std::string & name, const HookFilter & flt) override
			{
				if (OptionalMember) {
					return modelPart->GetChildRef(name, flt);
				}
				return NULL;
			}
			virtual void SetValue(ValueSourcePtr s) override
			{
				if (OptionalMember) {
					modelPart->SetValue(s);
				}
			}
			virtual void GetValue(ValueTargetPtr s) override
			{
				if (!OptionalMember) {
					OptionalMember = typename T::element_type();
					modelPart = new T(*OptionalMember);
				}
				modelPart->GetValue(s);
			}

			virtual bool HasValue() const override { return OptionalMember; }

			virtual ModelPartType GetType() const
			{
				if (HasValue()) {
					return modelPart->GetType();
				}
				return mpt_Null;
			}
			virtual const Metadata & GetMetadata() const override { return modelPart->GetMetadata(); }

		private:
			IceUtil::Optional< typename T::element_type > & OptionalMember;
			ModelPartPtr modelPart;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForComplex : public ModelPart {
		public:
			class HookBase : public HookCommon {
				public:
					virtual ModelPartPtr Get(T * t) const = 0;
			};
			typedef IceUtil::Handle<HookBase> HookPtr;

			template <typename MT, typename CT, MT CT::*M>
			class HookMetadata : public HookBase {
				public:
					virtual const Metadata & GetMetadata() const override { return metadata; }

				private:
					static Metadata metadata;
			};

			template <typename MT, typename CT, MT CT::*M, typename MP>
			class Hook : public HookMetadata<MT, CT, M> {
				public:
					Hook(const std::string & n) :
						name(n)
					{
					}

					ModelPartPtr Get(T * t) const override
					{
						return t ? new MP(t->*M) : NULL;
					}

					std::string PartName() const override
					{
						return name;
					}


				private:
					const std::string name;
			};

			virtual void OnEachChild(const ChildHandler & ch)
			{
				for (const auto & h : hooks) {
					auto modelPart = h->Get(GetModel());
					ch(h->PartName(), modelPart && modelPart->HasValue() ? modelPart : ModelPartPtr(), h);
				}
			}

			virtual ChildRefPtr GetAnonChildRef(const HookFilter & flt) override
			{
				for (const auto & h : hooks) {
					if (!flt || flt(h)) {
						return new MemberChildRef(h->Get(GetModel()), h->GetMetadata());
					}
				}
				return NULL;
			}
			ChildRefPtr GetChildRef(const std::string & name, const HookFilter & flt) override
			{
				for (const auto & h : hooks) {
					if (h->PartName() == name && (!flt || flt(h))) {
						return new MemberChildRef(h->Get(GetModel()), h->GetMetadata());
					}
				}
				return NULL;
			}

			virtual ModelPartType GetType() const { return mpt_Complex; }

			virtual const Metadata & GetMetadata() const override { return metadata; }

			virtual T * GetModel() = 0;

			typedef std::vector<HookPtr> Hooks;

		private:
			static Hooks hooks;
			static Metadata metadata;
	};
	
	template<typename T>
	class DLL_PUBLIC ModelPartForClass : public ModelPartForComplex<typename T::element_type> {
		public:
			typedef T element_type;

			ModelPartForClass(T & h) :
				ModelObject(h)
			{
			}

			ModelPartForClass(T * h) :
				ModelObject(*h)
			{
			}

			virtual void Create() override
			{
				ModelObject = new typename T::element_type();
			}

			typename T::element_type * GetModel() override
			{
				return ModelObject.get();
			}

			virtual ModelPartPtr GetSubclassModelPart(const std::string & name) override
			{
				auto ref = classRefMap()->find(ModelPart::ToModelTypeName(name));
				if (ref == classRefMap()->end()) {
					throw UnknownType(name);
				}
				return ref->second(&this->ModelObject);
			}

			virtual bool HasValue() const override { return ModelObject; }

			virtual TypeId GetTypeId() const override;

			virtual IceUtil::Optional<std::string> GetTypeIdProperty() const override { return typeIdProperty; }

		private:
			T & ModelObject;
			static std::string typeIdProperty;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForStruct : public ModelPartForComplex<T> {
		public:
			typedef T element_type;

			ModelPartForStruct(T & o) :
				ModelObject(o)
			{
			}

			ModelPartForStruct(T * o) :
				ModelObject(*o)
			{
			}

			T * GetModel() override
			{
				return &ModelObject;
			}

			virtual bool HasValue() const override { return true; }

		private:
			T & ModelObject;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForRoot : public ModelPart {
		public:
			ModelPartForRoot() :
				ModelObject(new T()),
				owned(true),
				mp(ModelPartFor(*ModelObject))
			{
			}

			ModelPartForRoot(T & o) :
				ModelObject(&o),
				owned(false),
				mp(ModelPartFor(*ModelObject))
			{
			}

			~ModelPartForRoot()
			{
				if (owned) {
					delete ModelObject;
				}
			}

			virtual ChildRefPtr GetAnonChildRef(const HookFilter &) override
			{
				mp->Create();
				return new ImplicitChildRef(mp);
			}

			virtual ChildRefPtr GetChildRef(const std::string & name, const HookFilter &) override
			{
				if (name != rootName) {
					throw IncorrectElementName(name);
				}
				mp->Create();
				return new ImplicitChildRef(mp);
			}

			virtual void OnEachChild(const ChildHandler & ch) override
			{
				ch(rootName, mp, NULL);
			}

			T & GetModel()
			{
				return *ModelObject;
			}

			virtual bool HasValue() const override { return ModelObject; }

			virtual ModelPartType GetType() const override
			{
				return mp->GetType();
			}

		private:
			T * ModelObject;
			bool owned;
			ModelPartPtr mp;
			DLL_PUBLIC static std::string rootName;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForEnum : public ModelPart {
		public:
			typedef T element_type;
			typedef boost::bimap<T, std::string> Enumerations;

			ModelPartForEnum(T & s) :
				modelPart(s)
			{
			}

			ModelPartForEnum(T * s) :
				modelPart(*s)
			{
			}

			virtual void OnEachChild(const ChildHandler &) override { }

			ChildRefPtr GetAnonChildRef(const HookFilter &) override { return NULL; }

			ChildRefPtr GetChildRef(const std::string &, const HookFilter &) override { return NULL; }

			virtual bool HasValue() const override { return true; }

			virtual ModelPartType GetType() const { return mpt_Simple; }

			virtual const Metadata & GetMetadata() const override { return metadata; }

			virtual void SetValue(ValueSourcePtr s) override;

			virtual void GetValue(ValueTargetPtr s) override;

		private:
			T & modelPart;
			static Metadata metadata;
			static Enumerations enumerations;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForSequence : public ModelPart {
		public:
			typedef T element_type;

			ModelPartForSequence(T & s) :
				sequence(s)
			{
			}

			ModelPartForSequence(T * s) :
				sequence(*s)
			{
			}

			virtual void OnEachChild(const ChildHandler & ch) override
			{
				for(auto & element : sequence) {
					ch(elementName, elementModelPart(element), NULL);
				}
			}

			ChildRefPtr GetAnonChildRef(const HookFilter &) override
			{
				sequence.push_back(typename element_type::value_type());
				return new ImplicitChildRef(ModelPartFor(sequence.back()));
			}

			ChildRefPtr GetChildRef(const std::string &, const HookFilter &) override;

			virtual bool HasValue() const override { return true; }

			virtual ModelPartType GetType() const { return mpt_Sequence; }

			virtual const Metadata & GetMetadata() const override { return metadata; }

		private:
			ModelPartPtr elementModelPart(typename T::value_type &) const;

			T & sequence;
			static std::string elementName;
			static Metadata metadata;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForDictionaryElement : public ModelPartForComplex<ModelPartForDictionaryElement<T> > {
		public:
			ModelPartForDictionaryElement(typename T::key_type * k, typename T::mapped_type * v) :
				key(k),
				value(v)
			{
			}

			ModelPartForDictionaryElement<T> * GetModel() override
			{
				return this;
			}

			virtual bool HasValue() const override { return true; }

			typename T::key_type * key;
			typename T::mapped_type * value;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForDictionaryElementInserter : public ModelPartForDictionaryElement<T> {
		public:
			ModelPartForDictionaryElementInserter(T & d) :
				ModelPartForDictionaryElement<T>(&key, &value),
				dictionary(d)
			{
			}

			virtual void Complete() override { dictionary.insert(typename T::value_type(key, value)); }

			mutable typename T::key_type key;
			mutable typename T::mapped_type value;

		private:
			T & dictionary;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForDictionary : public ModelPart {
		public:
			typedef T element_type;

			ModelPartForDictionary(T & d) :
				dictionary(d)
			{
			}

			ModelPartForDictionary(T * d) :
				dictionary(*d)
			{
			}

			virtual void OnEachChild(const ChildHandler & ch) override
			{
				for (auto & pair : dictionary) {
					ch(pairName, new ModelPartForDictionaryElement<T>(const_cast<typename T::key_type *>(&pair.first), &pair.second), NULL);
				}
			}

			ChildRefPtr GetAnonChildRef(const HookFilter &) override
			{
				return new ImplicitChildRef(new ModelPartForDictionaryElementInserter<T>(dictionary));
			}

			ChildRefPtr GetChildRef(const std::string & name, const HookFilter &) override
			{
				if (name != pairName) {
					throw IncorrectElementName(name);
				}
				return new ImplicitChildRef(new ModelPartForDictionaryElementInserter<T>(dictionary));
			}

			virtual bool HasValue() const override { return true; }

			virtual ModelPartType GetType() const { return mpt_Dictionary; }

			virtual const Metadata & GetMetadata() const override { return metadata; }

		private:
			T & dictionary;
			static std::string pairName;
			static Metadata metadata;
	};
}

#endif

