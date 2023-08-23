#include <benchmark/benchmark.h>
#include <definedDirs.h>
#include <enums.h>
#include <json.h>
#include <json/serializer.h>
#include <locals.h>
#include <optionals.h>
#include <slicer/slicer.h>
#include <xml.h>
#include <xml/serializer.h>
// Must go last
#include <libxml++/parsers/domparser.h>

class CoreFixture : public benchmark::Fixture {
protected:
	template<typename Deserializer, typename T, typename... Args>
	void
	runDeserialize(benchmark::State & state, Args &&... args)
	{
		for (auto _ : state) {
			benchmark::DoNotOptimize(Slicer::DeserializeAny<Deserializer, T>(std::forward<Args>(args)...));
		}
	}
};

struct JsonValueFromFile : public json::Value {
	explicit JsonValueFromFile(const std::filesystem::path & path) :
		json::Value {[&path] {
			std::ifstream in {path};
			return json::parseValue(in);
		}()}
	{
	}
};

#define DESERIALIZE_TEST(name, type, ds, ext, obj, suf) \
	BENCHMARK_F(CoreFixture, name##_##ext)(benchmark::State & state) \
	{ \
		obj dom {rootDir / "initial/" #name "." #ext}; \
		runDeserialize<ds, type>(state, dom suf); \
	}

#define XML_DESERIALIZE_TEST(name, type) \
	DESERIALIZE_TEST(name, type, Slicer::XmlDocumentDeserializer, xml, xmlpp::DomParser, .get_document())
XML_DESERIALIZE_TEST(attributemap, TestXml::Maps);
XML_DESERIALIZE_TEST(bare, TestXml::BareContainers);
XML_DESERIALIZE_TEST(someenums, TestModule::SomeEnumsPtr);
XML_DESERIALIZE_TEST(enum, TestModule::SomeNumbers);
XML_DESERIALIZE_TEST(elementmap, TestXml::Maps);
XML_DESERIALIZE_TEST(builtins, TestModule::BuiltInsPtr);
XML_DESERIALIZE_TEST(entityref, TestXml::EntityRef);
XML_DESERIALIZE_TEST(int, int);
XML_DESERIALIZE_TEST(isodate, TestModule::IsoDate);
XML_DESERIALIZE_TEST(seqOfClass, TestModule::Classes);
XML_DESERIALIZE_TEST(simpleArray2, TestModule::SimpleSeq);
XML_DESERIALIZE_TEST(string, std::string);
XML_DESERIALIZE_TEST(struct, TestModule::StructType);
XML_DESERIALIZE_TEST(xmlattr, TestModule::ClassClassPtr);
#undef XML_DESERIALIZE_TEST

#define JSON_DESERIALIZE_TEST(name, type) \
	DESERIALIZE_TEST(name, type, Slicer::JsonValueDeserializer, json, JsonValueFromFile, )
JSON_DESERIALIZE_TEST(builtins2, TestModule::BuiltInsPtr);
JSON_DESERIALIZE_TEST(builtins3, TestModule::BuiltInsPtr);
JSON_DESERIALIZE_TEST(localClass, Locals::LocalClassPtr);
JSON_DESERIALIZE_TEST(localSubClass, Locals::LocalSubClassPtr);
JSON_DESERIALIZE_TEST(localSub2Class, Locals::LocalSub2ClassPtr);
JSON_DESERIALIZE_TEST(objectmap, TestJson::Properties);
JSON_DESERIALIZE_TEST(objectmapMember, TestJson::HasProperitiesPtr);
JSON_DESERIALIZE_TEST(optionals2, TestModule::Optionals2Ptr);
JSON_DESERIALIZE_TEST(optionals3, TestModule::Optionals2Ptr);
JSON_DESERIALIZE_TEST(optionals5, TestModule::Optionals2Ptr);
JSON_DESERIALIZE_TEST(seqOfClass2, TestModule::Classes);
JSON_DESERIALIZE_TEST(simpleArray1, TestModule::SimpleSeq);
JSON_DESERIALIZE_TEST(string2, std::string);
JSON_DESERIALIZE_TEST(struct2, TestModule::StructType);
#undef JSON_DESERIALIZE_TEST

#undef DESERIALIZE_TEST

BENCHMARK_MAIN();
