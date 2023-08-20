#include "sqlSelectDeserializer.h"
#include "sqlExceptions.h"
#include "sqlSource.h"
#include <column.h>
#include <memory>
#include <selectcommand.h>
#include <slicer/common.h>
#include <slicer/modelParts.h>
#include <utility>

namespace Slicer {
	SqlSelectDeserializer::SqlSelectDeserializer(DB::SelectCommand * c, std::optional<std::string> tc) :
		cmd(c), columnCount(0), typeIdColName(std::move(tc))
	{
	}

	void
	SqlSelectDeserializer::Deserialize(ModelPartForRootParam mp)
	{
		cmd->execute();
		columnCount = cmd->columnCount();
		if (typeIdColName) {
			typeIdColIdx = cmd->getOrdinal(*typeIdColName);
		}
		switch (mp->GetType()) {
			case Slicer::ModelPartType::Sequence:
				DeserializeSequence(mp);
				return;
			case Slicer::ModelPartType::Complex:
				DeserializeObject(mp);
				return;
			case Slicer::ModelPartType::Simple:
				DeserializeSimple(mp);
				return;
			default:
				throw UnsupportedModelType();
		}
	}

	void
	SqlSelectDeserializer::DeserializeSimple(ModelPartParam mp)
	{
		if (!cmd->fetch()) {
			if (!mp->IsOptional()) {
				throw NoRowsReturned();
			}
			return;
		}
		if (!(*cmd)[0].isNull()) {
			mp->OnAnonChild([this](auto && fmp, auto &&) {
				fmp->Create();
				fmp->SetValue(SqlSource((*cmd)[0]));
				fmp->Complete();
			});
		}
		if (cmd->fetch()) {
			throw TooManyRowsReturned();
		}
	}

	void
	SqlSelectDeserializer::DeserializeSequence(ModelPartParam omp)
	{
		omp->OnAnonChild([this](auto && mp, auto &&) {
			while (cmd->fetch()) {
				DeserializeRow(mp);
			}
		});
	}

	void
	SqlSelectDeserializer::DeserializeObject(ModelPartParam mp)
	{
		if (!cmd->fetch()) {
			if (!mp->IsOptional()) {
				throw NoRowsReturned();
			}
			return;
		}
		DeserializeRow(mp);
		if (cmd->fetch()) {
			while (cmd->fetch()) { }
			throw TooManyRowsReturned();
		}
	}

	void
	SqlSelectDeserializer::DeserializeRow(ModelPartParam mp)
	{
		mp->OnAnonChild([this](auto && rmp, auto &&) {
			switch (rmp->GetType()) {
				case Slicer::ModelPartType::Complex: {
					auto apply = [this](auto && rcmp) {
						rcmp->Create();
						for (auto col = 0U; col < columnCount; col += 1) {
							const DB::Column & c = (*cmd)[col];
							if (!c.isNull()) {
								rcmp->OnChild(
										[&c](auto && fmp, auto &&) {
											if (fmp) {
												fmp->Create();
												fmp->SetValue(SqlSource(c));
												fmp->Complete();
											}
										},
										c.name, nullptr, false);
							}
						}
						rcmp->Complete();
					};
					if (typeIdColIdx) {
						std::string subclass;
						(*cmd)[*typeIdColIdx] >> subclass;
						return rmp->OnSubclass(apply, subclass);
					}
					apply(rmp);
				} break;
				case Slicer::ModelPartType::Simple: {
					rmp->Create();
					const DB::Column & c = (*cmd)[0];
					if (!c.isNull()) {
						rmp->SetValue(SqlSource(c));
					}
					rmp->Complete();
				} break;
				default:
					throw UnsupportedModelType();
			}
		});
	}
}
