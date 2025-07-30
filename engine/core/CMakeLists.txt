MESSAGE("Configuring Vadon Core")

VadonCreateEngineTarget("VadonCore" "Vadon" "VADONCORE_EXPORTS")

set(VADON_CORE_LIBRARIES Vadon::Render)
target_link_libraries(${VADON_CURRENT_TARGET} PUBLIC VadonGraphicsAPI)