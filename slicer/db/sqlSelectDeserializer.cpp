#include "sqlSelectDeserializer.h"
#include "sqlSource.h"
#include <boost/algorithm/string/predicate.hpp>

namespace Slicer {
	NoRowsReturned::NoRowsReturned() : std::runtime_error("No rows returned") { }

	TooManyRowsReturned::TooManyRowsReturned() : std::runtime_error("Too many rows returned") { }

	UnsupportedModelType::UnsupportedModelType() : std::invalid_argument("Unspported model type") { }

	SqlSelectDeserializer::SqlSelectDeserializer(DB::SelectCommand & c, IceUtil::Optional<std::string> tc) :
		cmd(c),
		typeIdColName(tc)
	{
	}

	void
	SqlSelectDeserializer::Deserialize(Slicer::ModelPartPtr mp)
	{
		cmd.execute();
		columnCount = cmd.columnCount();
		if (typeIdColName) {
			typeIdColIdx = cmd.getOrdinal(*typeIdColName);
		}
		switch (mp->GetType()) {
			case Slicer::mpt_Sequence:
				DeserializeSequence(mp);
				return;
			case Slicer::mpt_Complex:
				DeserializeObject(mp);
				return;
			case Slicer::mpt_Simple:
				DeserializeSimple(mp);
				return;
			default:
				throw UnsupportedModelType();
		}
	}

	void
	SqlSelectDeserializer::DeserializeSimple(Slicer::ModelPartPtr mp)
	{
		auto fmp = mp->GetAnonChild();
		if (!cmd.fetch()) {
			throw NoRowsReturned();
		}
		SqlSourcePtr h = new SqlSource(cmd[0]);
		if (!h->isNull()) {
			fmp->Create();
			fmp->SetValue(h);
			fmp->Complete();
		}
		if (cmd.fetch()) {
			throw TooManyRowsReturned();
		}
	}

	void
	SqlSelectDeserializer::DeserializeSequence(Slicer::ModelPartPtr mp)
	{
		mp = mp->GetAnonChild();
		while (cmd.fetch()) {
			DeserializeRow(mp);
		}
	}

	void
	SqlSelectDeserializer::DeserializeObject(Slicer::ModelPartPtr mp)
	{
		if (!cmd.fetch()) {
			throw NoRowsReturned();
		}
		DeserializeRow(mp);
		if (cmd.fetch()) {
			while (cmd.fetch()) ;
			throw TooManyRowsReturned();
		}
	}

	void
	SqlSelectDeserializer::DeserializeRow(Slicer::ModelPartPtr mp)
	{
		auto rmp = mp->GetAnonChild();
		if (rmp) {
			if (typeIdColIdx) {
				std::string subclass;
				cmd[*typeIdColIdx] >> subclass;
				rmp = rmp->GetSubclassModelPart(subclass);
			}
			rmp->Create();
			for (auto col = 0u; col < columnCount; col += 1) {
				const DB::Column & c = cmd[col];
				SqlSourcePtr h = new SqlSource(c);
				auto fmpr = rmp->GetAnonChildRef([&c](Slicer::HookCommonPtr h) {
						return boost::iequals(c.name.raw(), h->PartName());
					});
				if (fmpr) {
					auto fmp = fmpr->Child();
					if (!h->isNull()) {
						fmp->Create();
						fmp->SetValue(h);
						fmp->Complete();
					}
				}
			}
			rmp->Complete();
		}
	}
}
