#ifndef VADON_PRIVATE_SCENE_RESOURCE_RESOURCESYSTEM_HPP
#define VADON_PRIVATE_SCENE_RESOURCE_RESOURCESYSTEM_HPP
#include <Vadon/Scene/Resource/ResourceSystem.hpp>
#include <Vadon/Private/Scene/Resource/Resource.hpp>

#include <Vadon/Utilities/Container/ObjectPool/Pool.hpp>
namespace Vadon::Private::Scene
{
	class ResourceSystem final : public Vadon::Scene::ResourceSystem
	{
	public:
		const ResourceBase* get_resource_base(ResourceHandle resource_handle) const override;

		ResourceHandle find_resource(ResourceID resource_id) const override;

		ResourceInfo get_resource_info(ResourceHandle resource_handle) const override;

		void set_resource_path(ResourceHandle resource_handle, const ResourcePath& path) override;

		std::vector<ResourceHandle> find_resources_of_type(Vadon::Utilities::TypeID type_id) const override;

		bool import_resource_library(Vadon::Core::RootDirectoryHandle root_directory, std::string_view extensions = "") override;
		ResourceHandle import_resource_file(const ResourcePath& path) override;
		ResourceHandle import_resource(Vadon::Utilities::Serializer& serializer) override;

		bool save_resource(ResourceHandle resource_handle) override;
		bool load_resource(ResourceHandle resource_handle) override;

		bool serialize_resource(Vadon::Utilities::Serializer& serializer, ResourceHandle resource_handle) override;

		bool serialize_resource_property(Vadon::Utilities::Serializer& serializer, std::string_view property_name, ResourceHandle& property_value) override;

		void register_resource_file_extension(std::string_view extension) override { m_resource_file_extensions += ","; m_resource_file_extensions += extension; }
	protected:
		ResourceHandle internal_create_resource(ResourceBase* resource, Vadon::Utilities::TypeID type_id) override;
	private:
		struct ResourceData
		{
			ResourceInfo info;
			ResourceBase* resource = nullptr;

			bool is_loaded() const { return resource != nullptr; }
		};

		ResourceSystem(Vadon::Core::EngineCoreInterface& core);

		bool initialize();
		void shutdown();

		bool internal_load_resource(Vadon::Utilities::Serializer& serializer, ResourceHandle resource_handle);
		bool internal_save_resource(Vadon::Utilities::Serializer& serializer, ResourceHandle resource_handle);

		ResourceHandle internal_register_resource(ResourceID resource_id, Vadon::Utilities::TypeID type_id);
		bool internal_create_resource(ResourceData& resource_data);

		bool has_resource(ResourceHandle resource_handle) const;

		using ResourcePool = Vadon::Utilities::ObjectPool<Vadon::Scene::Resource, ResourceData>;
		ResourcePool m_resource_pool;

		std::unordered_map<ResourceID, ResourceHandle> m_resource_lookup;
		std::string m_resource_file_extensions;

		friend class SceneSystem;
	};
}
#endif