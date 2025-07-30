MESSAGE("Configuring Vadon Graphics API")

VadonCreateEngineTarget("VadonGraphicsAPI" "GraphicsAPI" "VADONGRAPHICSAPI_EXPORTS")

set(VADON_GRAPHICS_API_LIBRARIES Vadon::Render)
target_link_libraries(${VADON_CURRENT_TARGET} PUBLIC ${VADON_GRAPHICS_API_LIBRARIES})