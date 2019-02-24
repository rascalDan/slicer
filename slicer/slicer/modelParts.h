#ifndef SLICER_MODELPARTS_H
#define SLICER_MODELPARTS_H

#include <Ice/Optional.h>
#include <Ice/InputStream.h>
#include <Ice/OutputStream.h>
#include <stdexcept>
#include <functional>
#include <vector>
#include <list>
#include <visibility.h>

namespace Slicer {
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

	typedef std::shared_ptr<ModelPart> ModelPartPtr;
	typedef std::shared_ptr<ModelPartForRootBase> ModelPartForRootPtr;
	typedef std::unique_ptr<HookCommon> HookCommonPtr;
	typedef Ice::optional<std::string> TypeId;

	typedef std::function<void(const std::string &, ModelPartPtr, const HookCommon *)> ChildHandler;

	typedef std::function<ModelPartPtr(void *)> ClassRef;
	typedef std::function<bool(const HookCommon *)> HookFilter;
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

	class DLL_PUBLIC ChildRef {
		public:
			ChildRef();
			ChildRef(ModelPartPtr);
			ChildRef(ModelPartPtr, const Metadata &);

			ModelPartPtr Child() const;
			const Metadata & ChildMetaData() const;
			operator bool() const;

		private:
			ModelPartPtr mpp;
			const Metadata & mdr;
	};

	class DLL_PUBLIC HookCommon {
		public:
			HookCommon(std::string);

			bool filter(const HookFilter & flt);
			void apply(const ChildHandler & ch, const ModelPartPtr & modelPart);

			virtual const Metadata & GetMetadata() const = 0;

			const std::string name;
	};

	struct DLL_PUBLIC case_less {
		bool operator()(std::string_view && lhs, std::string_view && rhs) const;
	};

	class DLL_PUBLIC ModelPart : public std::enable_shared_from_this<ModelPart> {
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
			virtual ChildRef GetAnonChildRef(const HookFilter & = HookFilter()) = 0;
			virtual ChildRef GetChildRef(const std::string & memberName, const HookFilter & = HookFilter(), bool matchCase = true) = 0;
			virtual ModelPartPtr GetSubclassModelPart(const std::string &);
			virtual TypeId GetTypeId() const;
			virtual Ice::optional<std::string> GetTypeIdProperty() const;
			virtual ModelPartType GetType() const = 0;
			virtual void Create();
			virtual void Complete();
			virtual void SetValue(ValueSource &&);
			virtual bool GetValue(ValueTarget &&);
			virtual bool HasValue() const = 0;
			virtual const Metadata & GetMetadata() const;
			virtual bool IsOptional() const;
			virtual ModelPartPtr GetContainedModelPart();
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
			virtual ChildRef GetAnonChildRef(const HookFilter &) override;
			virtual ChildRef GetChildRef(const std::string & name, const HookFilter &, bool matchCase = true) override;
			virtual void OnEachChild(const ChildHandler & ch) override;
			virtual ModelPartType GetType() const override;
			virtual bool IsOptional() const override;
			virtual void Write(::Ice::OutputStream &) const = 0;
			virtual void Read(::Ice::InputStream &) = 0;
			virtual ModelPartPtr GetContainedModelPart() override;

			ModelPartPtr mp;
	};
}

#endif

