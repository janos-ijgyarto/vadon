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

# Install the DLL only
install(TARGETS freetype RUNTIME COMPONENT ${VADON_INSTALL_THIRD_PARTY_COMPONENT} DESTINATION ${VADON_INSTALL_RUNTIME_DESTINATION})

# Reset folder
set(CMAKE_FOLDER ${PREV_CMAKE_FOLDER})