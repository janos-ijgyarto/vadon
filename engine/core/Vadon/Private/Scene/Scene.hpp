#ifndef VADON_PRIVATE_SCENE_SCENE_HPP
#define VADON_PRIVATE_SCENE_SCENE_HPP
#include <Vadon/Private/Scene/Resource/Resource.hpp>
#include <Vadon/Utilities/Data/Variant.hpp>
#include <Vadon/Utilities/TypeInfo/Registry/ErasedDataType.hpp>
namespace Vadon::Private::Scene
{
	using SceneComponent = Vadon::Scene::SceneComponent;

	struct SceneData : public Vadon::Scene::Scene
	{
		struct ComponentData
		{
			struct Property
			{
				std::string name;
				Vadon::Utilities::Variant value;
				Vadon::Utilities::ErasedDataTypeID data_type;
			};

			Vadon::Utilities::TypeID type_id = Vadon::Utilities::c_invalid_type_id;
			std::vector<Property> properties;
		};

		struct EntityData
		{
			std::string name;
			int32_t parent = -1;
			ResourceHandle scene;
			std::vector<ComponentData> components;

			bool has_parent() const { return (parent >= 0); }
		};

		// FIXME: optimize by splitting into arrays for data/strings/etc?
		std::vector<EntityData> entities;

		void clear()
		{
			entities.clear();
		}

		void swap(SceneData& other)
		{
			entities.swap(other.entities);
		}

		static void register_scene_type_info();
	};
}
#endif