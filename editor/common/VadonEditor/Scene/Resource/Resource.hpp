#ifndef VADONEDITOR_SCENE_RESOURCE_RESOURCE_HPP
#define VADONEDITOR_SCENE_RESOURCE_RESOURCE_HPP
#include <Vadon/Scene/Resource/Resource.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/Property.hpp>
namespace VadonEditor::Core
{
	struct Property;
}
namespace VadonEditor::Scene
{
	class Resource
	{
	public:
		bool load();

		// TODO: also implement reference counting
		// Each time something is loaded that references this resource
		// add a reference
		// Allows us to unload when nothing references it anymore

		std::vector<Core::Property> get_properties() const;

		Vadon::Utilities::Variant get_property(const Vadon::Utilities::PropertyUUID& property_uuid) const;
		void set_property(const Vadon::Utilities::PropertyUUID& property_uuid, const Vadon::Utilities::Variant& value);
	private:
		Vadon::Scene::ResourceID m_id;
		Vadon::Scene::ResourceHandle m_handle;
	};
}
#endif