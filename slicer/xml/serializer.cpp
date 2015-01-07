#include "serializer.h"
#include <slicer/metadata.h>
#include <libxml++/document.h>
#include <libxml++/parsers/domparser.h>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/intrusive_ptr.hpp>
#include <stdexcept>
#include <glibmm/ustring.h>

namespace Slicer {
	const std::string md_attribute = "xml:attribute";

	class BadBooleanValue : public std::invalid_argument {
		public:
			BadBooleanValue(const Glib::ustring &) :
				std::invalid_argument("Bad boolean value")
			{
			}
	};

	static const Glib::ustring TrueText("true");
	static const Glib::ustring FalseText("false");

	class XmlValueSource : public ValueSource {
		public:
			XmlValueSource(const Glib::ustring & s) :
				value(s)
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
			XmlContentValueSource(const xmlpp::ContentNode * c) :
				XmlValueSource(c->get_content())
			{
			}
	};

	class XmlAttributeValueSource : public XmlValueSource {
		public:
			XmlAttributeValueSource(const xmlpp::Attribute * a) :
				XmlValueSource(a->get_value())
			{
			}
	};

	class XmlValueTarget : public ValueTarget {
		public:
			XmlValueTarget(boost::function<void(const Glib::ustring &)> a) :
				apply(a)
			{
			}

			virtual void get(const bool & value) const
			{
				if (value) {
					apply(TrueText);
				}
				else {
					apply(FalseText);
				}
			}

			virtual void get(const Ice::Byte & value) const
			{
				apply(boost::lexical_cast<Glib::ustring>((int)value));
			}

			virtual void get(const Ice::Short & value) const
			{
				apply(boost::lexical_cast<Glib::ustring>(value));
			}

			virtual void get(const Ice::Int & value) const
			{
				apply(boost::lexical_cast<Glib::ustring>(value));
			}

			virtual void get(const Ice::Long & value) const
			{
				apply(boost::lexical_cast<Glib::ustring>(value));
			}

			virtual void get(const Ice::Float & value) const
			{
				apply(boost::lexical_cast<Glib::ustring>(value));
			}

			virtual void get(const Ice::Double & value) const
			{
				apply(boost::lexical_cast<Glib::ustring>(value));
			}

			virtual void get(const std::string & value) const
			{
				apply(value);
			}

		private:
			const boost::function<void(const Glib::ustring &)> apply;
	};


	class XmlAttributeValueTarget : public XmlValueTarget {
		public:
			XmlAttributeValueTarget(xmlpp::Element * p, const std::string & n) :
				XmlValueTarget(boost::bind(&xmlpp::Element::set_attribute, p, Glib::ustring(n), _1, Glib::ustring()))
			{
			}
	};

	class XmlContentValueTarget : public XmlValueTarget {
		public:
			XmlContentValueTarget(xmlpp::Element * p) :
				XmlValueTarget(boost::bind(&xmlpp::Element::set_child_text, p, _1))
			{
			}
	};

	void
	XmlDeserializer::DocumentTreeIterate(const xmlpp::Node * node, ModelPartPtr mp)
	{
		while (node) {
			if (auto element = dynamic_cast<const xmlpp::Element *>(node)) {
				auto smp = mp->GetChild(element->get_name(),
						boost::bind(metaDataFlagNotSet, boost::bind(&Slicer::HookCommon::GetMetadata, _1), md_attribute));
				if (smp) {
					if (auto typeIdPropName = smp->GetTypeIdProperty()) {
						if (auto typeAttr = element->get_attribute(*typeIdPropName)) {
							smp = smp->GetSubclassModelPart(typeAttr->get_value());
						}
					}
					smp->Create();
					auto attrs(element->get_attributes());
					if (!attrs.empty()) {
						DocumentTreeIterate(attrs.front(), smp);
					}
					DocumentTreeIterate(element->get_first_child(), smp);
					smp->Complete();
				}
			}
			else if (auto attribute = dynamic_cast<const xmlpp::Attribute *>(node)) {
				auto smp = mp->GetChild(attribute->get_name(),
						boost::bind(metaDataFlagSet, boost::bind(&Slicer::HookCommon::GetMetadata, _1), md_attribute));
				if (smp) {
					smp->Create();
					smp->SetValue(new XmlAttributeValueSource(attribute));
					smp->Complete();
				}
			}
			else if (auto content = dynamic_cast<const xmlpp::ContentNode *>(node)) {
				mp->SetValue(new XmlContentValueSource(content));
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
	XmlSerializer::ModelTreeIterate(xmlpp::Element * n, const std::string & name, ModelPartPtr mp, HookCommonPtr hp)
	{
		if (name.empty()) {
			return;
		}
		if (hp && metaDataFlagSet(hp->GetMetadata(), md_attribute)) {
			mp->GetValue(new XmlAttributeValueTarget(n, name));
		}
		else if (mp) {
			ModelTreeProcessElement(n->add_child(name), mp);
		}
	}

	void
	XmlSerializer::ModelTreeProcessElement(xmlpp::Element * element, ModelPartPtr mp)
	{
		auto typeIdPropName = mp->GetTypeIdProperty();
		auto typeId = mp->GetTypeId();
		if (typeId && typeIdPropName) {
			element->set_attribute(*typeIdPropName, *typeId);
			mp = mp->GetSubclassModelPart(*typeId);
		}
		mp->GetValue(new XmlContentValueTarget(element));
		mp->OnEachChild(boost::bind(&XmlSerializer::ModelTreeIterate, element, _1, _2, _3));
	}

	void
	XmlSerializer::ModelTreeIterateRoot(xmlpp::Document * doc, const std::string & name, ModelPartPtr mp)
	{
		ModelTreeProcessElement(doc->create_root_node(name), mp);
	}

	XmlFileSerializer::XmlFileSerializer(const boost::filesystem::path & p) :
		path(p)
	{
	}

	XmlFileDeserializer::XmlFileDeserializer(const boost::filesystem::path & p) :
		path(p)
	{
	}

	void
	XmlFileDeserializer::Deserialize(ModelPartPtr modelRoot)
	{
		xmlpp::DomParser dom(path.string());
		auto doc = dom.get_document();
		DocumentTreeIterate(doc, modelRoot);
	}

	void
	XmlFileSerializer::Serialize(ModelPartPtr modelRoot)
	{
		xmlpp::Document doc;
		modelRoot->OnEachChild(boost::bind(&XmlSerializer::ModelTreeIterateRoot, &doc, _1, _2));
		doc.write_to_file_formatted(path.string());
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
	XmlDocumentDeserializer::Deserialize(ModelPartPtr modelRoot)
	{
		DocumentTreeIterate(doc, modelRoot);
	}

	void
	XmlDocumentSerializer::Serialize(ModelPartPtr modelRoot)
	{
		doc = new xmlpp::Document();
		modelRoot->OnEachChild(boost::bind(&XmlSerializer::ModelTreeIterateRoot, doc, _1, _2));
	}
}

