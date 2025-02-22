#include "duckdb/catalog/catalog_search_path.hpp"
#include "duckdb/function/scalar/generic_functions.hpp"
#include "duckdb/main/database.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/planner/expression/bound_function_expression.hpp"
#include "duckdb/transaction/transaction.hpp"

namespace duckdb {

// current_query
struct SystemBindData : public FunctionData {
	ClientContext &context;

	explicit SystemBindData(ClientContext &context) : context(context) {
	}

	unique_ptr<FunctionData> Copy() override {
		return make_unique<SystemBindData>(context);
	}

	static SystemBindData &GetFrom(ExpressionState &state) {
		auto &func_expr = (BoundFunctionExpression &)state.expr;
		return (SystemBindData &)*func_expr.bind_info;
	}
};

unique_ptr<FunctionData> BindSystemFunction(ClientContext &context, ScalarFunction &bound_function,
                                            vector<unique_ptr<Expression>> &arguments) {
	return make_unique<SystemBindData>(context);
}

static void CurrentQueryFunction(DataChunk &input, ExpressionState &state, Vector &result) {
	auto &info = SystemBindData::GetFrom(state);
	Value val(info.context.GetCurrentQuery());
	result.Reference(val);
}

// current_schema
static void CurrentSchemaFunction(DataChunk &input, ExpressionState &state, Vector &result) {
	Value val(SystemBindData::GetFrom(state).context.catalog_search_path->GetDefault());
	result.Reference(val);
}

// current_schemas
static void CurrentSchemasFunction(DataChunk &input, ExpressionState &state, Vector &result) {
	vector<Value> schema_list;
	vector<string> search_path = SystemBindData::GetFrom(state).context.catalog_search_path->Get();
	std::transform(search_path.begin(), search_path.end(), std::back_inserter(schema_list),
	               [](const string &s) -> Value { return Value(s); });
	auto val = Value::LIST(schema_list);
	result.Reference(val);
}

// txid_current
static void TransactionIdCurrent(DataChunk &input, ExpressionState &state, Vector &result) {
	auto &func_expr = (BoundFunctionExpression &)state.expr;
	auto &info = (SystemBindData &)*func_expr.bind_info;

	auto &transaction = Transaction::GetTransaction(info.context);
	auto val = Value::BIGINT(transaction.start_time);
	result.Reference(val);
}

// version
static void VersionFunction(DataChunk &input, ExpressionState &state, Vector &result) {
	auto val = Value(DuckDB::LibraryVersion());
	result.Reference(val);
}

void SystemFun::RegisterFunction(BuiltinFunctions &set) {
	auto varchar_list_type = LogicalType::LIST(LogicalType::VARCHAR);

	set.AddFunction(
	    ScalarFunction("current_query", {}, LogicalType::VARCHAR, CurrentQueryFunction, true, BindSystemFunction));
	set.AddFunction(
	    ScalarFunction("current_schema", {}, LogicalType::VARCHAR, CurrentSchemaFunction, false, BindSystemFunction));
	set.AddFunction(ScalarFunction("current_schemas", {LogicalType::BOOLEAN}, varchar_list_type, CurrentSchemasFunction,
	                               false, BindSystemFunction));
	set.AddFunction(
	    ScalarFunction("txid_current", {}, LogicalType::BIGINT, TransactionIdCurrent, false, BindSystemFunction));
	set.AddFunction(ScalarFunction("version", {}, LogicalType::VARCHAR, VersionFunction));
}

} // namespace duckdb
