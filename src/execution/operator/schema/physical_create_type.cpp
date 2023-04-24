#include "duckdb/execution/operator/schema/physical_create_type.hpp"

#include "duckdb/catalog/catalog.hpp"
#include "duckdb/common/types/column_data_collection.hpp"

namespace duckdb {

PhysicalCreateType::PhysicalCreateType(unique_ptr<CreateTypeInfo> info, idx_t estimated_cardinality)
    : PhysicalOperator(PhysicalOperatorType::CREATE_TYPE, {LogicalType::BIGINT}, estimated_cardinality),
      info(std::move(info)) {
}

//===--------------------------------------------------------------------===//
// Sink
//===--------------------------------------------------------------------===//
class CreateTypeGlobalState : public GlobalSinkState {
public:
	explicit CreateTypeGlobalState(ClientContext &context) : result(LogicalType::VARCHAR) {
	}
	Vector result;
	idx_t size = 0;
	idx_t capacity = STANDARD_VECTOR_SIZE;
};

unique_ptr<GlobalSinkState> PhysicalCreateType::GetGlobalSinkState(ClientContext &context) const {
	return make_uniq<CreateTypeGlobalState>(context);
}

SinkResultType PhysicalCreateType::Sink(ExecutionContext &context, DataChunk &chunk, OperatorSinkInput &input) const {
	auto &gstate = input.global_state.Cast<CreateTypeGlobalState>();
	idx_t total_row_count = gstate.size + chunk.size();
	if (total_row_count > NumericLimits<uint32_t>::Maximum()) {
		throw InvalidInputException("Attempted to create ENUM of size %llu, which exceeds the maximum size of %llu",
		                            total_row_count, NumericLimits<uint32_t>::Maximum());
	}
	UnifiedVectorFormat sdata;
	chunk.data[0].ToUnifiedFormat(chunk.size(), sdata);

	if (total_row_count > gstate.capacity) {
		// We must resize our result vector
		gstate.result.Resize(gstate.capacity, gstate.capacity * 2);
		gstate.capacity *= 2;
	}

	auto src_ptr = (string_t *)sdata.data;
	auto result_ptr = FlatVector::GetData<string_t>(gstate.result);
	// Input vector has NULL value, we just throw an exception
	for (idx_t i = 0; i < chunk.size(); i++) {
		idx_t idx = sdata.sel->get_index(i);
		if (!sdata.validity.RowIsValid(idx)) {
			throw InvalidInputException("Attempted to create ENUM type with NULL value!");
		}
		result_ptr[gstate.size++] =
		    StringVector::AddStringOrBlob(gstate.result, src_ptr[idx].GetDataUnsafe(), src_ptr[idx].GetSize());
	}
	return SinkResultType::NEED_MORE_INPUT;
}

//===--------------------------------------------------------------------===//
// Source
//===--------------------------------------------------------------------===//
class CreateTypeSourceState : public GlobalSourceState {
public:
	CreateTypeSourceState() : finished(false) {
	}

	bool finished;
};

unique_ptr<GlobalSourceState> PhysicalCreateType::GetGlobalSourceState(ClientContext &context) const {
	return make_uniq<CreateTypeSourceState>();
}

SourceResultType PhysicalCreateType::GetData(ExecutionContext &context, DataChunk &chunk, OperatorSourceInput &input) const {
	auto &state = input.global_state.Cast<CreateTypeSourceState>();
	if (state.finished) {
		return SourceResultType::FINISHED;
	}

	if (IsSink()) {
		D_ASSERT(info->type == LogicalType::INVALID);
		auto &g_sink_state = sink_state->Cast<CreateTypeGlobalState>();
		info->type = LogicalType::ENUM(info->name, g_sink_state.result, g_sink_state.size);
	}

	auto &catalog = Catalog::GetCatalog(context.client, info->catalog);
	auto catalog_entry = catalog.CreateType(context.client, info.get());
	D_ASSERT(catalog_entry->type == CatalogType::TYPE_ENTRY);
	auto catalog_type = (TypeCatalogEntry *)catalog_entry;
	LogicalType::SetCatalog(info->type, catalog_type);
	state.finished = true;

	return SourceResultType::HAVE_MORE_OUTPUT;
}

} // namespace duckdb
