#include "serializer.h"
#include <Ice/Config.h>
#include <boost/numeric/conversion/cast.hpp>
#include <cmath>
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
		explicit DocumentTreeIterate(ModelPartPtr & mp) : modelPart(mp) { }
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
					auto value = emp->GetChild(valueName);
					key->Create();
					key->SetValue(JsonValueSource(element.first));
					key->Complete();
					std::visit(DocumentTreeIterate(value), element.second);
					emp->Complete();
				}
			}
			else {
				for (const auto & element : o) {
					auto emp = modelPart->GetChild(element.first);
					if (emp) {
						std::visit(DocumentTreeIterate(emp), element.second);
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
				auto emp = modelPart->GetAnonChild();
				if (emp) {
					emp->Create();
					std::visit(DocumentTreeIterate(emp), element);
					emp->Complete();
				}
			}
			modelPart->Complete();
		}

	private:
		ModelPartPtr & modelPart;
	};

	void
	JsonSerializer::ModelTreeIterateSeq(json::Value * n, const ModelPartPtr & mp)
	{
		if (!mp->HasValue()) {
			return;
		}
		auto arr = std::get_if<json::Array>(n);
		arr->emplace_back();
		ModelTreeIterateRoot(&arr->back(), mp);
	}

	void
	JsonSerializer::ModelTreeIterateDictObj(json::Value * n, const ModelPartPtr & mp)
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
	JsonSerializer::ModelTreeIterate(json::Value * n, const std::string & name, ModelPartPtr mp)
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
					json::Object nn;
					if (auto typeIdName = mp->GetTypeIdProperty()) {
						if (auto typeId = mp->GetTypeId()) {
							nn.insert({*typeIdName, *typeId});
							mp = mp->GetSubclassModelPart(*typeId);
						}
					}
					mp->OnEachChild([capture0 = &std::get<json::Object>(*n).insert({name, nn}).first->second](
											auto && PH1, auto && PH2, auto &&) {
						return JsonSerializer::ModelTreeIterate(capture0, PH1, PH2);
					});
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
	JsonSerializer::ModelTreeIterateRoot(json::Value * n, ModelPartPtr mp)
	{
		if (mp) {
			switch (mp->GetType()) {
				case ModelPartType::Null:
					*n = json::Null();
					return;
				case ModelPartType::Simple:
					mp->GetValue(JsonValueTarget(*n));
					break;
				case ModelPartType::Complex:
					*n = json::Object();
					if (auto typeIdName = mp->GetTypeIdProperty()) {
						if (auto typeId = mp->GetTypeId()) {
							std::get<json::Object>(*n).insert({*typeIdName, *typeId});
							mp = mp->GetSubclassModelPart(*typeId);
						}
					}
					mp->OnEachChild([n](auto && PH1, auto && PH2, auto &&) {
						return JsonSerializer::ModelTreeIterate(n, PH1, PH2);
					});
					break;
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
	JsonStreamDeserializer::Deserialize(ModelPartForRootPtr modelRoot)
	{
		json::Value obj = json::parseValue(strm);
		auto mp = modelRoot->GetAnonChild();
		std::visit(DocumentTreeIterate(mp), obj);
	}

	void
	JsonStreamSerializer::Serialize(ModelPartForRootPtr modelRoot)
	{
		json::Value doc;
		modelRoot->OnEachChild([&doc](auto &&, auto && PH2, auto &&) {
			return JsonSerializer::ModelTreeIterateRoot(&doc, PH2);
		});
		json::serializeValue(doc, strm, "utf-8");
	}

	JsonFileSerializer::JsonFileSerializer(std::filesystem::path p) : path(std::move(p)) { }

	JsonFileDeserializer::JsonFileDeserializer(std::filesystem::path p) : path(std::move(p)) { }

	void
	JsonFileDeserializer::Deserialize(ModelPartForRootPtr modelRoot)
	{
		std::ifstream inFile(path);
		json::Value obj = json::parseValue(inFile);
		auto mp = modelRoot->GetAnonChild();
		std::visit(DocumentTreeIterate(mp), obj);
	}

	void
	JsonFileSerializer::Serialize(ModelPartForRootPtr modelRoot)
	{
		json::Value doc;
		modelRoot->OnEachChild([&doc](auto &&, auto && PH2, auto &&) {
			return JsonSerializer::ModelTreeIterateRoot(&doc, PH2);
		});
		std::ofstream outFile(path);
		json::serializeValue(doc, outFile, "utf-8");
	}

	JsonValueSerializer::JsonValueSerializer(json::Value & v) : value(v) { }

	JsonValueDeserializer::JsonValueDeserializer(const json::Value & v) : value(v) { }

	void
	JsonValueDeserializer::Deserialize(ModelPartForRootPtr modelRoot)
	{
		auto mp = modelRoot->GetAnonChild();
		std::visit(DocumentTreeIterate(mp), value);
	}

	void
	JsonValueSerializer::Serialize(ModelPartForRootPtr modelRoot)
	{
		modelRoot->OnEachChild([this](auto &&, auto && PH2, auto &&) {
			return JsonSerializer::ModelTreeIterateRoot(&value, PH2);
		});
	}
}
