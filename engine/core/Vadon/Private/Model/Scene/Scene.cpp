#include <Vadon/Private/Model/Scene/Scene.hpp>

#include <Vadon/Model/Resource/Registry.hpp>

namespace Vadon::Private::Model
{
	void EntityData::set_components(const std::vector<ComponentData>& component_vec)
	{
		// Clear previous components
		clear_component_data();
		components = component_vec;
	}

	void EntityData::clear_component_data()
	{
		// We need to destroy each component in each entity, since they were heap-allocated
		// FIXME: use refcounting and/or some kind of allocator strategy to make this more robust!
		for (Vadon::Private::Model::ComponentData& current_component : components)
		{
			Vadon::Utilities::TypeRegistry::destroy_object(current_component);
		}

		components.clear();
	}
}