#include "serializer.h"
#include <xmlExceptions.h>
#include <slicer/metadata.h>
#include <libxml++/document.h>
#include <libxml++/parsers/domparser.h>
#include <boost/lexical_cast.hpp>
#include <functional>
#include <stdexcept>
#include <glibmm/ustring.h>
#include <compileTimeFormatter.h>

NAMEDFACTORY(".xml", Slicer::XmlFileSerializer, Slicer::FileSerializerFactory);
NAMEDFACTORY(".xml", Slicer::XmlFileDeserializer, Slicer::FileDeserializerFactory);
NAMEDFACTORY("application/xml", Slicer::XmlStreamSerializer, Slicer::StreamSerializerFactory);
NAMEDFACTORY("application/xml", Slicer::XmlStreamDeserializer, Slicer::StreamDeserializerFactory);

namespace Slicer {
	using namespace std::placeholders;

	const std::string md_attribute = "xml:attribute";
	const std::string md_text = "xml:text";
	const std::string md_bare = "xml:bare";
	const std::string md_attributes = "xml:attributes";
	const std::string md_elements = "xml:elements";
	const std::string keyName = "key";
	const std::string valueName = "value";
	using ElementCreatorF = xmlpp::Element * (xmlpp::Element::*) (const Glib::ustring &, const Glib::ustring &);
	const auto defaultElementCreator = std::bind((ElementCreatorF)&xmlpp::Element::add_child_element, _1, _2, Glib::ustring());

	static const Glib::ustring TrueText("true");
	static const Glib::ustring FalseText("false");

	class XmlValueSource : public ValueSource {
		public:
			explicit XmlValueSource(Glib::ustring s) :
				value(std::move(s))
			{
			}

			void set(bool & v) const override
			{
				if (value == TrueText) { v = true; return; }
				if (value == FalseText) { v = false; return; }
				throw BadBooleanValue(value);
			}

			void set(Ice::Byte & v) const override
			{
				v = boost::numeric_cast<Ice::Byte>(boost::lexical_cast<int>(value));
			}

			void set(Ice::Short & v) const override
			{
				v = boost::lexical_cast<Ice::Short>(value);
			}

			void set(Ice::Int & v) const override
			{
				v = boost::lexical_cast<Ice::Int>(value);
			}

			void set(Ice::Long & v) const override
			{
				v = boost::lexical_cast<Ice::Long>(value);
			}

			void set(Ice::Float & v) const override
			{
				v = boost::lexical_cast<Ice::Float>(value);
			}

			void set(Ice::Double & v) const override
			{
				v = boost::lexical_cast<Ice::Double>(value);
			}

			void set(std::string & v) const override
			{
				v = value.raw();
			}

		protected:
			const Glib::ustring value;
	};

	class XmlContentValueSource : public XmlValueSource {
		public:
			explicit XmlContentValueSource() :
				XmlValueSource(Glib::ustring())
			{
			}
			explicit XmlContentValueSource(const xmlpp::ContentNode * c) :
				XmlValueSource(c->get_content())
			{
			}
	};

	class XmlAttributeValueSource : public XmlValueSource {
		public:
			explicit XmlAttributeValueSource(const xmlpp::Attribute * a) :
				XmlValueSource(a->get_value())
			{
			}
	};

	class XmlValueTarget : public ValueTarget {
		public:
			explicit XmlValueTarget(std::function<void(const Glib::ustring &)> a) :
				apply(std::move(a))
			{
			}

			void get(const bool & value) const override
			{
				if (value) {
					apply(TrueText);
				}
				else {
					apply(FalseText);
				}
			}

			void get(const Ice::Byte & value) const override
			{
				apply(boost::lexical_cast<Glib::ustring>((int)value));
			}

			void get(const Ice::Short & value) const override
			{
				apply(boost::lexical_cast<Glib::ustring>(value));
			}

			void get(const Ice::Int & value) const override
			{
				apply(boost::lexical_cast<Glib::ustring>(value));
			}

			void get(const Ice::Long & value) const override
			{
				apply(boost::lexical_cast<Glib::ustring>(value));
			}

			void get(const Ice::Float & value) const override
			{
				apply(boost::lexical_cast<Glib::ustring>(value));
			}

			void get(const Ice::Double & value) const override
			{
				apply(boost::lexical_cast<Glib::ustring>(value));
			}

			void get(const std::string & value) const override
			{
				apply(value);
			}

		private:
			const std::function<void(const Glib::ustring &)> apply;
	};


	class XmlAttributeValueTarget : public XmlValueTarget {
		public:
			explicit XmlAttributeValueTarget(xmlpp::Element * p, const std::string & n) :
				XmlValueTarget(std::bind(&xmlpp::Element::set_attribute, p, Glib::ustring(n), _1, Glib::ustring()))
			{
			}
	};

	class XmlContentValueTarget : public XmlValueTarget {
		public:
			explicit XmlContentValueTarget(xmlpp::Element * p) :
				XmlValueTarget(std::bind(&xmlpp::Element::set_first_child_text, p, _1))
			{
			}

			explicit XmlContentValueTarget(const CurrentElementCreator & cec) :
				XmlValueTarget(std::bind(&xmlpp::Element::set_first_child_text, std::bind(&CurrentElementCreator::deref, &cec), _1))
			{
			}
	};

	void
	XmlDeserializer::DocumentTreeIterateDictAttrs(const xmlpp::Element::const_AttributeList & attrs, ModelPartPtr dict)
	{
		for (const auto & attr : attrs) {
			auto emp = dict->GetAnonChild();
			emp->Create();
			auto key = emp->GetChild(keyName);
			auto value = emp->GetChild(valueName);
			key->SetValue(XmlValueSource(attr->get_name()));
			key->Complete();
			value->SetValue(XmlValueSource(attr->get_value()));
			value->Complete();
			emp->Complete();
		}
	}

	void
	XmlDeserializer::DocumentTreeIterateDictElements(const xmlpp::Element * element, ModelPartPtr dict)
	{
		auto node = element->get_first_child();
		while (node) {
			if (auto element = dynamic_cast<const xmlpp::Element *>(node)) {
				auto emp = dict->GetAnonChild();
				emp->Create();
				auto key = emp->GetChild(keyName);
				auto value = emp->GetChildRef(valueName);
				key->SetValue(XmlValueSource(element->get_name()));
				key->Complete();
				DocumentTreeIterateElement(element, value.Child(), value);
				emp->Complete();
			}
			node = node->get_next_sibling();
		}
	}

	void
	XmlDeserializer::DocumentTreeIterateElement(const xmlpp::Element * element, ModelPartPtr smp, const ChildRef & smpr)
	{
		if (auto typeIdPropName = smp->GetTypeIdProperty()) {
			if (auto typeAttr = element->get_attribute(*typeIdPropName)) {
				smp = smp->GetSubclassModelPart(typeAttr->get_value());
			}
		}
		smp->Create();
		if (metaDataFlagSet(smpr.ChildMetaData(), md_attributes)) {
			auto attrs(element->get_attributes());
			if (!attrs.empty()) {
				DocumentTreeIterateDictAttrs(attrs, smp);
			}
		}
		else if (metaDataFlagSet(smpr.ChildMetaData(), md_elements)) {
			DocumentTreeIterateDictElements(element, smp);
		}
		else {
			auto attrs(element->get_attributes());
			if (!attrs.empty()) {
				DocumentTreeIterate(attrs.front(), smp);
			}
			auto firstChild = element->get_first_child();
			if (firstChild) {
				DocumentTreeIterate(firstChild, smp);
			}
			else {
				smp->SetValue(XmlContentValueSource());
			}
		}
		smp->Complete();
	}

	void
	XmlDeserializer::DocumentTreeIterate(const xmlpp::Node * node, ModelPartPtr mp)
	{
		while (node) {
			if (auto element = dynamic_cast<const xmlpp::Element *>(node)) {
				auto smpr = mp->GetChildRef(element->get_name(), [](const auto & h) {
					return metaDataFlagNotSet(h->GetMetadata(), md_attribute);
				});
				if (smpr) {
					auto smp = smpr.Child();
					if (metaDataFlagSet(smpr.ChildMetaData(), md_bare)) {
						smp = smp->GetAnonChild();
					}
					if (smp) {
						DocumentTreeIterateElement(element, smp, smpr);
					}
				}
			}
			else if (auto attribute = dynamic_cast<const xmlpp::Attribute *>(node)) {
				auto smp = mp->GetChild(attribute->get_name(), [](const auto & h) {
					return metaDataFlagSet(h->GetMetadata(), md_attribute);
				});
				if (smp) {
					smp->Create();
					smp->SetValue(XmlAttributeValueSource(attribute));
					smp->Complete();
				}
			}
			else if (auto content = dynamic_cast<const xmlpp::ContentNode *>(node)) {
				ModelPartPtr smp;
				if (!content->is_white_space()) {
					smp = mp->GetAnonChild([](const auto & h) {
						return metaDataFlagSet(h->GetMetadata(), md_text);
					});
				}
				if (smp) {
					smp->SetValue(XmlContentValueSource(content));
				}
				else {
					mp->SetValue(XmlContentValueSource(content));
				}
			}
			node = node->get_next_sibling();
		}
	}

	void
	XmlDeserializer::DocumentTreeIterate(const xmlpp::Document * doc, ModelPartPtr mp)
	{
		DocumentTreeIterate(doc->get_root_node(), mp);
	}

	void
	XmlSerializer::ModelTreeIterate(xmlpp::Element * n, const std::string & name, ModelPartPtr mp, const HookCommon * hp, const ElementCreator & ec)
	{
		if (name.empty()) {
			return;
		}
		if (hp && metaDataFlagSet(hp->GetMetadata(), md_attribute)) {
			mp->GetValue(XmlAttributeValueTarget(n, name));
		}
		else if (hp && metaDataFlagSet(hp->GetMetadata(), md_text)) {
			mp->GetValue(XmlContentValueTarget(n));
		}
		else if (hp && metaDataFlagSet(hp->GetMetadata(), md_attributes)) {
			ModelTreeIterateDictAttrs(n->add_child_element(name), mp);
		}
		else if (hp && metaDataFlagSet(hp->GetMetadata(), md_elements)) {
			ModelTreeIterateDictElements(n->add_child_element(name), mp);
		}
		else {
			if (hp && metaDataFlagSet(hp->GetMetadata(), md_bare)) {
				ModelTreeProcessElement(n, mp, std::bind((ElementCreatorF)&xmlpp::Element::add_child_element, _1, name, Glib::ustring()));
			}
			else {
				CurrentElementCreator cec(std::bind(ec, n, name));
				ModelTreeProcessElement(cec, mp, defaultElementCreator);
			}
		}
	}

	void
	XmlSerializer::ModelTreeIterateDictAttrs(xmlpp::Element * element, ModelPartPtr dict)
	{
		dict->OnEachChild([element](const auto &, const auto & mp, const auto &) {
			if (mp->HasValue()) {
				mp->GetChild(keyName)->GetValue(XmlValueTarget([&mp,element](const auto & name) {
					mp->GetChild(valueName)->GetValue(XmlAttributeValueTarget(element, name));
				}));
			}
		});
	}

	void
	XmlSerializer::ModelTreeIterateDictElements(xmlpp::Element * element, ModelPartPtr dict)
	{
		dict->OnEachChild([element](const auto &, const auto & mp, const auto &) {
			if (mp->HasValue()) {
				mp->GetChild(keyName)->GetValue(XmlValueTarget([&mp,element](const auto & name) {
					CurrentElementCreator cec([&element, &name]() { return element->add_child_element(name); });
					ModelTreeProcessElement(cec, mp->GetChild(valueName), defaultElementCreator);
				}));
			}
		});
	}

	void
	XmlSerializer::ModelTreeProcessElement(const CurrentElementCreator & cec, ModelPartPtr mp, const ElementCreator & ec)
	{
		if (mp->GetType() == mpt_Simple) {
			mp->GetValue(XmlContentValueTarget(cec));
		}
		else if (mp->HasValue()) {
			auto typeIdPropName = mp->GetTypeIdProperty();
			auto typeId = mp->GetTypeId();
			auto element = cec.get();
			if (typeId && typeIdPropName) {
				element->set_attribute(*typeIdPropName, *typeId);
				mp = mp->GetSubclassModelPart(*typeId);
			}
			mp->OnEachChild(std::bind(&XmlSerializer::ModelTreeIterate, element, _1, _2, _3, ec));
		}
	}

	void
	XmlSerializer::ModelTreeIterateRoot(xmlpp::Document * doc, const std::string & name, ModelPartPtr mp)
	{
		ModelTreeProcessElement(doc->create_root_node(name), mp, defaultElementCreator);
	}

	XmlStreamSerializer::XmlStreamSerializer(std::ostream & s) :
		strm(s)
	{
	}

	XmlStreamDeserializer::XmlStreamDeserializer(std::istream & s) :
		strm(s)
	{
	}

	void
	XmlStreamDeserializer::Deserialize(ModelPartForRootPtr modelRoot)
	{
		xmlpp::DomParser dom;
		dom.parse_stream(strm);
		auto doc = dom.get_document();
		DocumentTreeIterate(doc, modelRoot);
	}

	void
	XmlStreamSerializer::Serialize(ModelPartForRootPtr modelRoot)
	{
		xmlpp::Document doc;
		modelRoot->OnEachChild(std::bind(&XmlSerializer::ModelTreeIterateRoot, &doc, _1, _2));
		doc.write_to_stream(strm);
	}

	XmlFileSerializer::XmlFileSerializer(std::filesystem::path p) :
		path(std::move(p))
	{
	}

	XmlFileDeserializer::XmlFileDeserializer(std::filesystem::path p) :
		path(std::move(p))
	{
	}

	void
	XmlFileDeserializer::Deserialize(ModelPartForRootPtr modelRoot)
	{
		xmlpp::DomParser dom(path);
		auto doc = dom.get_document();
		DocumentTreeIterate(doc, modelRoot);
	}

	void
	XmlFileSerializer::Serialize(ModelPartForRootPtr modelRoot)
	{
		xmlpp::Document doc;
		modelRoot->OnEachChild(std::bind(&XmlSerializer::ModelTreeIterateRoot, &doc, _1, _2));
		doc.write_to_file_formatted(path);
	}

	XmlDocumentSerializer::XmlDocumentSerializer(xmlpp::Document * & d) :
		doc(d)
	{
	}

	XmlDocumentDeserializer::XmlDocumentDeserializer(const xmlpp::Document * d) :
		doc(d)
	{
	}

	void
	XmlDocumentDeserializer::Deserialize(ModelPartForRootPtr modelRoot)
	{
		DocumentTreeIterate(doc, modelRoot);
	}

	void
	XmlDocumentSerializer::Serialize(ModelPartForRootPtr modelRoot)
	{
		doc = new xmlpp::Document();
		modelRoot->OnEachChild(std::bind(&XmlSerializer::ModelTreeIterateRoot, doc, _1, _2));
	}

	AdHocFormatter(BadBooleanValueMsg, "Bad boolean value [%?]");
	void BadBooleanValue::ice_print(std::ostream & s) const
	{
		BadBooleanValueMsg::write(s, text);
	}
}

