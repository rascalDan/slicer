#include "serializer.h"
#include <charconv>
#include <compileTimeFormatter.h>
#include <functional>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#ifndef __clang__
#	pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
#include <glibmm/ustring.h>
#include <libxml++/attribute.h>
#include <libxml++/document.h>
#include <libxml++/nodes/contentnode.h>
#include <libxml++/nodes/element.h>
#include <libxml++/nodes/node.h>
#include <libxml++/parsers/domparser.h>
#pragma GCC diagnostic pop
#include <Ice/Config.h>
#include <boost/numeric/conversion/cast.hpp>
#include <factory.h>
#include <lazyPointer.h>
#include <list>
#include <optional>
#include <slicer/metadata.h>
#include <slicer/modelParts.h>
#include <slicer/serializer.h>
#include <string_view>
#include <utility>
#include <xmlExceptions.h>
// IWYU pragma: no_include <boost/detail/basic_pointerbuf.hpp>

NAMEDFACTORY(".xml", Slicer::XmlFileSerializer, Slicer::FileSerializerFactory)
NAMEDFACTORY(".xml", Slicer::XmlFileDeserializer, Slicer::FileDeserializerFactory)
NAMEDFACTORY("application/xml", Slicer::XmlStreamSerializer, Slicer::StreamSerializerFactory)
NAMEDFACTORY("application/xml", Slicer::XmlStreamDeserializer, Slicer::StreamDeserializerFactory)

namespace Slicer {
	namespace {
		constexpr std::string_view md_attribute {"xml:attribute"};
		constexpr std::string_view md_text {"xml:text"};
		constexpr std::string_view md_bare {"xml:bare"};
		constexpr std::string_view md_attributes {"xml:attributes"};
		constexpr std::string_view md_elements {"xml:elements"};
		constexpr std::string_view keyName {"key"};
		constexpr std::string_view valueName {"value"};

		using CurrentElementCreator = ::AdHoc::LazyPointer<xmlpp::Element, xmlpp::Element *>;
		using ElementCreator = std::function<xmlpp::Element *(xmlpp::Element *, const Glib::ustring &)>;

		constexpr auto defaultElementCreator = [](auto && element, auto && name) {
			return element->add_child_element(name);
		};

		const Glib::ustring TrueText("true");
		const Glib::ustring FalseText("false");

		class XmlValueSource : public ValueSource {
		public:
			explicit XmlValueSource() = default;

			explicit XmlValueSource(Glib::ustring s) : value(std::move(s)) { }

			explicit XmlValueSource(const xmlpp::ContentNode * c) : value(c->get_content()) { }

			explicit XmlValueSource(const xmlpp::Attribute * a) : value(a->get_value()) { }

			void
			set(bool & v) const override
			{
				if (value == TrueText) {
					v = true;
					return;
				}
				if (value == FalseText) {
					v = false;
					return;
				}
				throw BadBooleanValue(value);
			}

			void
			set(Ice::Byte & v) const override
			{
				from_chars(v);
			}

			void
			set(Ice::Short & v) const override
			{
				from_chars(v);
			}

			void
			set(Ice::Int & v) const override
			{
				from_chars(v);
			}

			void
			set(Ice::Long & v) const override
			{
				from_chars(v);
			}

			void
			set(Ice::Float & v) const override
			{
				from_chars(v);
			}

			void
			set(Ice::Double & v) const override
			{
				from_chars(v);
			}

			void
			set(std::string & v) const override
			{
				v = value.raw();
			}

		private:
			template<typename T>
			void
			from_chars(T & v) const
			{
				std::string_view raw {value.raw()};
				if (std::from_chars(raw.begin(), raw.end(), v).ec != std::errc {}) {
					throw BadNumericValue(value);
				}
			}

			const Glib::ustring value;
		};

		class XmlValueTarget : public ValueTarget {
		public:
			using ApplyFunction = std::function<void(const Glib::ustring &)>;

			explicit XmlValueTarget(ApplyFunction a) : apply(std::move(a)) { }

			explicit XmlValueTarget(xmlpp::Element * p, const std::string & n) :
				apply([p, n](auto && PH1) {
					p->set_attribute(n, PH1);
				})
			{
			}

			explicit XmlValueTarget(xmlpp::Element * p) :
				apply([p](auto && PH1) {
					p->set_first_child_text(PH1);
				})
			{
			}

			explicit XmlValueTarget(const CurrentElementCreator & cec) :
				apply([&](auto && PH1) {
					cec->set_first_child_text(PH1);
				})
			{
			}

			void
			get(const bool & value) const override
			{
				if (value) {
					apply(TrueText);
				}
				else {
					apply(FalseText);
				}
			}

			void
			get(const Ice::Byte & value) const override
			{
				apply(Glib::ustring::format(value));
			}

			void
			get(const Ice::Short & value) const override
			{
				apply(Glib::ustring::format(value));
			}

			void
			get(const Ice::Int & value) const override
			{
				apply(Glib::ustring::format(value));
			}

			void
			get(const Ice::Long & value) const override
			{
				apply(Glib::ustring::format(value));
			}

			void
			get(const Ice::Float & value) const override
			{
				apply(Glib::ustring::format(value));
			}

			void
			get(const Ice::Double & value) const override
			{
				apply(Glib::ustring::format(value));
			}

			void
			get(const std::string & value) const override
			{
				apply(value);
			}

		private:
			const ApplyFunction apply;
		};

		void DocumentTreeIterate(const xmlpp::Node * node, ModelPartParam mp);
		void DocumentTreeIterateElement(const xmlpp::Element * element, ModelPartParam mp, const Metadata & md);
		void DocumentTreeIterate(const xmlpp::Document * doc, ModelPartParam mp);
		void DocumentTreeIterateDictAttrs(const xmlpp::Element::const_AttributeList & attrs, ModelPartParam dict);
		void DocumentTreeIterateDictElements(const xmlpp::Element * parent, ModelPartParam dict);

		void
		DocumentTreeIterateDictAttrs(const xmlpp::Element::const_AttributeList & attrs, ModelPartParam dict)
		{
			for (const auto & attr : attrs) {
				dict->OnAnonChild([&attr](auto && emp, auto &&) {
					emp->Create();
					emp->OnChild(
							[&attr](auto && child, auto &&) {
								child->SetValue(XmlValueSource(attr->get_name()));
								child->Complete();
							},
							keyName);
					emp->OnChild(
							[&attr](auto && value, auto &&) {
								value->SetValue(XmlValueSource(attr->get_value()));
								value->Complete();
							},
							valueName);
					emp->Complete();
				});
			}
		}

		void
		DocumentTreeIterateDictElements(const xmlpp::Element * element, ModelPartParam dict)
		{
			auto node = element->get_first_child();
			while (node) {
				if (auto childElement = dynamic_cast<const xmlpp::Element *>(node)) {
					dict->OnAnonChild([childElement](auto && emp, auto &&) {
						emp->Create();
						emp->OnChild(
								[childElement](auto && child, auto &&) {
									child->SetValue(XmlValueSource(childElement->get_name()));
									child->Complete();
								},
								keyName);
						emp->OnChild(
								[childElement](auto && value, auto && md) {
									DocumentTreeIterateElement(childElement, value, md);
								},
								valueName);
						emp->Complete();
					});
				}
				node = node->get_next_sibling();
			}
		}

		void
		DocumentTreeIterateElement(const xmlpp::Element * element, ModelPartParam smp, const Metadata & md)
		{
			auto oec = [&md, element](const auto & lmp) {
				lmp->Create();
				if (md.flagSet(md_attributes)) {
					auto attrs(element->get_attributes());
					if (!attrs.empty()) {
						DocumentTreeIterateDictAttrs(attrs, lmp);
					}
				}
				else if (md.flagSet(md_elements)) {
					DocumentTreeIterateDictElements(element, lmp);
				}
				else {
					auto attrs(element->get_attributes());
					if (!attrs.empty()) {
						DocumentTreeIterate(attrs.front(), lmp);
					}
					auto firstChild = element->get_first_child();
					if (firstChild) {
						DocumentTreeIterate(firstChild, lmp);
					}
					else {
						lmp->SetValue(XmlValueSource());
					}
				}
				lmp->Complete();
			};
			if (auto typeIdPropName = smp->GetTypeIdProperty()) {
				if (auto typeAttr = element->get_attribute(*typeIdPropName)) {
					return smp->OnSubclass(oec, typeAttr->get_value());
				}
			}
			oec(smp);
		}

		void
		DocumentTreeIterate(const xmlpp::Node * node, ModelPartParam mp)
		{
			while (node) {
				if (auto element = dynamic_cast<const xmlpp::Element *>(node)) {
					mp->OnChild(
							[element](auto && smp, auto && md) {
								if (md.flagSet(md_bare)) {
									smp->OnAnonChild([element](auto && bmp, auto && bmd) {
										DocumentTreeIterateElement(element, bmp, bmd);
									});
									return;
								}
								DocumentTreeIterateElement(element, smp, md);
							},
							element->get_name().raw(),
							[](const auto & h) {
								return h->GetMetadata().flagNotSet(md_attribute);
							});
				}
				else if (auto attribute = dynamic_cast<const xmlpp::Attribute *>(node)) {
					mp->OnChild(
							[attribute](auto && smp, auto &&) {
								smp->Create();
								smp->SetValue(XmlValueSource(attribute));
								smp->Complete();
							},
							attribute->get_name().raw(),
							[](const auto & h) {
								return h->GetMetadata().flagSet(md_attribute);
							});
				}
				else if (auto content = dynamic_cast<const xmlpp::ContentNode *>(node)) {
					bool bare = false;
					if (!content->is_white_space()) {
						bare = (mp->OnAnonChild(
								[content](auto && smp, auto &&) {
									smp->SetValue(XmlValueSource(content));
								},
								[](const auto & h) {
									return h->GetMetadata().flagSet(md_text);
								}));
					}
					if (!bare) {
						mp->SetValue(XmlValueSource(content));
					}
				}
				node = node->get_next_sibling();
			}
		}

		void
		DocumentTreeIterate(const xmlpp::Document * doc, ModelPartParam mp)
		{
			DocumentTreeIterate(doc->get_root_node(), mp);
		}

		void ModelTreeIterate(xmlpp::Element *, const std::string &, ModelPartParam mp, const HookCommon * hp,
				const ElementCreator &);
		void ModelTreeIterateRoot(xmlpp::Document *, const std::string &, ModelPartParam mp);
		void ModelTreeProcessElement(const CurrentElementCreator &, ModelPartParam mp, const ElementCreator &);
		void ModelTreeIterateDictAttrs(xmlpp::Element * element, ModelPartParam dict);
		void ModelTreeIterateDictElements(xmlpp::Element * element, ModelPartParam dict);

		void
		ModelTreeIterate(xmlpp::Element * n, const std::string & name, ModelPartParam mp, const HookCommon * hp,
				const ElementCreator & ec)
		{
			if (name.empty()) {
				return;
			}
			if (hp && hp->GetMetadata().flagSet(md_attribute)) {
				mp->GetValue(XmlValueTarget(n, name));
			}
			else if (hp && hp->GetMetadata().flagSet(md_text)) {
				mp->GetValue(XmlValueTarget(n));
			}
			else if (hp && hp->GetMetadata().flagSet(md_attributes)) {
				ModelTreeIterateDictAttrs(n->add_child_element(name), mp);
			}
			else if (hp && hp->GetMetadata().flagSet(md_elements)) {
				ModelTreeIterateDictElements(n->add_child_element(name), mp);
			}
			else if (hp && hp->GetMetadata().flagSet(md_bare)) {
				ModelTreeProcessElement(n, mp, [name](auto && PH1, auto &&) {
					return PH1->add_child_element(name);
				});
			}
			else {
				CurrentElementCreator cec([ec, n, name] {
					return ec(n, name);
				});
				ModelTreeProcessElement(cec, mp, defaultElementCreator);
			}
		}

		void
		ModelTreeIterateDictAttrs(xmlpp::Element * element, ModelPartParam dict)
		{
			dict->OnEachChild([element](const auto &, const auto & mp, const auto &) {
				if (mp->HasValue()) {
					mp->OnChild(
							[mp, element](auto && key, auto &&) {
								key->GetValue(XmlValueTarget([mp, element](const auto & name) {
									mp->OnChild(
											[element, &name](auto && value, auto &&) {
												value->GetValue(XmlValueTarget(element, name));
											},
											valueName);
								}));
							},
							keyName);
				}
			});
		}

		void
		ModelTreeIterateDictElements(xmlpp::Element * element, ModelPartParam dict)
		{
			dict->OnEachChild([element](const auto &, const auto & mp, const auto &) {
				if (mp->HasValue()) {
					mp->OnChild(
							[mp, element](auto && key, auto &&) {
								key->GetValue(XmlValueTarget([mp, element](const auto & name) {
									CurrentElementCreator cec([&element, &name]() {
										return element->add_child_element(name);
									});
									mp->OnChild(
											[&cec](auto && value, auto &&) {
												ModelTreeProcessElement(cec, value, defaultElementCreator);
											},
											valueName);
								}));
							},
							keyName);
				}
			});
		}

		void
		ModelTreeProcessElement(const CurrentElementCreator & cec, ModelPartParam mp, const ElementCreator & ec)
		{
			if (mp->GetType() == ModelPartType::Simple) {
				mp->GetValue(XmlValueTarget(cec));
			}
			else if (mp->HasValue()) {
				auto oec = [element = cec.get(), &ec](const auto & lmp) {
					lmp->OnEachChild([element, &ec](auto && PH1, auto && PH2, auto && PH3) {
						return ModelTreeIterate(element, PH1, PH2, PH3, ec);
					});
					return element;
				};
				if (auto typeIdPropName = mp->GetTypeIdProperty()) {
					if (auto typeId = mp->GetTypeId()) {
						return mp->OnSubclass(
								[oec, &typeIdPropName, &typeId](auto && smp) {
									oec(smp)->set_attribute(*typeIdPropName, *typeId);
								},
								*typeId);
					}
				}
				oec(mp);
			}
		}

		void
		ModelTreeIterateRoot(xmlpp::Document * doc, const std::string & name, ModelPartParam mp)
		{
			ModelTreeProcessElement(doc->create_root_node(name), mp, defaultElementCreator);
		}
	}

	XmlStreamSerializer::XmlStreamSerializer(std::ostream & s) : strm(s) { }

	XmlStreamDeserializer::XmlStreamDeserializer(std::istream & s) : strm(s) { }

	void
	XmlStreamDeserializer::Deserialize(ModelPartForRootParam modelRoot)
	{
		xmlpp::DomParser dom;
		dom.parse_stream(strm);
		auto doc = dom.get_document();
		DocumentTreeIterate(doc, modelRoot);
	}

	void
	XmlStreamSerializer::Serialize(ModelPartForRootParam modelRoot)
	{
		XmlDocumentSerializer::Serialize(modelRoot);
		doc.write_to_stream(strm);
	}

	XmlFileSerializer::XmlFileSerializer(const std::filesystem::path & p) : XmlStreamSerializer {strm}, strm(p) { }

	XmlFileDeserializer::XmlFileDeserializer(std::filesystem::path p) : path(std::move(p)) { }

	void
	XmlFileDeserializer::Deserialize(ModelPartForRootParam modelRoot)
	{
		xmlpp::DomParser dom(path);
		auto doc = dom.get_document();
		DocumentTreeIterate(doc, modelRoot);
	}

	XmlDocumentDeserializer::XmlDocumentDeserializer(const xmlpp::Document * d) : doc(d) { }

	void
	XmlDocumentDeserializer::Deserialize(ModelPartForRootParam modelRoot)
	{
		DocumentTreeIterate(doc, modelRoot);
	}

	void
	XmlDocumentSerializer::Serialize(ModelPartForRootParam modelRoot)
	{
		modelRoot->OnEachChild([this](auto && PH1, auto && PH2, auto &&) {
			return ModelTreeIterateRoot(&doc, PH1, PH2);
		});
	}

	AdHocFormatter(BadBooleanValueMsg, "Bad boolean value [%?]");

	void
	BadBooleanValue::ice_print(std::ostream & s) const
	{
		BadBooleanValueMsg::write(s, text);
	}

	AdHocFormatter(BadNumericValueMsg, "Bad numeric value [%?]");

	void
	BadNumericValue::ice_print(std::ostream & s) const
	{
		BadNumericValueMsg::write(s, text);
	}
}
