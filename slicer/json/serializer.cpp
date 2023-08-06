#include "serializer.h"
#include <Ice/Config.h>
#include <boost/numeric/conversion/cast.hpp>
#include <factory.h>
#include <fstream> // IWYU pragma: keep
#include <functional>
#include <glibmm/ustring.h>
#include <jsonpp.h>
#include <map>
#include <memory>
#include <optional>
#include <slicer/modelParts.h>
#include <slicer/serializer.h>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

NAMEDFACTORY(".js", Slicer::JsonFileSerializer, Slicer::FileSerializerFactory)
NAMEDFACTORY(".js", Slicer::JsonFileDeserializer, Slicer::FileDeserializerFactory)
NAMEDFACTORY(".json", Slicer::JsonFileSerializer, Slicer::FileSerializerFactory)
NAMEDFACTORY(".json", Slicer::JsonFileDeserializer, Slicer::FileDeserializerFactory)
NAMEDFACTORY("application/javascript", Slicer::JsonStreamSerializer, Slicer::StreamSerializerFactory)
NAMEDFACTORY("application/javascript", Slicer::JsonStreamDeserializer, Slicer::StreamDeserializerFactory)
NAMEDFACTORY("application/json", Slicer::JsonStreamSerializer, Slicer::StreamSerializerFactory)
NAMEDFACTORY("application/json", Slicer::JsonStreamDeserializer, Slicer::StreamDeserializerFactory)

namespace Slicer {
	constexpr std::string_view md_object {"json:object"};
	constexpr std::string_view keyName {"key"};
	constexpr std::string_view valueName {"value"};

	using namespace std::placeholders;

	class JsonValueSource : public ValueSource {
	public:
		explicit JsonValueSource(const json::Value & s) : value(s) { }

		void
		set(bool & v) const override
		{
			v = std::get<bool>(value);
		}

		void
		set(Ice::Byte & v) const override
		{
			v = boost::numeric_cast<Ice::Byte>(std::get<json::Number>(value));
		}

		void
		set(Ice::Short & v) const override
		{
			v = boost::numeric_cast<Ice::Short>(std::get<json::Number>(value));
		}

		void
		set(Ice::Int & v) const override
		{
			v = boost::numeric_cast<Ice::Int>(std::get<json::Number>(value));
		}

		void
		set(Ice::Long & v) const override
		{
			v = boost::numeric_cast<Ice::Long>(std::get<json::Number>(value));
		}

		void
		set(Ice::Float & v) const override
		{
			v = boost::numeric_cast<Ice::Float>(std::get<json::Number>(value));
		}

		void
		set(Ice::Double & v) const override
		{
			v = boost::numeric_cast<Ice::Double>(std::get<json::Number>(value));
		}

		void
		set(std::string & v) const override
		{
			v = std::get<json::String>(value);
		}

	private:
		const json::Value & value;
	};

	class JsonValueTarget : public ValueTarget {
	public:
		explicit JsonValueTarget(json::Value & t) : target(t)
		{
			target = json::Null();
		}

		void
		get(const bool & value) const override
		{
			target = value;
		}

		void
		get(const Ice::Byte & value) const override
		{
			target = boost::numeric_cast<json::Number>(value);
		}

		void
		get(const Ice::Short & value) const override
		{
			target = boost::numeric_cast<json::Number>(value);
		}

		void
		get(const Ice::Int & value) const override
		{
			target = boost::numeric_cast<json::Number>(value);
		}

		void
		get(const Ice::Long & value) const override
		{
			target = boost::numeric_cast<json::Number>(value);
		}

		void
		get(const Ice::Float & value) const override
		{
			target = boost::numeric_cast<json::Number>(value);
		}

		void
		get(const Ice::Double & value) const override
		{
			target = boost::numeric_cast<json::Number>(value);
		}

		void
		get(const std::string & value) const override
		{
			target = value;
		}

	private:
		json::Value & target;
	};

	class DocumentTreeIterate {
	public:
		static void
		visit(ModelPartPtr && mp, const json::Value & v)
		{
			std::visit(DocumentTreeIterate {std::move(mp)}, v);
		}

		template<typename SimpleT>
		void
		operator()(const SimpleT & v) const
		{
			modelPart->Create();
			modelPart->SetValue(JsonValueSource(v));
			modelPart->Complete();
		}

		void
		operator()(const json::Null &) const
		{
			modelPart->Complete();
		}

		void
		operator()(const json::Object & o) const
		{
			if (auto typeIdName = modelPart->GetTypeIdProperty()) {
				auto typeAttrItr = o.find(*typeIdName);
				if (typeAttrItr != o.end() && std::holds_alternative<json::String>(typeAttrItr->second)) {
					modelPart = modelPart->GetSubclassModelPart(std::get<json::String>(typeAttrItr->second));
				}
			}
			modelPart->Create();
			if (modelPart->GetMetadata().flagSet(md_object)) {
				for (const auto & element : o) {
					auto emp = modelPart->GetAnonChild();
					emp->Create();
					auto key = emp->GetChild(keyName);
					key->Create();
					key->SetValue(JsonValueSource(element.first));
					key->Complete();
					visit(emp->GetChild(valueName), element.second);
					emp->Complete();
				}
			}
			else {
				for (const auto & element : o) {
					if (auto emp = modelPart->GetChild(element.first)) {
						visit(std::move(emp), element.second);
						emp->Complete();
					}
				}
				modelPart->Complete();
			}
		}

		void
		operator()(const json::Array & a) const
		{
			modelPart->Create();
			for (const auto & element : a) {
				if (auto emp = modelPart->GetAnonChild()) {
					emp->Create();
					visit(std::move(emp), element);
					emp->Complete();
				}
			}
			modelPart->Complete();
		}

		ModelPartPtr && modelPart;
	};

	void
	JsonSerializer::ModelTreeIterateSeq(json::Value * n, ModelPartParam mp)
	{
		if (!mp->HasValue()) {
			return;
		}
		auto arr = std::get_if<json::Array>(n);
		arr->emplace_back();
		ModelTreeIterateRoot(&arr->back(), mp);
	}

	void
	JsonSerializer::ModelTreeIterateDictObj(json::Value * n, ModelPartParam mp)
	{
		if (!mp->HasValue()) {
			return;
		}
		auto obj = std::get_if<json::Object>(n);
		json::Object::key_type k;
		json::Value v;
		json::Value kv;
		mp->GetChild(keyName)->GetValue(JsonValueTarget(kv));
		JsonValueSource s(kv);
		s.set(k);
		ModelTreeIterateRoot(&v, mp->GetChild(valueName));
		obj->insert({k, v});
	}

	void
	JsonSerializer::ModelTreeIterate(json::Value * n, const std::string & name, ModelPartParam mp)
	{
		if (name.empty() || !n || !mp) {
			return;
		}
		switch (mp->GetType()) {
			case ModelPartType::Null:
				std::get<json::Object>(*n).insert({name, json::Null()});
				return;
			case ModelPartType::Simple: {
				json::Value v;
				if (mp->GetValue(JsonValueTarget(v))) {
					std::get<json::Object>(*n).insert({name, v});
				}
				break;
			}
			case ModelPartType::Complex:
				if (mp->HasValue()) {
					auto oec = [n, &name](const auto & lmp) {
						auto & obj = std::get<json::Object>(*n).emplace(name, json::Object {}).first->second;
						lmp->OnEachChild([&obj](auto && PH1, auto && PH2, auto &&) {
							return JsonSerializer::ModelTreeIterate(&obj, PH1, PH2);
						});
						return &std::get<json::Object>(obj);
					};
					if (auto typeIdName = mp->GetTypeIdProperty()) {
						if (auto typeId = mp->GetTypeId()) {
							oec(mp->GetSubclassModelPart(*typeId))->emplace(*typeIdName, *typeId);
							return;
						}
					}
					oec(mp);
				}
				break;
			case ModelPartType::Sequence:
				if (mp->HasValue()) {
					mp->OnEachChild(
							[capture0 = &std::get<json::Object>(*n).insert({name, json::Array()}).first->second](
									auto &&, auto && PH2, auto &&) {
								return JsonSerializer::ModelTreeIterateSeq(capture0, PH2);
							});
				}
				break;
			case ModelPartType::Dictionary:
				if (mp->HasValue()) {
					if (mp->GetMetadata().flagSet(md_object)) {
						mp->OnEachChild(
								[capture0 = &std::get<json::Object>(*n).insert({name, json::Object()}).first->second](
										auto &&, auto && PH2, auto &&) {
									return JsonSerializer::ModelTreeIterateDictObj(capture0, PH2);
								});
					}
					else {
						mp->OnEachChild(
								[capture0 = &std::get<json::Object>(*n).insert({name, json::Array()}).first->second](
										auto &&, auto && PH2, auto &&) {
									return JsonSerializer::ModelTreeIterateSeq(capture0, PH2);
								});
					}
				}
				break;
		}
	}

	void
	JsonSerializer::ModelTreeIterateRoot(json::Value * n, ModelPartParam mp)
	{
		if (mp) {
			switch (mp->GetType()) {
				case ModelPartType::Null:
					*n = json::Null();
					return;
				case ModelPartType::Simple:
					mp->GetValue(JsonValueTarget(*n));
					break;
				case ModelPartType::Complex: {
					auto oec = [n](const auto & lmp) {
						*n = json::Object();
						lmp->OnEachChild([n](auto && PH1, auto && PH2, auto &&) {
							return JsonSerializer::ModelTreeIterate(n, PH1, PH2);
						});
						return &std::get<json::Object>(*n);
					};
					if (auto typeIdName = mp->GetTypeIdProperty()) {
						if (auto typeId = mp->GetTypeId()) {
							oec(mp->GetSubclassModelPart(*typeId))->emplace(*typeIdName, *typeId);
							return;
						}
					}
					oec(mp);
					break;
				}
				case ModelPartType::Sequence:
					*n = json::Array();
					mp->OnEachChild([n](auto &&, auto && PH2, auto &&) {
						return JsonSerializer::ModelTreeIterateSeq(n, PH2);
					});
					break;
				case ModelPartType::Dictionary:
					if (mp->GetMetadata().flagSet(md_object)) {
						*n = json::Object();
						mp->OnEachChild([n](auto &&, auto && PH2, auto &&) {
							return JsonSerializer::ModelTreeIterateDictObj(n, PH2);
						});
					}
					else {
						*n = json::Array();
						mp->OnEachChild([n](auto && PH1, auto && PH2, auto &&) {
							return JsonSerializer::ModelTreeIterate(n, PH1, PH2);
						});
					}
					break;
			}
		}
	}

	JsonStreamSerializer::JsonStreamSerializer(std::ostream & s) : strm(s) { }

	JsonStreamDeserializer::JsonStreamDeserializer(std::istream & s) : strm(s) { }

	void
	JsonStreamDeserializer::Deserialize(ModelPartForRootParam modelRoot)
	{
		DocumentTreeIterate::visit(modelRoot->GetAnonChild(), json::parseValue(strm));
	}

	void
	JsonStreamSerializer::Serialize(ModelPartForRootParam modelRoot)
	{
		JsonValueSerializer::Serialize(modelRoot);
		json::serializeValue(value, strm, "utf-8");
	}

	JsonFileSerializer::JsonFileSerializer(const std::filesystem::path & p) : JsonStreamSerializer {strm}, strm(p) { }

	JsonFileDeserializer::JsonFileDeserializer(std::filesystem::path p) : path(std::move(p)) { }

	void
	JsonFileDeserializer::Deserialize(ModelPartForRootParam modelRoot)
	{
		std::ifstream inFile(path);
		DocumentTreeIterate::visit(modelRoot->GetAnonChild(), json::parseValue(inFile));
	}

	JsonValueDeserializer::JsonValueDeserializer(const json::Value & v) : value(v) { }

	void
	JsonValueDeserializer::Deserialize(ModelPartForRootParam modelRoot)
	{
		DocumentTreeIterate::visit(modelRoot->GetAnonChild(), value);
	}

	void
	JsonValueSerializer::Serialize(ModelPartForRootParam modelRoot)
	{
		modelRoot->OnEachChild([this](auto &&, auto && PH2, auto &&) {
			return JsonSerializer::ModelTreeIterateRoot(&value, PH2);
		});
	}
}
