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

			template<typename T>
			static ModelPartPtr CreateFor(T & t);
			template<typename T>
			static ModelPartPtr CreateRootFor(T & t);

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
			virtual bool IsOptional() const;

			static const std::string & ToExchangeTypeName(const std::string &);
			static const std::string & ToModelTypeName(const std::string &);
	};

	template<typename T>
	class ModelPartForRoot : public ModelPart {
		public:
			ModelPartForRoot(T & o);

			virtual ChildRefPtr GetAnonChildRef(const HookFilter &) override;
			virtual ChildRefPtr GetChildRef(const std::string & name, const HookFilter &) override;
			virtual void OnEachChild(const ChildHandler & ch) override;
			virtual bool HasValue() const override;
			virtual ModelPartType GetType() const override;
			virtual bool IsOptional() const override;

		private:
			T * ModelObject;
			ModelPartPtr mp;
			static std::string rootName;
	};
}

#endif

