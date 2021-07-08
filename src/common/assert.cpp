#include "duckdb/common/assert.hpp"
#include "duckdb/common/exception.hpp"

namespace duckdb {

void DuckDBAssertInternal(bool condition, const char *condition_name, const char *file, int linenr) {
	if (condition) {
		return;
	} // LCOV_EXCL_START
	throw InternalException("Assertion triggered in file \"%s\" on line %d: %s", file, linenr, condition_name);
} // LCOV_EXCL_STOP

} // namespace duckdb
