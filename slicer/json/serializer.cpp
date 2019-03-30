#include "serializer.h"
#include <slicer/metadata.h>
#include <jsonpp.h>
#include <boost/lexical_cast.hpp>
#include <functional>
#include <stdexcept>
#include <fstream>
#include <glibmm/ustring.h>

NAMEDFACTORY(".js", Slicer::JsonFileSerializer, Slicer::FileSerializerFactory);
NAMEDFACTORY(".js", Slicer::JsonFileDeserializer, Slicer::FileDeserializerFactory);
NAMEDFACTORY(".json", Slicer::JsonFileSerializer, Slicer::FileSerializerFactory);
NAMEDFACTORY(".json", Slicer::JsonFileDeserializer, Slicer::FileDeserializerFactory);
NAMEDFACTORY("application/javascript", Slicer::JsonStreamSerializer, Slicer::StreamSerializerFactory);
NAMEDFACTORY("application/javascript", Slicer::JsonStreamDeserializer, Slicer::StreamDeserializerFactory);
NAMEDFACTORY("application/json", Slicer::JsonStreamSerializer, Slicer::StreamSerializerFactory);
NAMEDFACTORY("application/json", Slicer::JsonStreamDeserializer, Slicer::StreamDeserializerFactory);

namespace Slicer {
	const std::string md_object = "json:object";
	const std::string keyName = "key";
	const std::string valueName = "value";

	using namespace std::placeholders;

	class JsonValueSource : public ValueSource {
		public:
			explicit JsonValueSource(const json::Value & s) :
				value(s)
			{
			}

			void set(bool & v) const override
			{
				v = std::get<bool>(value);
			}

			void set(Ice::Byte & v) const override
			{
				v = boost::numeric_cast<Ice::Byte>(std::get<json::Number>(value));
			}

			void set(Ice::Short & v) const override
			{
				v = boost::numeric_cast<Ice::Short>(std::get<json::Number>(value));
			}

			void set(Ice::Int & v) const override
			{
				v = boost::numeric_cast<Ice::Int>(std::get<json::Number>(value));
			}

			void set(Ice::Long & v) const override
			{
				v = boost::numeric_cast<Ice::Long>(std::get<json::Number>(value));
			}

			void set(Ice::Float & v) const override
			{
				v = boost::numeric_cast<Ice::Float>(std::get<json::Number>(value));
			}

			void set(Ice::Double & v) const override
			{
				v = boost::numeric_cast<Ice::Double>(std::get<json::Number>(value));
			}

			void set(std::string & v) const override
			{
				v = std::get<json::String>(value);
			}

		private:
			const json::Value & value;
	};

	class JsonValueTarget : public ValueTarget {
		public:
			explicit JsonValueTarget(json::Value & t) :
				target(t)
			{
				target = json::Null();
			}

			void get(const bool & value) const override
			{
				target = value;
			}

			void get(const Ice::Byte & value) const override
			{
				target = boost::numeric_cast<json::Number>(value);
			}

			void get(const Ice::Short & value) const override
			{
				target = boost::numeric_cast<json::Number>(value);
			}

			void get(const Ice::Int & value) const override
			{
				target = boost::numeric_cast<json::Number>(value);
			}

			void get(const Ice::Long & value) const override
			{
				target = boost::numeric_cast<json::Number>(value);
			}

			void get(const Ice::Float & value) const override
			{
				target = boost::numeric_cast<json::Number>(value);
			}

			void get(const Ice::Double & value) const override
			{
				target = boost::numeric_cast<json::Number>(value);
			}

			void get(const std::string & value) const override
			{
				target = value;
			}

		private:
			json::Value & target;
	};

	class DocumentTreeIterate {
		public:
			explicit DocumentTreeIterate(ModelPartPtr & mp) : modelPart(mp)
			{
			}
			template<typename SimpleT>
			void operator()(const SimpleT & v) const
			{
				modelPart->Create();
				modelPart->SetValue(JsonValueSource(v));
				modelPart->Complete();
			}
			void operator()(const json::Null &) const
			{
				modelPart->Complete();
			}
			void operator()(const json::Object & o) const
			{
				if (auto typeIdName = modelPart->GetTypeIdProperty()) {
					auto typeAttrItr = o.find(*typeIdName);
					if (typeAttrItr != o.end() && std::holds_alternative<json::String>(typeAttrItr->second)) {
						modelPart = modelPart->GetSubclassModelPart(std::get<json::String>(typeAttrItr->second));
					}
				}
				modelPart->Create();
				if (metaDataFlagSet(modelPart->GetMetadata(), md_object)) {
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
							emp->Create();
							std::visit(DocumentTreeIterate(emp), element.second);
							emp->Complete();
						}
					}
					modelPart->Complete();
				}
			}
			void operator()(const json::Array & a) const
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
		obj->insert({ k, v });
	}

	void
	JsonSerializer::ModelTreeIterate(json::Value * n, const std::string & name, ModelPartPtr mp)
	{
		if (name.empty() || !n || !mp) {
			return;
		}
		switch (mp->GetType()) {
			case mpt_Null:
				std::get<json::Object>(*n).insert({name, json::Null()});
				return;
			case mpt_Simple:
				{
					json::Value v;
					if (mp->GetValue(JsonValueTarget(v))) {
						std::get<json::Object>(*n).insert({ name, v });
					}
					break;
				}
			case mpt_Complex:
				if (mp->HasValue()) {
					json::Object nn;
					if (auto typeIdName = mp->GetTypeIdProperty()) {
						if (auto typeId = mp->GetTypeId()) {
							nn.insert({*typeIdName, *typeId});
							mp = mp->GetSubclassModelPart(*typeId);
						}
					}
					mp->OnEachChild(std::bind(&JsonSerializer::ModelTreeIterate, &std::get<json::Object>(*n).insert({name, nn}).first->second, _1, _2));
				}
				break;
			case mpt_Sequence:
				if (mp->HasValue()) {
					mp->OnEachChild(std::bind(&JsonSerializer::ModelTreeIterateSeq, &std::get<json::Object>(*n).insert({name, json::Array()}).first->second, _2));
				}
				break;
			case mpt_Dictionary:
				if (mp->HasValue()) {
					if (metaDataFlagSet(mp->GetMetadata(), md_object)) {
						mp->OnEachChild(std::bind(&JsonSerializer::ModelTreeIterateDictObj, &std::get<json::Object>(*n).insert({name, json::Object()}).first->second, _2));
					}
					else {
						mp->OnEachChild(std::bind(&JsonSerializer::ModelTreeIterateSeq, &std::get<json::Object>(*n).insert({name, json::Array()}).first->second, _2));
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
				case mpt_Null:
					*n = json::Null();
					return;
				case mpt_Simple:
					mp->GetValue(JsonValueTarget(*n));
					break;
				case mpt_Complex:
					*n = json::Object();
					if (auto typeIdName = mp->GetTypeIdProperty()) {
						if (auto typeId = mp->GetTypeId()) {
							std::get<json::Object>(*n).insert({*typeIdName, *typeId});
							mp = mp->GetSubclassModelPart(*typeId);
						}
					}
					mp->OnEachChild(std::bind(&JsonSerializer::ModelTreeIterate, n, _1, _2));
					break;
				case mpt_Sequence:
					*n = json::Array();
					mp->OnEachChild(std::bind(&JsonSerializer::ModelTreeIterateSeq, n, _2));
					break;
				case mpt_Dictionary:
					if (metaDataFlagSet(mp->GetMetadata(), md_object)) {
						*n = json::Object();
						mp->OnEachChild(std::bind(&JsonSerializer::ModelTreeIterateDictObj, n, _2));
					}
					else {
						*n = json::Array();
						mp->OnEachChild(std::bind(&JsonSerializer::ModelTreeIterate, n, _1, _2));
					}
					break;
			}
		}
	}

	JsonStreamSerializer::JsonStreamSerializer(std::ostream & s) :
		strm(s)
	{
	}

	JsonStreamDeserializer::JsonStreamDeserializer(std::istream & s) :
		strm(s)
	{
	}

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
		modelRoot->OnEachChild(std::bind(&JsonSerializer::ModelTreeIterateRoot, &doc, _2));
		json::serializeValue(doc, strm, "utf-8");
	}

	JsonFileSerializer::JsonFileSerializer(std::filesystem::path p) :
		path(std::move(p))
	{
	}

	JsonFileDeserializer::JsonFileDeserializer(std::filesystem::path p) :
		path(std::move(p))
	{
	}

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
		modelRoot->OnEachChild(std::bind(&JsonSerializer::ModelTreeIterateRoot, &doc, _2));
		std::ofstream outFile(path);
		json::serializeValue(doc, outFile, "utf-8");
	}

	JsonValueSerializer::JsonValueSerializer(json::Value & v) :
		value(v)
	{
	}

	JsonValueDeserializer::JsonValueDeserializer(const json::Value & v) :
		value(v)
	{
	}

	void
	JsonValueDeserializer::Deserialize(ModelPartForRootPtr modelRoot)
	{
		auto mp = modelRoot->GetAnonChild();
		std::visit(DocumentTreeIterate(mp), value);
	}

	void
	JsonValueSerializer::Serialize(ModelPartForRootPtr modelRoot)
	{
		modelRoot->OnEachChild(std::bind(&JsonSerializer::ModelTreeIterateRoot, &value, _2));
	}
}


