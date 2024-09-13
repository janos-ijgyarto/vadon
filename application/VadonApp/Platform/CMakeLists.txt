add_subdirectory(Event)
add_subdirectory(Input)
add_subdirectory(Window)

target_sources(${VADONAPP_CURRENT_TARGET}
    PRIVATE
    Configuration.hpp
	PlatformInterface.hpp
    Module.hpp
   )