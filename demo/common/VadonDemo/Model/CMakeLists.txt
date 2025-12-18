target_sources(${VADONDEMO_CURRENT_TARGET}
    PRIVATE
    Component.hpp
    Component.cpp
    Model.hpp
    Model.cpp
    Resource.hpp
    Resource.cpp
    )

add_subdirectory(Collision)
add_subdirectory(Enemy)
add_subdirectory(Weapon)