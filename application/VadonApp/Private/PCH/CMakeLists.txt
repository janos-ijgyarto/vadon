target_sources(${VADONAPP_CURRENT_TARGET}
    PRIVATE
    VadonApp.hpp
   )

target_precompile_headers(${VADONAPP_CURRENT_TARGET} PRIVATE VadonApp.hpp)