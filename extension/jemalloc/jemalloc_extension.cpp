#define DUCKDB_EXTENSION_MAIN

#include <iostream>

#include "jemalloc_extension.hpp"

#include "duckdb/common/allocator.hpp"
#include "jemalloc/jemalloc.h"

#include "buffer/bf.h"
#include "buffer/bf_malloc.h"

#ifndef DUCKDB_NO_THREADS
#include "duckdb/common/thread.hpp"
#endif


extern BFmspace *_mspace_data, *_mspace_bf;

namespace duckdb {

void JemallocExtension::Load(DuckDB &db) {
	// NOP: This extension can only be loaded statically
}

std::string JemallocExtension::Name() {
	return "jemalloc";
}

data_ptr_t JemallocExtension::Allocate(PrivateAllocatorData *private_data, idx_t size) {
	auto res = data_ptr_cast(BF_AllocBuf(size));
	// return data_ptr_cast(duckdb_jemalloc::je_malloc(size));
	// BF_shm_print_stats(_mspace_data);
	// std::cerr << (void*) res << " = Allocate(" << size << ")" << std::endl;
	return res;
}

void JemallocExtension::Free(PrivateAllocatorData *private_data, data_ptr_t pointer, idx_t size) {
	// std::cerr << "Free(" << std::hex << (void*) pointer << ")" << std::endl;
	BF_FreeBuf(pointer);
}

data_ptr_t JemallocExtension::Reallocate(PrivateAllocatorData *private_data, data_ptr_t pointer, idx_t old_size,
                                         idx_t size) {
	// std::cerr << "Reallocate(" << std::hex << (void*) pointer << ", " << size << ")" << std::endl;
	return data_ptr_cast(BF_ReallocBuf(pointer, size));
}

static void JemallocCTL(const char *name, void *old_ptr, size_t *old_len, void *new_ptr, size_t new_len) {
	if (duckdb_jemalloc::je_mallctl(name, old_ptr, old_len, new_ptr, new_len) != 0) {
#ifdef DEBUG
		// We only want to throw an exception here when debugging
		throw InternalException("je_mallctl failed for setting \"%s\"", name);
#endif
	}
}

template <class T>
static void SetJemallocCTL(const char *name, T &val) {
	JemallocCTL(name, &val, sizeof(T));
}

static void SetJemallocCTL(const char *name) {
	JemallocCTL(name, nullptr, nullptr, nullptr, 0);
}

template <class T>
static T GetJemallocCTL(const char *name) {
	T result;
	size_t len = sizeof(T);
	JemallocCTL(name, &result, &len, nullptr, 0);
	return result;
}

void JemallocExtension::ThreadFlush(idx_t threshold) {
	return;

	std::cerr << "ThreadFlush(" << threshold << ")" << std::endl;
	// We flush after exceeding the threshold
	if (GetJemallocCTL<uint64_t>("thread.peak.read") < threshold) {
		return;
	}

	// Flush thread-local cache
	SetJemallocCTL("thread.tcache.flush");

	// Flush this thread's arena
	const auto purge_arena = StringUtil::Format("arena.%llu.purge", idx_t(GetJemallocCTL<unsigned>("thread.arena")));
	SetJemallocCTL(purge_arena.c_str());

	// Reset the peak after resetting
	SetJemallocCTL("thread.peak.reset");
}

void JemallocExtension::FlushAll() {
	return;

	std::cerr << "FlushAll()" << std::endl;
	// Flush thread-local cache
	SetJemallocCTL("thread.tcache.flush");

	// Flush all arenas
	const auto purge_arena = StringUtil::Format("arena.%llu.purge", MALLCTL_ARENAS_ALL);
	SetJemallocCTL(purge_arena.c_str());

	// Reset the peak after resetting
	SetJemallocCTL("thread.peak.reset");
}

std::string JemallocExtension::Version() const {
#ifdef EXT_VERSION_JEMALLOC
	return EXT_VERSION_JEMALLOC;
#else
	return "";
#endif
}

} // namespace duckdb

extern "C" {

DUCKDB_EXTENSION_API void jemalloc_init(duckdb::DatabaseInstance &db) {
	duckdb::DuckDB db_wrapper(db);
	db_wrapper.LoadExtension<duckdb::JemallocExtension>();
}

DUCKDB_EXTENSION_API const char *jemalloc_version() {
	return duckdb::DuckDB::LibraryVersion();
}
}

#ifndef DUCKDB_EXTENSION_MAIN
#error DUCKDB_EXTENSION_MAIN not defined
#endif
