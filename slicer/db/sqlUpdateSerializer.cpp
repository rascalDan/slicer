#include "sqlUpdateSerializer.h"
#include "sqlBinder.h"
#include "sqlCommon.h"
#include <command_fwd.h>
#include <common.h>
#include <compileTimeFormatter.h>
#include <connection.h>
#include <memory>
#include <modifycommand.h>
#include <slicer/modelParts.h>
#include <sqlExceptions.h>
#include <utility>

namespace Slicer {
	using namespace std::placeholders;

	SqlUpdateSerializer::SqlUpdateSerializer(DB::Connection * const c, std::string t) :
		connection(c), tableName(std::move(t))
	{
	}

	void
	SqlUpdateSerializer::Serialize(ModelPartForRootParam mp)
	{
		switch (mp->GetType()) {
			case Slicer::ModelPartType::Sequence:
				mp->OnEachChild([this](auto &&, auto && PH2, auto &&) {
					SerializeSequence(PH2);
				});
				return;
			case Slicer::ModelPartType::Complex:
				mp->OnEachChild([this](auto &&, auto && PH2, auto &&) {
					SerializeObject(PH2);
				});
				return;
			default:
				throw UnsupportedModelType();
		}
	}

	void
	SqlUpdateSerializer::SerializeObject(ModelPartParam mp) const
	{
		auto ins = createUpdate(mp);
		bindObjectAndExecute(mp, ins.get());
	}

	void
	SqlUpdateSerializer::SerializeSequence(ModelPartParam mp) const
	{
		mp->OnContained([this, mp](auto && cmp) {
			mp->OnEachChild([upd = createUpdate(cmp)](auto &&, auto && chmp, auto &&) {
				bindObjectAndExecute(chmp, upd.get());
			});
		});
	}

	void
	SqlUpdateSerializer::bindObjectAndExecute(ModelPartParam mp, DB::ModifyCommand * upd)
	{
		unsigned int paramNo = 0;
		mp->OnEachChild([&upd, &paramNo](auto &&, auto && cmp, auto && h) {
			if (isValue(h)) {
				if (!cmp->GetValue(SqlBinder(*upd, paramNo))) {
					upd->bindNull(paramNo);
				}
				paramNo++;
			}
		});
		mp->OnEachChild([&upd, &paramNo](auto &&, auto && cmp, auto && h) {
			if (isPKey(h)) {
				cmp->GetValue(SqlBinder(*upd, paramNo++));
			}
		});
		if (upd->execute() == 0) {
			throw NoRowsFound();
		}
	}

	DB::ModifyCommandPtr
	SqlUpdateSerializer::createUpdate(ModelPartParam mp) const
	{
		using namespace AdHoc::literals;
		std::stringstream update;
		"UPDATE %? SET "_fmt(update, tableName);
		int fieldNo = 0;
		mp->OnEachChild([&update, &fieldNo](auto && name, auto &&, auto && h) {
			if (isValue(h)) {
				if (fieldNo++) {
					update << ", ";
				}
				"%? = ?"_fmt(update, name);
			}
		});
		update << " WHERE ";
		fieldNo = 0;
		mp->OnEachChild([&update, &fieldNo](auto && name, auto &&, auto && h) {
			if (isPKey(h)) {
				if (fieldNo++) {
					update << " AND ";
				}
				"%? = ?"_fmt(update, name);
			}
		});
		return connection->modify(std::move(update).str());
	}
}
