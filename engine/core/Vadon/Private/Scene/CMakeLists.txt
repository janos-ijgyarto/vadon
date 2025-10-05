add_subdirectory(Animation)
add_subdirectory(Resource)

target_sources(${VADON_CURRENT_TARGET}
    PRIVATE
        Scene.hpp
        SceneSystem.hpp
        SceneSystem.cpp
   )