//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/common/types/bit.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/common.hpp"
#include "duckdb/common/types.hpp"
#include "duckdb/common/types/string_type.hpp"

namespace duckdb {

//! The Bit class is a static class that holds helper functions for the BIT type.
class Bit {
public:
	//! Returns the number of bits in the bit string
	DUCKDB_API static idx_t BitLength(string_t bits);
	//! Returns the number of set bits in the bit string
	DUCKDB_API static idx_t BitCount(string_t bits);
	//! Returns the number of bytes in the bit string
	DUCKDB_API static idx_t OctetLength(string_t bits);
	//! Extracts the nth bit from bit string; the first (leftmost) bit is indexed 0
	DUCKDB_API static idx_t GetBit(string_t bit_string, idx_t n);
	//! Sets the nth bit in bit string to newvalue; the first (leftmost) bit is indexed 0
	DUCKDB_API static void SetBit(string_t &bit_string, idx_t n, idx_t new_value);
	//! Returns first starting index of the specified substring within bits, or zero if it's not present.
	DUCKDB_API static idx_t BitPosition(string_t substring, string_t bits);
	//! Converts bits to a string, writing the output to the designated output string.
	//! The string needs to have space for at least GetStringSize(bits) bytes.
	DUCKDB_API static void ToString(string_t bits, char *output);
	DUCKDB_API static string ToString(string_t str);
	//! Returns the bit size of a string -> bit conversion
	DUCKDB_API static bool TryGetBitStringSize(string_t str, idx_t &result_size, string *error_message);
	//! Convert a string to a bit. This function should ONLY be called after calling GetBitSize, since it does NOT
	//! perform data validation.
	DUCKDB_API static void ToBit(string_t str, string_t &output);

	DUCKDB_API static string ToBit(string_t str);

	DUCKDB_API static void BlobToBit(string_t blob, string_t &output);

	DUCKDB_API static string BlobToBit(string_t blob);

	template<class T>
	DUCKDB_API static void NumericToBit(T numeric, string_t &output_str)  {
		auto output = output_str.GetDataWriteable();
		auto data = const_data_ptr_cast(&numeric);

		*(output++) = 0;
		for (idx_t idx = 0; idx < sizeof(T); ++idx) {
			output[idx] = data[sizeof(T) - idx - 1];
		}
		Bit::Finalize(output_str);
	}

	template<class T>
	DUCKDB_API static string NumericToBit(T numeric) {
		auto bit_len = sizeof(T) + 1;
		auto buffer = make_unsafe_uniq_array<char>(bit_len);
		string_t output_str(buffer.get(), bit_len);
		Bit::NumericToBit(numeric, output_str);
		return output_str.GetString();
	}

	template<class T>
	DUCKDB_API static void BitToNumeric(string_t bit, T &output_num) {
		output_num = 0;
		auto data = const_data_ptr_cast(bit.GetData());
		auto output = data_ptr_cast(&output_num);

		uint8_t padded_byte = data[1] & ((1 << (8 - data[0])) - 1);

		idx_t start = sizeof(T) - bit.GetSize() + 1;

		output[sizeof(T) - 1 - start] = padded_byte;

		for (idx_t idx = start + 1; idx < sizeof(T); ++idx) {
			output[sizeof(T) - 1 - idx] = data[1 + idx - start];
		}

	}

	template<class T>
	DUCKDB_API static T BitToNumeric(string_t bit) {
		T output;
		Bit::BitToNumeric(bit, output);
		return (output);
	}

	//! Creates a new bitstring of determined length
	DUCKDB_API static void BitString(const string_t &input, const idx_t &len, string_t &result);
	DUCKDB_API static void SetEmptyBitString(string_t &target, string_t &input);
	DUCKDB_API static void SetEmptyBitString(string_t &target, idx_t len);
	DUCKDB_API static idx_t ComputeBitstringLen(idx_t len);

	DUCKDB_API static void RightShift(const string_t &bit_string, const idx_t &shif, string_t &result);
	DUCKDB_API static void LeftShift(const string_t &bit_string, const idx_t &shift, string_t &result);
	DUCKDB_API static void BitwiseAnd(const string_t &rhs, const string_t &lhs, string_t &result);
	DUCKDB_API static void BitwiseOr(const string_t &rhs, const string_t &lhs, string_t &result);
	DUCKDB_API static void BitwiseXor(const string_t &rhs, const string_t &lhs, string_t &result);
	DUCKDB_API static void BitwiseNot(const string_t &rhs, string_t &result);

	DUCKDB_API static void Verify(const string_t &input);

private:
	static void Finalize(string_t &str);
	static idx_t GetBitInternal(string_t bit_string, idx_t n);
	static void SetBitInternal(string_t &bit_string, idx_t n, idx_t new_value);
	static idx_t GetBitIndex(idx_t n);
};
} // namespace duckdb
