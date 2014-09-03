#include "serializer.h"
#include <jsonpp.h>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <stdexcept>
#include <fstream>
#include <glibmm/ustring.h>

namespace Slicer {
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

			virtual void set(const Ice::Short & value) const
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
			DocumentTreeIterate(ModelPartPtr mp) : modelPart(mp)
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
				modelPart->Create();
				BOOST_FOREACH(const auto & element, o) {
					auto emp = modelPart->GetChild(element.first);
					if (emp) {
						emp->Create();
						boost::apply_visitor(DocumentTreeIterate(emp), *element.second);
				emp->Complete();
					}
				}
				modelPart->Complete();
			}
			void operator()(const json::Array & a) const
			{
				modelPart->Create();
				BOOST_FOREACH(const auto & element, a) {
					auto emp = modelPart->GetChild(std::string());
					if (emp) {
						emp->Create();
						boost::apply_visitor(DocumentTreeIterate(emp), *element);
				emp->Complete();
					}
				}
				modelPart->Complete();
			}
		private:
			ModelPartPtr modelPart;
	};

	Json::Json(const boost::filesystem::path & p) :
		path(p)
	{
	}

	void
	Json::ModelTreeIterateSeq(json::Value * n, ModelPartPtr mp)
	{
		auto & arr = boost::get<json::Array &>(*n);
		arr.push_back(json::ValuePtr(new json::Value()));
		ModelTreeIterateRoot(arr.back().get(), mp);
	}

	void
	Json::ModelTreeIterate(json::Value * n, const std::string & name, ModelPartPtr mp)
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
						if (auto typeId = mp->GetTypeId()) {
							boost::get<json::Object>(*nn).insert({"slicer-typeid", json::ValuePtr(new json::Value(*typeId))});
							mp = mp->GetSubclassModelPart(*typeId);
						}
						mp->OnEachChild(boost::bind(&Json::ModelTreeIterate, boost::get<json::Object>(*n).insert({name, nn}).first->second.get(), _1, _2));
						break;
					}
				case mpt_Sequence:
				case mpt_Dictionary:
					mp->OnEachChild(boost::bind(&Json::ModelTreeIterateSeq, boost::get<json::Object>(*n).insert({name, json::ValuePtr(new json::Value(json::Array()))}).first->second.get(), _2));
					break;
			}
		}
	}

	void
	Json::ModelTreeIterateRoot(json::Value * n, ModelPartPtr mp)
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
					if (auto typeId = mp->GetTypeId()) {
						boost::get<json::Object>(*n).insert({"slicer-typeid", json::ValuePtr(new json::Value(*typeId))});
						mp = mp->GetSubclassModelPart(*typeId);
					}
					mp->OnEachChild(boost::bind(&Json::ModelTreeIterate, n, _1, _2));
					break;
				case mpt_Sequence:
					*n = json::Array();
					mp->OnEachChild(boost::bind(&Json::ModelTreeIterate, n, _1, _2));
					break;
				case mpt_Dictionary:
					*n = json::Array();
					mp->OnEachChild(boost::bind(&Json::ModelTreeIterate, n, _1, _2));
					break;
			}
		}
	}

	void
	Json::Deserialize(ModelPartPtr modelRoot)
	{
		std::ifstream inFile(path.string());
		std::stringstream buffer;
		buffer << inFile.rdbuf();
		Glib::ustring doc(buffer.str());
		Glib::ustring::const_iterator itr = doc.begin();
		json::Value obj = json::parseValue(itr);
		boost::apply_visitor(DocumentTreeIterate(modelRoot->GetChild(std::string())), obj);
	}

	void
	Json::Serialize(ModelPartPtr modelRoot)
	{
		json::Value doc;
		modelRoot->OnEachChild(boost::bind(&Json::ModelTreeIterateRoot, &doc, _2));
		std::ofstream outFile(path.string());
		json::serializeValue(doc, outFile, "utf-8");
	}
}


