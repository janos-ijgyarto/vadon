add_subdirectory(Node)

target_sources(${VADON_CURRENT_TARGET}
    PRIVATE
        Module.hpp
        Scene.hpp
        SceneSystem.hpp
   )