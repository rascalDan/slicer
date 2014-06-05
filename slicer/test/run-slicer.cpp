#include <slicer/parser.h>
#include <slicer/slicer.h>
#include <xml/serializer.h>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/assert.hpp>
#include <types.h>

namespace fs = boost::filesystem;

template<typename T, typename Serializer>
void
verify(const fs::path & root, const fs::path & tmp, const fs::path & infile, const boost::function<void(const T &)> & check = NULL)
{
	const fs::path input = root / "initial" / infile;
	const fs::path output = tmp / infile;

	fprintf(stderr, "%s : Deserialize\n", input.string().c_str());
	IceInternal::Handle<T> p = Slicer::Deserialize<Serializer, T>(input);
	fprintf(stderr, "%s : Check1\n", input.string().c_str());
	if (check) check(*p);
	fprintf(stderr, "%s : Serialize -> %s\n", input.string().c_str(), output.string().c_str());
	Slicer::Serialize<Serializer>(p, output);
	fprintf(stderr, "%s : Check2\n", input.string().c_str());
	if (check) check(*p);
	fprintf(stderr, "%s : OK\n", input.string().c_str());
}

void
checkOptionals_notset(const TestModule::Optionals & opts)
{
	BOOST_ASSERT(!opts.optSimple);
	BOOST_ASSERT(!opts.optStruct);
	BOOST_ASSERT(!opts.optClass);
	BOOST_ASSERT(!opts.optSeq);
	BOOST_ASSERT(!opts.optDict);
}

void
checkOptionals_areset(const TestModule::Optionals & opts)
{
	BOOST_ASSERT(opts.optSimple);
	BOOST_ASSERT(opts.optSimple == 4);
	BOOST_ASSERT(opts.optStruct);
	BOOST_ASSERT(opts.optStruct->a == 1);
	BOOST_ASSERT(opts.optStruct->b == 2);
	BOOST_ASSERT(opts.optClass);
	BOOST_ASSERT((*opts.optClass)->a == 1);
	BOOST_ASSERT((*opts.optClass)->b == 2);
	BOOST_ASSERT(opts.optSeq->size() == 2);
	BOOST_ASSERT((*opts.optSeq)[0]->a == 3);
	BOOST_ASSERT((*opts.optSeq)[0]->b == 4);
	BOOST_ASSERT((*opts.optSeq)[1]->a == 5);
	BOOST_ASSERT((*opts.optSeq)[1]->b == 6);
	BOOST_ASSERT(opts.optDict);
	BOOST_ASSERT(opts.optDict->size() == 2);
	BOOST_ASSERT(opts.optDict->find(1) == opts.optDict->end());
	BOOST_ASSERT(opts.optDict->find(10) != opts.optDict->end());
	BOOST_ASSERT(opts.optDict->find(10)->second->a == 11);
	BOOST_ASSERT(opts.optDict->find(10)->second->b == 12);
	BOOST_ASSERT(opts.optDict->find(13) != opts.optDict->end());
	BOOST_ASSERT(opts.optDict->find(13)->second->a == 14);
	BOOST_ASSERT(opts.optDict->find(13)->second->b == 15);
}

int
main(int, char ** argv)
{
	const fs::path me = argv[0];
	const fs::path base = "types";
	const fs::path bjamout = me.parent_path();
	const fs::path root = me.parent_path().parent_path().parent_path().parent_path();
	fprintf(stderr, "root -- %s\n", root.string().c_str());
	const fs::path slice = fs::change_extension(root / base, ".ice");
	fprintf(stderr, "slice - %s\n", slice.string().c_str());

	// Tmp
	const fs::path tmp = root / "bin" / "slicer";
	fprintf(stderr, "tmp --- %s\n", tmp.string().c_str());
	fs::create_directory(tmp);

	// Execute
	verify<TestModule::BuiltIns, Slicer::Xml>(root, tmp, "builtins.xml");
	verify<TestModule::Optionals, Slicer::Xml>(root, tmp, "optionals-notset.xml", checkOptionals_notset);
	verify<TestModule::Optionals, Slicer::Xml>(root, tmp, "optionals-areset.xml", checkOptionals_areset);

	return 0;
}

