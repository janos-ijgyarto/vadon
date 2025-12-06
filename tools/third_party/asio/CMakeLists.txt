CPMAddPackage(
  NAME asio
  VERSION 1-36-0
  URL https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-36-0.zip
  DOWNLOAD_ONLY YES
)

if (asio_ADDED)
	# asio has no CMake support, so we create our own target
	add_library(asio STATIC Asio.cpp)
	target_include_directories(asio PUBLIC "${asio_SOURCE_DIR}/asio/include")
	target_compile_definitions(asio PUBLIC ASIO_SEPARATE_COMPILATION)
	set_target_properties(asio PROPERTIES FOLDER "ThirdParty")
endif()