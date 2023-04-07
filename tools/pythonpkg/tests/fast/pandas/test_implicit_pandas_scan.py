# simple DB API testcase

import duckdb
import pandas
import pytest
from conftest import NumpyPandas, ArrowPandas
from semver import Version

numpy_nullable_df = pandas.DataFrame([{"COL1": "val1", "CoL2": 1.05},{"COL1": "val4", "CoL2": 17}])
if Version.parse(pandas.__version__) >= (2,0,0):
	pyarrow_df = numpy_nullable_df.convert_dtypes(dtype_backend="pyarrow")
else:
	# dtype_backend is not supported in pandas < 2.0.0
	pyarrow_df = numpy_nullable_df

class TestImplicitPandasScan(object):
    @pytest.mark.parametrize('pandas', [NumpyPandas(), ArrowPandas()])
    def test_local_pandas_scan(self, duckdb_cursor, pandas):
        con = duckdb.connect()
        df = pandas.DataFrame([{"COL1": "val1", "CoL2": 1.05},{"COL1": "val3", "CoL2": 17}])
        r1 = con.execute('select * from df').fetchdf()
        assert r1["COL1"][0] == "val1"
        assert r1["COL1"][1] == "val3"
        assert r1["CoL2"][0] == 1.05
        assert r1["CoL2"][1] == 17

    @pytest.mark.parametrize('pandas', [NumpyPandas(), ArrowPandas()])
    def test_global_pandas_scan(self, duckdb_cursor, pandas):
        con = duckdb.connect()
        r1 = con.execute(f'select * from {pandas.backend}_df').fetchdf()
        assert r1["COL1"][0] == "val1"
        assert r1["COL1"][1] == "val4"
        assert r1["CoL2"][0] == 1.05
        assert r1["CoL2"][1] == 17
