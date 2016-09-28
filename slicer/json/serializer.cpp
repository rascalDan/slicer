#include "serializer.h"
#include <slicer/metadata.h>
#include <jsonpp.h>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
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

	class JsonValueSource : public ValueSource {
		public:
			JsonValueSource(const json::Value & s) :
				value(s)
			{
			}

			void set(bool & v) const override
			{
				v = boost::get<bool>(value);
			}

			void set(Ice::Byte & v) const override
			{
				v = boost::numeric_cast<Ice::Byte>(boost::get<json::Number>(value));
			}

			void set(Ice::Short & v) const override
			{
				v = boost::numeric_cast<Ice::Short>(boost::get<json::Number>(value));
			}

			void set(Ice::Int & v) const override
			{
				v = boost::numeric_cast<Ice::Int>(boost::get<json::Number>(value));
			}

			void set(Ice::Long & v) const override
			{
				v = boost::numeric_cast<Ice::Long>(boost::get<json::Number>(value));
			}

			void set(Ice::Float & v) const override
			{
				v = boost::numeric_cast<Ice::Float>(boost::get<json::Number>(value));
			}

			void set(Ice::Double & v) const override
			{
				v = boost::numeric_cast<Ice::Double>(boost::get<json::Number>(value));
			}

			void set(std::string & v) const override
			{
				v = boost::get<json::String>(value);
			}

		protected:
			const json::Value & value;
	};

	class JsonValueTarget : public ValueTarget {
		public:
			JsonValueTarget(json::Value & t) :
				target(t)
			{
				target = json::Null();
			}

			virtual void get(const bool & value) const
			{
				target = value;
			}

			virtual void get(const Ice::Byte & value) const
			{
				target = boost::numeric_cast<json::Number>(value);
			}

			virtual void get(const Ice::Short & value) const
			{
				target = boost::numeric_cast<json::Number>(value);
			}

			virtual void get(const Ice::Int & value) const
			{
				target = boost::numeric_cast<json::Number>(value);
			}

			virtual void get(const Ice::Long & value) const
			{
				target = boost::numeric_cast<json::Number>(value);
			}

			virtual void get(const Ice::Float & value) const
			{
				target = boost::numeric_cast<json::Number>(value);
			}

			virtual void get(const Ice::Double & value) const
			{
				target = boost::numeric_cast<json::Number>(value);
			}

			virtual void get(const std::string & value) const
			{
				target = value;
			}

		private:
			json::Value & target;
	};

	class DocumentTreeIterate : public boost::static_visitor<> {
		public:
			DocumentTreeIterate(ModelPartPtr & mp) : modelPart(mp)
			{
			}
			template<typename SimpleT>
			void operator()(const SimpleT & v) const
			{
				modelPart->Create();
				modelPart->SetValue(new JsonValueSource(v));
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
					if (typeAttrItr != o.end() && boost::get<json::String>(typeAttrItr->second.get())) {
						modelPart = modelPart->GetSubclassModelPart(boost::get<json::String>(*typeAttrItr->second));
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
						key->SetValue(new JsonValueSource(element.first));
						key->Complete();
						boost::apply_visitor(DocumentTreeIterate(value), *element.second);
						emp->Complete();
					}
				}
				else {
					for (const auto & element : o) {
						auto emp = modelPart->GetChild(element.first);
						if (emp) {
							emp->Create();
							boost::apply_visitor(DocumentTreeIterate(emp), *element.second);
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
						boost::apply_visitor(DocumentTreeIterate(emp), *element);
						emp->Complete();
					}
				}
				modelPart->Complete();
			}
		private:
			ModelPartPtr & modelPart;
	};

	void
	JsonSerializer::ModelTreeIterateSeq(json::Value * n, ModelPartPtr mp)
	{
		auto arr = boost::get<json::Array>(n);
		arr->push_back(json::ValuePtr(new json::Value()));
		ModelTreeIterateRoot(arr->back().get(), mp);
	}

	void
	JsonSerializer::ModelTreeIterateDictObj(json::Value * n, ModelPartPtr mp)
	{
		auto obj = boost::get<json::Object>(n);
		json::Object::key_type k;
		auto v = json::ValuePtr(new json::Value());
		json::Value kv;
		mp->GetChild(keyName)->GetValue(new JsonValueTarget(kv));
		JsonValueSource s(kv);
		s.set(k);
		ModelTreeIterateRoot(v.get(), mp->GetChild(valueName));
		obj->insert({ k, v });
	}

	void
	JsonSerializer::ModelTreeIterate(json::Value * n, const std::string & name, ModelPartPtr mp)
	{
		if (name.empty() || !n) {
			return;
		}
		if (mp) {
			switch (mp->GetType()) {
				case mpt_Null:
					boost::get<json::Object>(*n).insert({name, json::ValuePtr(new json::Value())});
					return;
				case mpt_Simple:
					mp->GetValue(new JsonValueTarget(*boost::get<json::Object>(*n).insert({name, json::ValuePtr(new json::Value())}).first->second));
					break;
				case mpt_Complex:
					{
						auto nn = json::ValuePtr(new json::Value(json::Object()));
						if (auto typeIdName = mp->GetTypeIdProperty()) {
							if (auto typeId = mp->GetTypeId()) {
								boost::get<json::Object>(*nn).insert({*typeIdName, json::ValuePtr(new json::Value(*typeId))});
								mp = mp->GetSubclassModelPart(*typeId);
							}
						}
						mp->OnEachChild(boost::bind(&JsonSerializer::ModelTreeIterate, boost::get<json::Object>(*n).insert({name, nn}).first->second.get(), _1, _2));
						break;
					}
				case mpt_Sequence:
					mp->OnEachChild(boost::bind(&JsonSerializer::ModelTreeIterateSeq, boost::get<json::Object>(*n).insert({name, json::ValuePtr(new json::Value(json::Array()))}).first->second.get(), _2));
					break;
				case mpt_Dictionary:
					if (metaDataFlagSet(mp->GetMetadata(), md_object)) {
						mp->OnEachChild(boost::bind(&JsonSerializer::ModelTreeIterateDictObj, boost::get<json::Object>(*n).insert({name, json::ValuePtr(new json::Value(json::Object()))}).first->second.get(), _2));
					}
					else {
						mp->OnEachChild(boost::bind(&JsonSerializer::ModelTreeIterateSeq, boost::get<json::Object>(*n).insert({name, json::ValuePtr(new json::Value(json::Array()))}).first->second.get(), _2));
					}
					break;
			}
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
					mp->GetValue(new JsonValueTarget(*n));
					break;
				case mpt_Complex:
					*n = json::Object();
					if (auto typeIdName = mp->GetTypeIdProperty()) {
						if (auto typeId = mp->GetTypeId()) {
							boost::get<json::Object>(*n).insert({*typeIdName, json::ValuePtr(new json::Value(*typeId))});
							mp = mp->GetSubclassModelPart(*typeId);
						}
					}
					mp->OnEachChild(boost::bind(&JsonSerializer::ModelTreeIterate, n, _1, _2));
					break;
				case mpt_Sequence:
					*n = json::Array();
					mp->OnEachChild(boost::bind(&JsonSerializer::ModelTreeIterateSeq, n, _2));
					break;
				case mpt_Dictionary:
					if (metaDataFlagSet(mp->GetMetadata(), md_object)) {
						*n = json::Object();
						mp->OnEachChild(boost::bind(&JsonSerializer::ModelTreeIterateDictObj, n, _2));
					}
					else {
						*n = json::Array();
						mp->OnEachChild(boost::bind(&JsonSerializer::ModelTreeIterate, n, _1, _2));
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
		boost::apply_visitor(DocumentTreeIterate(mp), obj);
	}

	void
	JsonStreamSerializer::Serialize(ModelPartForRootPtr modelRoot)
	{
		json::Value doc;
		modelRoot->OnEachChild(boost::bind(&JsonSerializer::ModelTreeIterateRoot, &doc, _2));
		json::serializeValue(doc, strm, "utf-8");
	}

	JsonFileSerializer::JsonFileSerializer(const boost::filesystem::path & p) :
		path(p)
	{
	}

	JsonFileDeserializer::JsonFileDeserializer(const boost::filesystem::path & p) :
		path(p)
	{
	}

	void
	JsonFileDeserializer::Deserialize(ModelPartForRootPtr modelRoot)
	{
		std::ifstream inFile(path.string());
		json::Value obj = json::parseValue(inFile);
		auto mp = modelRoot->GetAnonChild();
		boost::apply_visitor(DocumentTreeIterate(mp), obj);
	}

	void
	JsonFileSerializer::Serialize(ModelPartForRootPtr modelRoot)
	{
		json::Value doc;
		modelRoot->OnEachChild(boost::bind(&JsonSerializer::ModelTreeIterateRoot, &doc, _2));
		std::ofstream outFile(path.string());
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
		boost::apply_visitor(DocumentTreeIterate(mp), value);
	}

	void
	JsonValueSerializer::Serialize(ModelPartForRootPtr modelRoot)
	{
		modelRoot->OnEachChild(boost::bind(&JsonSerializer::ModelTreeIterateRoot, &value, _2));
	}
}


