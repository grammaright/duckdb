git checkout v1.0.0 src/storage/                
git checkout v1.0.0 extension/jemalloc/         
git checkout v1.0.0 tools/sqlite3_api_wrapper/  

sed -i '11i\BufferManager\:\:BufferManager\(\)\ \{\}' src/storage/buffer_manager.cpp