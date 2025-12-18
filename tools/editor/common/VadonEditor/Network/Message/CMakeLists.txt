add_subdirectory(Window)

target_sources(${VADONEDITOR_COMMON}
    PRIVATE
    Message.hpp
    MessageSerializer.hpp
    MessageSerializer.cpp
)