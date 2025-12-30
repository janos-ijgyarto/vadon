add_subdirectory(Reflection)
add_subdirectory(TypeList)

target_sources(${VADON_CURRENT_TARGET}
	PRIVATE
	Registry.hpp
	Registry.cpp
	TypeErasure.hpp
	TypeErasure.cpp
	TypeInfo.hpp
	TypeName.hpp
	)