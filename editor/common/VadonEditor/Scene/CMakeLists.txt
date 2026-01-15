add_subdirectory(Resource)

target_sources(${VADONEDITOR_CURRENT_TARGET}
    PRIVATE
    Event.hpp
    Scene.hpp
    Scene.cpp
    SceneSystem.hpp
    SceneSystem.cpp
   )