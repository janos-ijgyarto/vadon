MESSAGE("Configuring Vadon Common")

include(Build/Common)

VadonCreateEngineTarget("VadonCommon" "Common" "VADONCOMMON_EXPORTS")

set(VADON_PUBLIC_THIRD_PARTY concurrentqueue glm::glm nlohmann_json)
target_link_libraries(${VADON_CURRENT_TARGET} PUBLIC ${VADON_PUBLIC_THIRD_PARTY})