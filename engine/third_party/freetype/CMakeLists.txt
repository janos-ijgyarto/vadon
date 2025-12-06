# Set the folder so our targets are organized
set(PREV_CMAKE_FOLDER ${CMAKE_FOLDER})
set(CMAKE_FOLDER "Dependencies/FreeType")

# Have to install manually so we only get what we need
set(SKIP_INSTALL_ALL ON)

CPMAddPackage(
  NAME freetype
  URL https://github.com/freetype/freetype/archive/refs/tags/VER-2-13-0.zip
  VERSION 2.13.0
)

set_target_properties(freetype
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Reset folder
set(CMAKE_FOLDER ${PREV_CMAKE_FOLDER})