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
	class ModelPartForSimple : public ModelPartForSimpleBase {
		public:
			typedef T element_type;

			ModelPartForSimple(T & h);

			virtual void SetValue(ValueSourcePtr s) override;
			virtual void GetValue(ValueTargetPtr s) override;

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
	class ModelPartForConverted : public ModelPartForConvertedBase {
		public:
			typedef T element_type;

			ModelPartForConverted(T & h);

			virtual void SetValue(ValueSourcePtr s) override;
			virtual void GetValue(ValueTargetPtr s) override;

		private:
			T & Member;
	};

	template<typename T>
	class ModelPartForOptional : public ModelPart {
		public:
			ModelPartForOptional(IceUtil::Optional< typename T::element_type > & h);
			virtual void OnEachChild(const ChildHandler & ch) override;
			virtual void Complete() override;
			virtual void Create() override;
			virtual ChildRefPtr GetAnonChildRef(const HookFilter & flt) override;
			virtual ChildRefPtr GetChildRef(const std::string & name, const HookFilter & flt) override;
			virtual void SetValue(ValueSourcePtr s) override;
			virtual void GetValue(ValueTargetPtr s) override;

			virtual bool HasValue() const override;

			virtual ModelPartType GetType() const;

			virtual bool IsOptional() const override;

			virtual const Metadata & GetMetadata() const override;

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
	class ModelPartForComplex : public ModelPartForComplexBase {
		public:
			class HookBase : public HookCommon {
				public:
					virtual ModelPartPtr Get(T * t) const = 0;
					virtual const Metadata & GetMetadata() const override { return emptyMetadata; }
			};
			typedef IceUtil::Handle<HookBase> HookPtr;

			template <typename MT, typename CT, MT CT::*M>
			class HookMetadata : public HookBase {
				public:
					virtual const Metadata & GetMetadata() const override { return metadata; }

				private:
					static Metadata metadata;
			};

			template <typename MT, typename CT, MT CT::*M, typename MP, typename Base = HookMetadata<MT, CT, M>>
			class Hook : public Base {
				public:
					Hook(const std::string & n) :
						name(n)
					{
					}

					ModelPartPtr Get(T * t) const override
					{
						return t ? new MP(const_cast<typename std::remove_const<MT>::type &>(t->*M)) : NULL;
					}

					std::string PartName() const override
					{
						return name;
					}


				private:
					const std::string name;
			};

			virtual void OnEachChild(const ChildHandler & ch);

			virtual ChildRefPtr GetAnonChildRef(const HookFilter & flt) override;
			ChildRefPtr GetChildRef(const std::string & name, const HookFilter & flt) override;

			virtual const Metadata & GetMetadata() const override;

			virtual T * GetModel() = 0;

			typedef std::vector<HookPtr> Hooks;

		private:
			static Hooks hooks;
			static Metadata metadata;
	};

	template<typename T>
	class ModelPartForClass : public ModelPartForComplex<typename T::element_type> {
		public:
			typedef T element_type;

			ModelPartForClass(T & h);

			virtual void Create() override;

			typename T::element_type * GetModel() override;

			virtual ModelPartPtr GetSubclassModelPart(const std::string & name) override;

			virtual bool HasValue() const override;

			virtual TypeId GetTypeId() const override;

			virtual IceUtil::Optional<std::string> GetTypeIdProperty() const override;

		private:
			T & ModelObject;
			static std::string typeIdProperty;
	};

	template<typename T>
	class ModelPartForStruct : public ModelPartForComplex<T> {
		public:
			typedef T element_type;

			ModelPartForStruct(T & o);

			T * GetModel() override;

			virtual bool HasValue() const override;

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
	class ModelPartForEnum : public ModelPartForEnumBase {
		public:
			typedef T element_type;
			typedef boost::bimap<T, std::string> Enumerations;

			ModelPartForEnum(T & s);

			virtual const Metadata & GetMetadata() const override;

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
	class ModelPartForSequence : public ModelPartForSequenceBase {
		public:
			typedef T element_type;

			ModelPartForSequence(T & s);

			virtual void OnEachChild(const ChildHandler & ch) override;

			ChildRefPtr GetAnonChildRef(const HookFilter &) override;

			ChildRefPtr GetChildRef(const std::string &, const HookFilter &) override;

			virtual const Metadata & GetMetadata() const override;

		private:
			ModelPartPtr elementModelPart(typename T::value_type &) const;

			T & sequence;
			static std::string elementName;
			static Metadata metadata;
	};

	template<typename T>
	class ModelPartForDictionaryElementInserter : public ModelPartForStruct<typename T::value_type> {
		public:
			ModelPartForDictionaryElementInserter(T & d);

			virtual void Complete() override;

			typename T::value_type value;

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
	class ModelPartForDictionary : public ModelPartForDictionaryBase {
		public:
			typedef T element_type;

			ModelPartForDictionary(T & d);

			virtual void OnEachChild(const ChildHandler & ch) override;

			ChildRefPtr GetAnonChildRef(const HookFilter &) override;

			ChildRefPtr GetChildRef(const std::string & name, const HookFilter &) override;

			virtual const Metadata & GetMetadata() const override;

		private:
			T & dictionary;
			static std::string pairName;
			static Metadata metadata;
	};

}

#endif

