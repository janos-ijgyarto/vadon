add_subdirectory(Core)
add_subdirectory(Model)
add_subdirectory(Platform)
add_subdirectory(UI)
add_subdirectory(View)

target_sources(${VADONEDITOR_CURRENT_TARGET}
    PRIVATE
	VadonEditor.hpp
   )