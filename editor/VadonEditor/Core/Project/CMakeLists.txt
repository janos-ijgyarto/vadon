add_subdirectory(Asset)

target_sources(${VADONEDITOR_CURRENT_TARGET}
    PRIVATE
    Project.hpp
    ProjectManager.hpp
    ProjectManager.cpp
   )