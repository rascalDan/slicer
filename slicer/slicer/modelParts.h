#ifndef SLICER_MODELPARTS_H
#define SLICER_MODELPARTS_H

#include <Ice/Config.h>
#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>
#include <Ice/Handle.h>
#include <Slice/Parser.h>
#include <stdexcept>
#include <boost/function.hpp>
#include <boost/foreach.hpp>

namespace Slicer {
	class IncorrectElementName : public std::invalid_argument {
		public:
			IncorrectElementName() :
				std::invalid_argument("") { }
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

	typedef boost::function<void(const std::string &, ModelPartPtr)> ChildHandler;

	class ModelPart : public IceUtil::Shared {
		public:
			virtual ~ModelPart() = default;

			virtual void OnEachChild(const ChildHandler &) = 0;
			virtual ModelPartPtr GetChild(const std::string &) = 0;
			virtual void Create();
			virtual void Complete();
			virtual void SetValue(ValueSourcePtr);
			virtual void GetValue(ValueTargetPtr);
	};

	template<typename T>
	class ModelPartForSimple : public ModelPart {
		public:
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

		private:
			T & Member;
	};

	template<typename T>
	class ModelPartForComplex : public ModelPart {
		public:
			class HookBase : public IceUtil::Shared {
				public:
					virtual ModelPartPtr Get(T & t) const = 0;
			};
			typedef IceUtil::Handle<HookBase> HookPtr;

			template <typename MT, MT T::*M, typename MP>
			class Hook : public HookBase {
				public:
					ModelPartPtr Get(T & t) const override
					{
						return new MP(t.*M);
					}
			};

			virtual void OnEachChild(const ChildHandler & ch)
			{
				for (auto h = hooks.begin(); h != hooks.end(); h++) {
					ch(h->first, h->second->Get(GetModel()));
				}
			}

			ModelPartPtr GetChild(const std::string & name) override
			{
				auto childitr = hooks.find(name);
				if (childitr != hooks.end()) {
					return childitr->second->Get(GetModel());
				}
				return NULL;
			}

			virtual T & GetModel() = 0;

			typedef std::map<std::string, HookPtr> Hooks;

		private:
			static Hooks hooks;
	};
	
	template<typename T>
	class ModelPartForClass : public ModelPartForComplex<T> {
		public:
			ModelPartForClass(IceInternal::Handle<T> & h) :
				ModelObject(h)
			{
			}

			virtual void Create() override
			{
				ModelObject = new T();
			}

			T & GetModel() override
			{
				return *ModelObject;
			}

		private:
			IceInternal::Handle<T> & ModelObject;
	};

	template<typename T>
	class ModelPartForStruct : public ModelPartForComplex<T> {
		public:
			ModelPartForStruct(T & o) :
				ModelObject(o)
			{
			}

			T & GetModel() override
			{
				return ModelObject;
			}

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

			ModelPartForClassRoot(IceInternal::Handle<T> o) :
				ModelPartForClass<T>(ModelObject)
			{
				ModelObject = o;
			}

			virtual ModelPartPtr GetChild(const std::string & name) override
			{
				if (!name.empty() && name != rootName) {
					throw IncorrectElementName();
				}
				ModelPartForClass<T>::Create();
				return new ModelPartForClass<T>(ModelObject);
			}

			virtual void OnEachChild(const ChildHandler & ch) override
			{
				ch(rootName, new ModelPartForClass<T>(ModelObject));
			}

			T & GetModel() override
			{
				return *ModelObject;
			}

		private:
			IceInternal::Handle<T> ModelObject;
			static std::string rootName;
	};

	template<typename T>
	class ModelPartForSequence : public ModelPart {
		public:
			ModelPartForSequence(T & s) :
				sequence(s)
			{
			}
			virtual void OnEachChild(const ChildHandler & ch) override
			{
				BOOST_FOREACH(auto & element, sequence) {
					ch(elementName, elementModelPart(element));
				}
			}

			ModelPartPtr GetChild(const std::string &) override;

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

			ModelPartForDictionaryElement<T> & GetModel() override
			{
				return *this;
			}

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
			ModelPartForDictionary(T & d) :
				dictionary(d)
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
					throw IncorrectElementName();
				}
				return new ModelPartForDictionaryElementInserter<T>(dictionary);
			}


		private:
			T & dictionary;
			static std::string pairName;
	};
}

#endif

