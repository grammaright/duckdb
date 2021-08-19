# this script generates data for the TPC-H dbgen
import os
from python_helpers import open_utf8

def get_csv_text(fpath, add_null_terminator = False):
	with open(fpath, 'rb') as f:
		text = bytearray(f.read())
	result_text = ""
	first = True
	for byte in text:
		if first:
			result_text += str(byte)
		else:
			result_text += ", " + str(byte)
		first = False
	if add_null_terminator:
		result_text += ", 0"
	return result_text

def write_dir(dirname, varname):
	files = os.listdir(dirname)
	files.sort()
	result = ""
	aggregated_result =  "const char *%s[] = {\n" % (varname,)
	for fname in files:
		file_varname = "%s_%s" % (varname,fname.split('.')[0])
		result += "const uint8_t %s[] = {" % (file_varname,) + get_csv_text(os.path.join(dirname, fname), True) + "};\n"
		aggregated_result += "\t(const char*) %s,\n" % (file_varname,)
	aggregated_result = aggregated_result[:-2] + "\n};\n"
	return result + aggregated_result

# ------------------------------------------- #
# ------------------------------------------- #
# -------------      TPC-H       ------------ #
# ------------------------------------------- #
# ------------------------------------------- #
tpch_dir = 'extension/tpch/dbgen'
tpch_queries = os.path.join(tpch_dir, 'queries')
tpch_answers_sf001 = os.path.join(tpch_dir, 'answers', 'sf0.01')
tpch_answers_sf01 = os.path.join(tpch_dir, 'answers', 'sf0.1')
tpch_answers_sf1 = os.path.join(tpch_dir, 'answers', 'sf1')
tpch_header = os.path.join(tpch_dir, 'include', 'tpch_constants.hpp')

def create_tpch_header(tpch_dir):
	result = """/* THIS FILE WAS AUTOMATICALLY GENERATED BY generate_csv_header.py */

#pragma once

const int TPCH_QUERIES_COUNT = 22;
"""
	# write the queries
	result += write_dir(tpch_queries, "TPCH_QUERIES")
	result += write_dir(tpch_answers_sf001, "TPCH_ANSWERS_SF0_01")
	result += write_dir(tpch_answers_sf01, "TPCH_ANSWERS_SF0_1")
	result += write_dir(tpch_answers_sf1, "TPCH_ANSWERS_SF1")

	with open_utf8(tpch_header, 'w+') as f:
		f.write(result)

print(tpch_header)
create_tpch_header(tpch_dir)

# ------------------------------------------- #
# ------------------------------------------- #
# -------------      TPC-DS      ------------ #
# ------------------------------------------- #
# ------------------------------------------- #
tpcds_dir = 'extension/tpcds/dsdgen'
tpcds_queries = os.path.join(tpcds_dir, 'queries')
tpcds_schema = os.path.join(tpcds_dir, 'schema')
tpcds_answers_sf001 = os.path.join(tpcds_dir, 'answers', 'sf0.01')
tpcds_answers_sf1 = os.path.join(tpcds_dir, 'answers', 'sf1')
tpcds_header = os.path.join(tpcds_dir, 'include', 'tpcds_constants.hpp')

def create_tpcds_header(tpch_dir):
	result = """/* THIS FILE WAS AUTOMATICALLY GENERATED BY generate_csv_header.py */

#pragma once

const int TPCDS_QUERIES_COUNT = 99;
const int TPCDS_TABLE_COUNT = 24;
"""
	# write the queries
	result += write_dir(tpcds_queries, "TPCDS_QUERIES")
	result += write_dir(tpcds_answers_sf001, "TPCDS_ANSWERS_SF0_01")
	result += write_dir(tpcds_answers_sf1, "TPCDS_ANSWERS_SF1")
	result += write_dir(tpcds_schema, "TPCDS_TABLE_DDL_NOKEYS")

	with open_utf8(tpcds_header, 'w+') as f:
		f.write(result)

print(tpcds_header)
create_tpcds_header(tpcds_dir)
