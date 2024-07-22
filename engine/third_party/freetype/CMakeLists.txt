# Set the folder so our targets are organized
set(PREV_CMAKE_FOLDER ${CMAKE_FOLDER})
set(CMAKE_FOLDER "Dependencies/FreeType")

CPMAddPackage(
  NAME freetype
  URL https://github.com/freetype/freetype/archive/refs/tags/VER-2-13-0.zip
  VERSION 2.13.0
)

# Reset folder
set(CMAKE_FOLDER ${PREV_CMAKE_FOLDER})