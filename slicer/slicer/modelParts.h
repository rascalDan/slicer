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
#include <boost/foreach.hpp>
#include <vector>

namespace Slicer {
	class IncorrectElementName : public std::invalid_argument {
		public:
			IncorrectElementName(const std::string & n);
	};

	class UnknownType : public std::invalid_argument {
		public:
			UnknownType(const std::string & n);
	};

	class ValueTarget : public IceUtil::Shared {
		public:
			virtual void get(const bool &) const = 0;
			virtual void get(const Ice::Byte &) const = 0;
			virtual void set(const Ice::Short &) const = 0;
			virtual void get(const Ice::Int &) const = 0;
			virtual void get(const Ice::Long &) const = 0;
			virtual void get(const Ice::Float &) const = 0;
			virtual void get(const Ice::Double &) const = 0;
			virtual void get(const std::string &) const = 0;
	};
	typedef IceUtil::Handle<ValueTarget> ValueTargetPtr;

	class ValueSource : public IceUtil::Shared {
		public:
			virtual void set(bool &) const = 0;
			virtual void set(Ice::Byte &) const = 0;
			virtual void set(Ice::Short &) const = 0;
			virtual void set(Ice::Int &) const = 0;
			virtual void set(Ice::Long &) const = 0;
			virtual void set(Ice::Float &) const = 0;
			virtual void set(Ice::Double &) const = 0;
			virtual void set(std::string &) const = 0;
	};
	typedef IceUtil::Handle<ValueSource> ValueSourcePtr;

	class ModelPart;

	typedef IceUtil::Handle<ModelPart> ModelPartPtr;
	typedef std::map<std::string, ModelPartPtr> ModelParts;
	typedef IceUtil::Optional<std::string> TypeId;

	typedef boost::function<void(const std::string &, ModelPartPtr)> ChildHandler;

	typedef boost::function<ModelPartPtr(void *)> ClassRef;
	typedef std::map<std::string, ClassRef> ClassRefMap;
	ClassRefMap * & classRefMap();
	enum ModelPartType {
		mpt_Null,
		mpt_Simple,
		mpt_Complex,
		mpt_Sequence,
		mpt_Dictionary,
	};

	class ModelPart : public IceUtil::Shared {
		public:
			virtual ~ModelPart() = default;

			virtual void OnEachChild(const ChildHandler &) = 0;
			virtual ModelPartPtr GetChild(const std::string & memberName) = 0;
			virtual ModelPartPtr GetSubclassModelPart(const std::string &);
			virtual TypeId GetTypeId() const;
			virtual ModelPartType GetType() const = 0;
			virtual void Create();
			virtual void Complete();
			virtual void SetValue(ValueSourcePtr);
			virtual void GetValue(ValueTargetPtr);
			virtual bool HasValue() const = 0;
	};

	template<typename T>
	class ModelPartForSimple : public ModelPart {
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
			virtual ModelPartPtr GetChild(const std::string &) override { return NULL; }
			virtual void SetValue(ValueSourcePtr s) override { s->set(Member); }
			virtual void GetValue(ValueTargetPtr s) override { s->get(Member); }
			virtual bool HasValue() const override { return true; }
			virtual ModelPartType GetType() const { return mpt_Simple; }

		private:
			T & Member;
	};

	template<typename T>
	class ModelPartForOptional : public ModelPart {
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
			virtual ModelPartPtr GetChild(const std::string & name) override
			{
				if (OptionalMember) {
					return modelPart->GetChild(name);
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

		private:
			IceUtil::Optional< typename T::element_type > & OptionalMember;
			ModelPartPtr modelPart;
	};

	template<typename T>
	class ModelPartForComplex : public ModelPart {
		public:
			class HookBase : public IceUtil::Shared {
				public:
					virtual ModelPartPtr Get(T * t) const = 0;

					virtual std::string PartName() const = 0;
			};
			typedef IceUtil::Handle<HookBase> HookPtr;

			template <typename MT, typename CT, MT CT::*M, typename MP>
			class Hook : public HookBase {
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
				BOOST_FOREACH (const auto & h, hooks) {
					auto modelPart = h->Get(GetModel());
					ch(h->PartName(), modelPart && modelPart->HasValue() ? modelPart : ModelPartPtr());
				}
			}

			ModelPartPtr GetChild(const std::string & name) override
			{
				auto childitr = std::find_if(hooks.begin(), hooks.end(), [&name](const typename Hooks::value_type & h) {
						return h->PartName() == name;
					});
				if (childitr != hooks.end()) {
					return (*childitr)->Get(GetModel());
				}
				return NULL;
			}

			virtual ModelPartType GetType() const { return mpt_Complex; }

			virtual T * GetModel() = 0;

			typedef std::vector<HookPtr> Hooks;

		private:
			static Hooks hooks;
	};
	
	template<typename T>
	class ModelPartForClass : public ModelPartForComplex<typename T::element_type> {
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
				auto ref = classRefMap()->find(name);
				if (ref == classRefMap()->end()) {
					throw UnknownType(name);
				}
				return ref->second(&this->ModelObject);
			}

			virtual bool HasValue() const override { return ModelObject; }

			virtual TypeId GetTypeId() const override;

		private:
			T & ModelObject;
	};

	template<typename T>
	class ModelPartForStruct : public ModelPartForComplex<T> {
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
	class ModelPartForClassRoot : public ModelPartForClass<T> {
		public:
			ModelPartForClassRoot() :
				ModelPartForClass<T>(ModelObject)
			{
			}

			ModelPartForClassRoot(T o) :
				ModelPartForClass<T>(ModelObject)
			{
				ModelObject = o;
			}

			virtual ModelPartPtr GetChild(const std::string & name) override
			{
				if (!name.empty() && name != rootName) {
					throw IncorrectElementName(rootName);
				}
				ModelPartForClass<T>::Create();
				return new ModelPartForClass<T>(ModelObject);
			}

			virtual void OnEachChild(const ChildHandler & ch) override
			{
				ch(rootName, new ModelPartForClass<T>(ModelObject));
			}

			typename T::element_type * GetModel() override
			{
				return ModelObject.get();
			}

			virtual bool HasValue() const override { return ModelObject; }

		private:
			T ModelObject;
			static std::string rootName;
	};

	template<typename T>
	class ModelPartForSequence : public ModelPart {
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
				BOOST_FOREACH(auto & element, sequence) {
					ch(elementName, elementModelPart(element));
				}
			}

			ModelPartPtr GetChild(const std::string &) override;

			virtual bool HasValue() const override { return true; }

			virtual ModelPartType GetType() const { return mpt_Sequence; }

		private:
			ModelPartPtr elementModelPart(typename T::value_type &) const;

			T & sequence;
			static std::string elementName;
	};

	template<typename T>
	class ModelPartForDictionaryElement : public ModelPartForComplex<ModelPartForDictionaryElement<T> > {
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
	class ModelPartForDictionaryElementInserter : public ModelPartForDictionaryElement<T> {
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
	class ModelPartForDictionary : public ModelPart {
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
				BOOST_FOREACH(auto & pair, dictionary) {
					ch(pairName, new ModelPartForDictionaryElement<T>(const_cast<typename T::key_type *>(&pair.first), &pair.second));
				}
			}
			ModelPartPtr GetChild(const std::string & name) override
			{
				if (!name.empty() && name != pairName) {
					throw IncorrectElementName(pairName);
				}
				return new ModelPartForDictionaryElementInserter<T>(dictionary);
			}

			virtual bool HasValue() const override { return true; }

			virtual ModelPartType GetType() const { return mpt_Dictionary; }

		private:
			T & dictionary;
			static std::string pairName;
	};
}

#endif

