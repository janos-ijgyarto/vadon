# Set the folder so our targets are organized
set(PREV_CMAKE_FOLDER ${CMAKE_FOLDER})
set(CMAKE_FOLDER "${VADON_PARENT_FOLDER}Dependencies/glm")

CPMAddPackage(
  NAME glm
  VERSION 0.9.9.8
  URL https://github.com/g-truc/glm/archive/refs/tags/0.9.9.8.zip
)

# Reset folder
set(CMAKE_FOLDER ${PREV_CMAKE_FOLDER})