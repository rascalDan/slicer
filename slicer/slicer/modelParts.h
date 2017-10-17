#ifndef SLICER_MODELPARTS_H
#define SLICER_MODELPARTS_H

#include <Ice/Config.h>
#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>
#include <IceUtil/Optional.h>
#include <Ice/Handle.h>
#include <Ice/StreamF.h>
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

	template <typename T>
	class TValueTarget {
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

	template <typename T>
	class TValueSource {
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

	typedef IceUtil::Handle<ModelPart> ModelPartPtr;
	typedef IceUtil::Handle<ModelPartForRootBase> ModelPartForRootPtr;
	typedef IceUtil::Handle<HookCommon> HookCommonPtr;
	typedef IceUtil::Optional<std::string> TypeId;

	typedef boost::function<void(const std::string &, ModelPartPtr, HookCommonPtr)> ChildHandler;

	typedef boost::function<ModelPartPtr(void *)> ClassRef;
	typedef boost::function<bool(HookCommonPtr)> HookFilter;
	typedef std::map<std::string, ClassRef> ClassRefMap;
	DLL_PUBLIC ClassRefMap * classRefMap();
	typedef boost::bimap<std::string, std::string> ClassNameMap;
	DLL_PUBLIC ClassNameMap * classNameMap();
	typedef std::list<std::string> Metadata;
	DLL_PUBLIC extern const Metadata emptyMetadata;

	enum ModelPartType {
		mpt_Null,
		mpt_Simple,
		mpt_Complex,
		mpt_Sequence,
		mpt_Dictionary,
	};

	enum TryConvertResult {
		tcr_NoAction = 0,
		tcr_NoValue,
		tcr_Value,
	};

	class DLL_PUBLIC ChildRef : virtual public IceUtil::Shared {
		public:
			ChildRef(ModelPartPtr);
			ChildRef(ModelPartPtr, const Metadata &);

			ModelPartPtr Child() const;
			const Metadata & ChildMetaData() const;

		private:
			ModelPartPtr mpp;
			const Metadata & mdr;
	};

	typedef IceUtil::Handle<ChildRef> ChildRefPtr;

	class DLL_PUBLIC HookCommon : virtual public IceUtil::Shared {
		public:
			HookCommon(const std::string &);

			bool filter(const HookFilter & flt);
			bool filter(const HookFilter & flt, const std::string &);
			void apply(const ChildHandler & ch, const ModelPartPtr & modelPart);

			virtual const Metadata & GetMetadata() const = 0;

			const std::string name;
	};

	class DLL_PUBLIC ModelPart : virtual public IceUtil::Shared {
		public:
			virtual ~ModelPart() = default;

			template<typename T>
			static ModelPartPtr CreateFor();
			template<typename T>
			static ModelPartPtr CreateFor(T & t);
			template<typename T>
			static ModelPartForRootPtr CreateRootFor(T & t);

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
			virtual void SetValue(ValueSource &&);
			virtual bool GetValue(ValueTarget &&);
			virtual bool HasValue() const = 0;
			virtual const Metadata & GetMetadata() const;
			virtual bool IsOptional() const;
			virtual ModelPartPtr GetContainedModelPart();

			static const std::string & ToExchangeTypeName(const std::string &);
			static const std::string & ToModelTypeName(const std::string &);
	};

	template<typename T>
	class DLL_PUBLIC ModelPartModel {
		protected:
			ModelPartModel() : Model(nullptr) { }
			ModelPartModel(T * m) : Model(m) { }
			T * Model;
	};

	class DLL_PUBLIC ModelPartForRootBase : public ModelPart {
		public:
			ModelPartForRootBase(ModelPartPtr mp);

			virtual const std::string & GetRootName() const = 0;
			virtual ChildRefPtr GetAnonChildRef(const HookFilter &) override;
			virtual ChildRefPtr GetChildRef(const std::string & name, const HookFilter &) override;
			virtual void OnEachChild(const ChildHandler & ch) override;
			virtual ModelPartType GetType() const override;
			virtual bool IsOptional() const override;
			virtual void Write(::Ice::OutputStreamPtr &) const = 0;
			virtual void Read(::Ice::InputStreamPtr &) = 0;
			virtual ModelPartPtr GetContainedModelPart() override;

			ModelPartPtr mp;
	};
}

#endif

