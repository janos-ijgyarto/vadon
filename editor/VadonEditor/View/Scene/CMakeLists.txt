add_subdirectory(ECS)
add_subdirectory(Property)
add_subdirectory(Resource)

target_sources(${VADONEDITOR_CURRENT_TARGET}
    PRIVATE
    SceneTree.hpp
    SceneTree.cpp
   )