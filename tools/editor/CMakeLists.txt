set(VADON_EDITOR "VadonEditor")

message(STATUS "Configuring Vadon Editor")

project(${VADON_EDITOR}
        VERSION 0.0.1
        DESCRIPTION "Vadon editor (using Qt)"
        LANGUAGES C CXX)

# NOTE: for tools, we want their binaries to end up in their own directories
set(VADON_EDITOR_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")

add_subdirectory(common)
add_subdirectory(application)