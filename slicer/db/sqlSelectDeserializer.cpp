#include "sqlSelectDeserializer.h"
#include "sqlExceptions.h"
#include "sqlSource.h"
#include <column.h>
#include <selectcommand.h>
#include <slicer/common.h>
#include <slicer/hookMap.h>
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
	SqlSelectDeserializer::fillLowerColumnNameCache()
	{
		BOOST_ASSERT(lowerColumnNames.empty());
		lowerColumnNames.reserve(columnCount);
		if (!typeIdColIdx) {
			orderedColumns.reserve(columnCount);
		}
		for (auto col = 0U; col < columnCount; col += 1) {
			const DB::Column & c = (*cmd)[col];
			lowerColumnNames.emplace_back(to_lower_copy(c.name));
		}
	}

	const DB::Column *
	SqlSelectDeserializer::searchOrFilleColumnCache(size_t idx, const HookCommon * hook)
	{
		if (idx < orderedColumns.size()) {
			return orderedColumns[idx];
		}
		BOOST_ASSERT(idx == orderedColumns.size());
		if (const auto itr = std::find(lowerColumnNames.begin(), lowerColumnNames.end(), hook->nameLower);
				itr != lowerColumnNames.end()) {
			return orderedColumns.emplace_back(&(*cmd)[static_cast<unsigned int>(itr - lowerColumnNames.begin())]);
		}
		else {
			return orderedColumns.emplace_back(nullptr);
		}
	}

	namespace {
		void
		assignFromColumn(ModelPartParam fmp, const DB::Column & c)
		{
			BOOST_ASSERT(fmp);
			BOOST_ASSERT(!c.isNull());
			fmp->Create();
			fmp->SetValue(SqlSource(c));
			fmp->Complete();
		}
	}

	void
	SqlSelectDeserializer::DeserializeSequence(ModelPartParam omp)
	{
		omp->OnAnonChild([this](auto && mp, auto &&) {
			if (lowerColumnNames.empty()) {
				fillLowerColumnNameCache();
			}
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
		if (lowerColumnNames.empty()) {
			fillLowerColumnNameCache();
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
						BOOST_ASSERT(columnCount == lowerColumnNames.size());
						if (typeIdColIdx) {
							for (auto col = 0U; col < columnCount; col += 1) {
								const DB::Column & c = (*cmd)[col];
								if (!c.isNull()) {
									rcmp->OnChild(
											[&c](auto && fmp, auto &&) {
												assignFromColumn(fmp, c);
											},
											lowerColumnNames[col], nullptr, MatchCase::No_Prelowered);
								}
							}
						}
						else {
							rcmp->OnEachChild([idx = 0U, this](auto &&, auto && fmp, auto && hook) mutable {
								if (auto c = searchOrFilleColumnCache(idx, hook); c && !c->isNull()) {
									assignFromColumn(fmp, *c);
								}
								++idx;
							});
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
					const DB::Column & c = (*cmd)[0];
					if (!c.isNull()) {
						assignFromColumn(rmp, c);
					}
				} break;
				default:
					throw UnsupportedModelType();
			}
		});
	}
}
