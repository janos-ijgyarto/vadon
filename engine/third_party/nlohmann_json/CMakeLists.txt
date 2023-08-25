CPMAddPackage(
  NAME nlohmann_json
  VERSION 3.11.2
  URL https://github.com/nlohmann/json/releases/download/v3.11.2/include.zip
  URL_HASH SHA256=e5c7a9f49a16814be27e4ed0ee900ecd0092bfb7dbfca65b5a421b774dccaaed
  DOWNLOAD_ONLY True
)

if (nlohmann_json_ADDED)
  add_library(nlohmann_json INTERFACE)
  target_include_directories(nlohmann_json INTERFACE ${nlohmann_json_SOURCE_DIR}/include)
endif()