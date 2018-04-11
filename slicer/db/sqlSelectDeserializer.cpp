#include "sqlSelectDeserializer.h"
#include "sqlSource.h"
#include <sqlExceptions.h>
#include <common.h>

namespace Slicer {
	SqlSelectDeserializer::SqlSelectDeserializer(DB::SelectCommand * c, IceUtil::Optional<std::string> tc) :
		cmd(c),
		typeIdColName(tc)
	{
	}

	void
	SqlSelectDeserializer::Deserialize(Slicer::ModelPartForRootPtr mp)
	{
		cmd->execute();
		columnCount = cmd->columnCount();
		if (typeIdColName) {
			typeIdColIdx = cmd->getOrdinal(*typeIdColName);
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
		if (!cmd->fetch()) {
			if (!mp->IsOptional()) {
				throw NoRowsReturned();
			}
			return;
		}
		if (!(*cmd)[0].isNull()) {
			auto fmp = mp->GetAnonChild();
			fmp->Create();
			fmp->SetValue(SqlSource((*cmd)[0]));
			fmp->Complete();
		}
		if (cmd->fetch()) {
			throw TooManyRowsReturned();
		}
	}

	void
	SqlSelectDeserializer::DeserializeSequence(Slicer::ModelPartPtr mp)
	{
		mp = mp->GetAnonChild();
		while (cmd->fetch()) {
			DeserializeRow(mp);
		}
	}

	void
	SqlSelectDeserializer::DeserializeObject(Slicer::ModelPartPtr mp)
	{
		if (!cmd->fetch()) {
			if (!mp->IsOptional()) {
				throw NoRowsReturned();
			}
			return;
		}
		DeserializeRow(mp);
		if (cmd->fetch()) {
			while (cmd->fetch()) ;
			throw TooManyRowsReturned();
		}
	}

	void
	SqlSelectDeserializer::DeserializeRow(Slicer::ModelPartPtr mp)
	{
		auto rmp = mp->GetAnonChild();
		if (rmp) {
			switch (rmp->GetType()) {
				case Slicer::mpt_Complex:
					{
						if (typeIdColIdx) {
							std::string subclass;
							(*cmd)[*typeIdColIdx] >> subclass;
							rmp = rmp->GetSubclassModelPart(subclass);
						}
						rmp->Create();
						for (auto col = 0u; col < columnCount; col += 1) {
							const DB::Column & c = (*cmd)[col];
							if (!c.isNull()) {
								auto fmpr = rmp->GetChildRef(c.name, NULL, false);
								if (fmpr) {
									auto fmp = fmpr.Child();
									fmp->Create();
									fmp->SetValue(SqlSource(c));
									fmp->Complete();
								}
							}
						}
						rmp->Complete();
					}
					break;
				case Slicer::mpt_Simple:
					{
						rmp->Create();
						const DB::Column & c = (*cmd)[0];
						if (!c.isNull()) {
							rmp->SetValue(SqlSource(c));
						}
						rmp->Complete();
					}
					break;
				default:
					throw UnsupportedModelType();
			}
		}
	}
}

