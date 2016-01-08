#ifndef SLICER_MODELPARTSTYPES_H
#define SLICER_MODELPARTSTYPES_H

#include "modelParts.h"

namespace Slicer {
	class DLL_PUBLIC ModelPartForSimpleBase : public ModelPart {
		public:
			virtual void OnEachChild(const ChildHandler &) override;
			virtual ChildRefPtr GetAnonChildRef(const HookFilter &) override;
			virtual ChildRefPtr GetChildRef(const std::string &, const HookFilter &) override;
			virtual bool HasValue() const override;
			virtual ModelPartType GetType() const override;
			static ModelPartType type;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForSimple : public ModelPartForSimpleBase {
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
			virtual void SetValue(ValueSourcePtr s) override { s->set(Member); }
			virtual void GetValue(ValueTargetPtr s) override { s->get(Member); }

		private:
			T & Member;
	};

	class DLL_PUBLIC ModelPartForConvertedBase : public ModelPart {
		public:
			virtual void OnEachChild(const ChildHandler &) override;
			virtual ChildRefPtr GetAnonChildRef(const HookFilter &) override;
			virtual ChildRefPtr GetChildRef(const std::string &, const HookFilter &) override;
			virtual bool HasValue() const override;
			virtual ModelPartType GetType() const override;
			static ModelPartType type;
	};

	template<typename T, typename M, T M::* MV>
	class DLL_PUBLIC ModelPartForConverted : public ModelPartForConvertedBase {
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
			virtual void SetValue(ValueSourcePtr s) override;
			virtual void GetValue(ValueTargetPtr s) override;

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

			virtual bool HasValue() const override { return OptionalMember && modelPart->HasValue(); }

			virtual ModelPartType GetType() const
			{
				return T::type;
			}

			virtual bool IsOptional() const override { return true; };

			virtual const Metadata & GetMetadata() const override { return modelPart->GetMetadata(); }

		private:
			IceUtil::Optional< typename T::element_type > & OptionalMember;
			ModelPartPtr modelPart;
	};

	class DLL_PUBLIC ModelPartForComplexBase : public ModelPart {
		public:
			virtual ModelPartType GetType() const override;
			static ModelPartType type;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForComplex : public ModelPartForComplexBase {
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

	class DLL_PUBLIC ModelPartForEnumBase : public ModelPart {
		public:
			virtual void OnEachChild(const ChildHandler &) override;
			ChildRefPtr GetAnonChildRef(const HookFilter &) override;
			ChildRefPtr GetChildRef(const std::string &, const HookFilter &) override;
			virtual bool HasValue() const override;
			virtual ModelPartType GetType() const override;
			static ModelPartType type;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForEnum : public ModelPartForEnumBase {
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

			virtual const Metadata & GetMetadata() const override { return metadata; }

			virtual void SetValue(ValueSourcePtr s) override;

			virtual void GetValue(ValueTargetPtr s) override;

		private:
			T & modelPart;
			static Metadata metadata;
			static Enumerations enumerations;
	};

	class DLL_PUBLIC ModelPartForSequenceBase : public ModelPart {
		public:
			virtual bool HasValue() const override;
			virtual ModelPartType GetType() const override;
			static ModelPartType type;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForSequence : public ModelPartForSequenceBase {
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

	class DLL_PUBLIC ModelPartForDictionaryBase : public ModelPart {
		public:
			virtual bool HasValue() const override;
			virtual ModelPartType GetType() const override;
			static ModelPartType type;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForDictionary : public ModelPartForDictionaryBase {
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

			virtual const Metadata & GetMetadata() const override { return metadata; }

		private:
			T & dictionary;
			static std::string pairName;
			static Metadata metadata;
	};

}

#endif

