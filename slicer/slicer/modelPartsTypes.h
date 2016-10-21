#ifndef SLICER_MODELPARTSTYPES_H
#define SLICER_MODELPARTSTYPES_H

#include "modelParts.h"

namespace Slicer {
	template<typename T>
	class DLL_PUBLIC ModelPartForRoot : public ModelPartForRootBase {
		public:
			ModelPartForRoot(T & o);

			const std::string & GetRootName() const override;
			virtual bool HasValue() const override;
			void Write(::Ice::OutputStreamPtr &) const override;
			void Read(::Ice::InputStreamPtr &) override;

			static const std::string rootName;

		private:
			T * ModelObject;
	};

	class DLL_PUBLIC ModelPartForSimpleBase : public ModelPart {
		public:
			virtual void OnEachChild(const ChildHandler &) override;
			virtual ChildRefPtr GetAnonChildRef(const HookFilter &) override;
			virtual ChildRefPtr GetChildRef(const std::string &, const HookFilter &) override;
			virtual bool HasValue() const override;
			virtual ModelPartType GetType() const override;
			static const ModelPartType type;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForSimple : public ModelPartForSimpleBase, protected ModelPartModel<T> {
		public:
			typedef T element_type;

			ModelPartForSimple(T & h);

			virtual void SetValue(ValueSourcePtr s) override;
			virtual void GetValue(ValueTargetPtr s) override;
	};

	class DLL_PUBLIC ModelPartForConvertedBase : public ModelPart {
		public:
			virtual void OnEachChild(const ChildHandler &) override;
			virtual ChildRefPtr GetAnonChildRef(const HookFilter &) override;
			virtual ChildRefPtr GetChildRef(const std::string &, const HookFilter &) override;
			virtual bool HasValue() const override;
			virtual ModelPartType GetType() const override;
			static const ModelPartType type;
	};

	template<typename T, typename M, T M::* MV>
	class DLL_PUBLIC ModelPartForConverted : public ModelPartForConvertedBase, protected ModelPartModel<T> {
		public:
			typedef T element_type;

			ModelPartForConverted(T & h);

			virtual void SetValue(ValueSourcePtr s) override;
			virtual void GetValue(ValueTargetPtr s) override;
	};
	
	class DLL_PUBLIC ModelPartForOptionalBase : public ModelPart {
		public:
			virtual void OnEachChild(const ChildHandler & ch) override;
			virtual void Complete() override;
			virtual ChildRefPtr GetAnonChildRef(const HookFilter & flt) override;
			virtual ChildRefPtr GetChildRef(const std::string & name, const HookFilter & flt) override;
			virtual void SetValue(ValueSourcePtr s) override;
			virtual bool HasValue() const override;
			virtual bool IsOptional() const override;
			virtual const Metadata & GetMetadata() const override;

		protected:
			virtual bool hasModel() const = 0;
			ModelPartPtr modelPart;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForOptional : public ModelPartForOptionalBase, protected ModelPartModel<IceUtil::Optional<typename T::element_type> > {
		public:
			ModelPartForOptional(IceUtil::Optional< typename T::element_type > & h);
			virtual void Create() override;
			virtual void GetValue(ValueTargetPtr s) override;
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

			static void registerClass(const std::string & className, const TypeId & typeName, const ClassRef &);
			static void unregisterClass(const std::string & className, const TypeId & typeName);
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForComplex : public ModelPartForComplexBase {
		public:
			class DLL_PRIVATE HookBase : public HookCommon {
				public:
					HookBase(const std::string & n) : HookCommon(n) { }
					virtual ModelPartPtr Get(T * t) const = 0;
					virtual const Metadata & GetMetadata() const override { return emptyMetadata; }
			};
			typedef IceUtil::Handle<HookBase> HookPtr;

			template <typename MT, typename MP>
			class DLL_PRIVATE Hook : public HookBase {
				public:
					Hook(MT T::* m, const std::string & n) :
						HookBase(n),
						member(m)
					{
					}

					ModelPartPtr Get(T * t) const override
					{
						return t ? new MP(const_cast<typename std::remove_const<MT>::type &>(t->*member)) : NULL;
					}

				private:
					const MT T::* member;
			};

			template <typename MT, typename MP>
			class DLL_PRIVATE HookMetadata : public Hook<MT, MP> {
				public:
					HookMetadata(MT T::* member, const std::string & n, const Metadata & md) :
						Hook<MT, MP>(member, n),
						metadata(md)
					{
					}

					virtual const Metadata & GetMetadata() const override { return metadata; }

					const Metadata metadata;
			};


			virtual void OnEachChild(const ChildHandler & ch);

			virtual ChildRefPtr GetAnonChildRef(const HookFilter & flt) override;
			ChildRefPtr GetChildRef(const std::string & name, const HookFilter & flt) override;

			virtual const Metadata & GetMetadata() const override;

			virtual T * GetModel() = 0;

			typedef std::vector<HookPtr> Hooks;

			static const Hooks hooks;
			static const Metadata metadata;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForClass : public ModelPartForComplex<T>, protected ModelPartModel<IceInternal::Handle<T> > {
		public:
			typedef IceInternal::Handle<T> element_type;

			ModelPartForClass(element_type & h);

			virtual void Create() override;

			T * GetModel() override;

			virtual ModelPartPtr GetSubclassModelPart(const std::string & name) override;

			virtual bool HasValue() const override;

			virtual TypeId GetTypeId() const override;

			virtual IceUtil::Optional<std::string> GetTypeIdProperty() const override;

			static const std::string typeIdProperty;
			static const std::string className;
			static const IceUtil::Optional<std::string> typeName;

			static ModelPartPtr CreateModelPart(void *);
			static void registerClass() __attribute__ ((constructor(210)));
			static void unregisterClass() __attribute__ ((destructor(210)));
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForStruct : public ModelPartForComplex<T>, protected ModelPartModel<T> {
		public:
			typedef T element_type;

			ModelPartForStruct(T & o);

			T * GetModel() override;

			virtual bool HasValue() const override;
	};

	class DLL_PUBLIC ModelPartForEnumBase : public ModelPart {
		public:
			virtual void OnEachChild(const ChildHandler &) override;
			ChildRefPtr GetAnonChildRef(const HookFilter &) override;
			ChildRefPtr GetChildRef(const std::string &, const HookFilter &) override;
			virtual bool HasValue() const override;
			virtual ModelPartType GetType() const override;
			static const ModelPartType type;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForEnum : public ModelPartForEnumBase, protected ModelPartModel<T> {
		public:
			typedef T element_type;
			typedef boost::bimap<T, std::string> Enumerations;

			ModelPartForEnum(T & s);

			virtual const Metadata & GetMetadata() const override;

			virtual void SetValue(ValueSourcePtr s) override;

			virtual void GetValue(ValueTargetPtr s) override;

			static const Metadata metadata;
			static const Enumerations enumerations;
			DLL_PUBLIC static const std::string & lookup(T);
			DLL_PUBLIC static T lookup(const std::string &);
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

			ModelPartForSequence(T & s);

			virtual void OnEachChild(const ChildHandler & ch) override;

			ChildRefPtr GetAnonChildRef(const HookFilter &) override;

			ChildRefPtr GetChildRef(const std::string &, const HookFilter &) override;

			virtual const Metadata & GetMetadata() const override;

			static const Metadata metadata;
			static const std::string elementName;

		private:
			ModelPartPtr elementModelPart(typename T::value_type &) const;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForDictionaryElementInserter : public ModelPartForStruct<typename T::value_type> {
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
			static const ModelPartType type;
	};

	template<typename T>
	class DLL_PUBLIC ModelPartForDictionary : public ModelPartForDictionaryBase, protected ModelPartModel<T> {
		public:
			typedef T element_type;

			ModelPartForDictionary(T & d);

			virtual void OnEachChild(const ChildHandler & ch) override;

			ChildRefPtr GetAnonChildRef(const HookFilter &) override;

			ChildRefPtr GetChildRef(const std::string & name, const HookFilter &) override;

			virtual const Metadata & GetMetadata() const override;

			static const Metadata metadata;
			static const std::string pairName;
	};

}

#endif


