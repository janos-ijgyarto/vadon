#include <VadonEditor/Model/ModelSystem.hpp>

#include <VadonEditor/Core/Editor.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>
#include <VadonEditor/Core/TypeInfo/MetadataRegistry.hpp>

#include <VadonEditor/Model/Resource/ResourceSystem.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>

#include <Vadon/Core/File/FileSystem.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>

#include <format>

namespace VadonEditor::Model
{
	struct ModelSystem::Internal
	{
		Vadon::ECS::World m_ecs_world;

		ResourceSystem m_resource_system;
		SceneSystem m_scene_system;

		Internal(Core::Editor& editor)
			: m_resource_system(editor)
			, m_scene_system(editor)
		{
		}

		bool initialize()
		{
			if (m_resource_system.initialize() == false)
			{
				return false;
			}

			if (m_scene_system.initialize() == false)
			{
				return false;
			}

			return true;
		}

		bool load_project(Core::Editor& editor)
		{
			Core::ProjectManager& project_manager = editor.get_system<Core::ProjectManager>();

			VADON_ASSERT(project_manager.get_state() == Core::ProjectManager::State::PROJECT_OPEN, "Project is in incorrect state!");
			if (m_resource_system.load_project_resources() == false)
			{
				Vadon::Core::Logger::log_error("Model system: unable to load project!\n");
				return false;
			}

			return true;
		}

		bool export_project(Core::Editor& editor, std::string_view output_path)
		{
			Core::ProjectManager& project_manager = editor.get_system<Core::ProjectManager>();

			VADON_ASSERT(project_manager.get_state() == Core::ProjectManager::State::PROJECT_LOADED, "Project is in incorrect state!");
			if (m_resource_system.export_project_resources(output_path) == false)
			{
				Vadon::Core::Logger::log_error("Model system: unable to load project!\n");
				return false;
			}

			return true;
		}
	};

	ModelSystem::~ModelSystem() = default;

	Vadon::ECS::World& ModelSystem::get_ecs_world()
	{
		return m_internal->m_ecs_world;
	}

	ResourceSystem& ModelSystem::get_resource_system()
	{
		return m_internal->m_resource_system;
	}
	
	SceneSystem& ModelSystem::get_scene_system()
	{
		return m_internal->m_scene_system;
	}

	Property ModelSystem::get_editor_property(Vadon::Utilities::TypeID owner_type_id, const Vadon::Utilities::Property& property) const
	{
		VADON_ASSERT(owner_type_id != Vadon::Utilities::TypeID::INVALID, "Invalid type ID!");

		Property editor_property;
		editor_property.id = property.info.id;

		// By default, assume trivial property
		const Vadon::Utilities::TypeID property_type_id = Vadon::Utilities::TypeRegistry::get_type_id(property.info.type);
		editor_property.type = { .category = VadonEditor::Model::PropertyDataType::TRIVIAL, .type_id = property_type_id };

		const ::Vadon::Foundation::TypeInfo owner_type_info = Vadon::Utilities::TypeRegistry::get_type_info(owner_type_id);
		VadonEditor::Core::MetadataRegistry& metadata_registry = m_editor.get_metadata_registry();

		const char* name_str = metadata_registry.get_property_metadata(owner_type_info.id, property.info.id, "name");
		editor_property.label = name_str != nullptr ? name_str : "";
		// TODO: have proper fallbacks!
		if (editor_property.label.empty())
		{
			VADON_ERROR("Must provide type metadata!");
		}

		const Vadon::Utilities::TypeID uuid_type_id = Vadon::Utilities::TypeRegistry::get_type_id<::Vadon::Foundation::UUID>();
		const Vadon::Utilities::TypeID array_type_id = Vadon::Utilities::TypeRegistry::get_type_id<Vadon::Utilities::BoxedVariantArray>();

		if (property_type_id == uuid_type_id)
		{
			// UUID type, check metadata if it's a resource ID
			const char* resource_uuid_str = metadata_registry.get_property_metadata(owner_type_info.id, property.info.id, "resource");
			if (resource_uuid_str != nullptr)
			{
				Vadon::Utilities::TypeUUID resource_type_uuid;
				if (Vadon::Utilities::uuid_from_base64_string(resource_uuid_str, resource_type_uuid) == true)
				{
					// TODO: make sure type is actually a valid resource type!
					editor_property.type = { .category = VadonEditor::Model::PropertyDataType::RESOURCE_ID, .type_id = Vadon::Utilities::TypeRegistry::get_type_id(resource_type_uuid) };
				}
			}
		}
		else if (property_type_id == array_type_id)
		{
			const char* array_str = metadata_registry.get_property_metadata(owner_type_info.id, property.info.id, "array");
			if (array_str != nullptr)
			{
				const std::string_view array_str_view = array_str;
				const size_t resource_uuid_offset = array_str_view.find("resource:");
				if (resource_uuid_offset != std::string::npos)
				{
					// Array holds Resource IDs
					const std::string_view resource_uuid_string = array_str_view.substr(resource_uuid_offset);
					Vadon::Utilities::TypeUUID resource_type_uuid;
					if (Vadon::Utilities::uuid_from_base64_string(resource_uuid_string, resource_type_uuid) == true)
					{
						editor_property.type = { .category = VadonEditor::Model::PropertyDataType::ARRAY, .type_id = Vadon::Utilities::TypeRegistry::get_type_id(resource_type_uuid) };
					}
					else
					{
						VADON_ERROR("Must provide type metadata!");
					}
				}
				else
				{
					// Expect a regular type UUID
					Vadon::Utilities::TypeUUID array_type_uuid;
					if (Vadon::Utilities::uuid_from_base64_string(array_str_view, array_type_uuid) == true)
					{
						// TODO: make sure it's a base type!
						editor_property.type = { .category = VadonEditor::Model::PropertyDataType::ARRAY, .type_id = Vadon::Utilities::TypeRegistry::get_type_id(array_type_uuid) };
					}
					else
					{
						VADON_ERROR("Must provide type metadata!");
					}
				}
			}
		}

		editor_property.value = property.value;

		return editor_property;
	}

	ModelSystem::ModelSystem(Core::Editor& editor)
		: System(editor)
		, m_internal(std::make_unique<Internal>(editor))
	{

	}

	bool ModelSystem::initialize()
	{
		return m_internal->initialize();
	}

	bool ModelSystem::load_project()
	{
		return m_internal->load_project(m_editor);
	}

	bool ModelSystem::export_project(std::string_view output_path)
	{
		return m_internal->export_project(m_editor, output_path);
	}
}