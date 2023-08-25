add_subdirectory(Core)
add_subdirectory(Platform)
add_subdirectory(Private)
add_subdirectory(UI)
add_subdirectory(Utilities)

target_sources(${VADONAPP_CURRENT_TARGET}
    PRIVATE
	VadonApp.hpp
   )