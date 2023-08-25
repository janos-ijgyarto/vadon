CPMAddPackage(
  NAME imgui
  VERSION 1.89.8
  URL https://github.com/ocornut/imgui/archive/refs/tags/v1.89.8.zip
  DOWNLOAD_ONLY True
)

# ImGui doesn't come with CMake so we have to do it ourselves
if (imgui_ADDED)
	# Set the folder so our targets are organized
	set(PREV_CMAKE_FOLDER ${CMAKE_FOLDER})
	set(CMAKE_FOLDER "${VADON_PARENT_FOLDER}Dependencies/ImGui")

	# If we added this folder, we set the preprocessor definition to use ImGui
	target_compile_definitions(${VADONAPP_CURRENT_TARGET} PRIVATE VADON_DEVELOPER_GUI_IMGUI)

	add_library(imgui STATIC)
	target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})

	target_sources(imgui
		PRIVATE
		${imgui_SOURCE_DIR}/imgui_demo.cpp
		${imgui_SOURCE_DIR}/imgui_draw.cpp
		${imgui_SOURCE_DIR}/imgui_tables.cpp
		${imgui_SOURCE_DIR}/imgui_widgets.cpp
		${imgui_SOURCE_DIR}/imgui.cpp
		${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.h
		${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
	)

	# Add to application
	target_link_libraries(${VADONAPP_CURRENT_TARGET} PRIVATE imgui)

	# Reset folder
	set(CMAKE_FOLDER ${PREV_CMAKE_FOLDER})
endif()