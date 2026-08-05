#include <VadonEditor/Model/Resource/Resource.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <Vadon/Model/Resource/ResourceSystem.hpp>

#include <Vadon/Utilities/Data/Object.hpp>
#include <Vadon/Utilities/Debugging/Assert.hpp>
#include <Vadon/Utilities/Serialization/Serializer.hpp>

namespace VadonEditor::Model
{
	Resource::~Resource()
	{
		VADON_ASSERT(is_loaded() == false, "Did not clean up engine resource!");
		VADON_ASSERT(m_embedded_resources.empty() == true, "Did not clean up embedded resources!");
	}

	Resource* Resource::find_embedded_resource(const Vadon::Model::ResourceID& resource_id) const
	{
		for (Resource* current_embedded : m_embedded_resources)
		{
			if (current_embedded->get_id() == resource_id)
			{
				return current_embedded;
			}
		}

		return nullptr;
	}

	Resource::Resource(Core::Editor& editor, const Vadon::Model::ResourceID& id)
		: m_editor(editor)
		, m_id(id)
		, m_owner(nullptr)
	{

	}

	bool Resource::initialize()
	{
		Vadon::Model::ResourceSystem& engine_resource_system = m_editor.get_engine_core().get_system<Vadon::Model::ResourceSystem>();
		if (is_loaded() == false)
		{
			m_handle = engine_resource_system.load_resource_base(m_id);
			if (m_handle.is_valid() == false)
			{
				return false;
			}
		}

		m_info = engine_resource_system.get_resource_info(m_handle);
		return true;
	}

	void Resource::shutdown()
	{
		m_handle.invalidate();
		m_embedded_resources.clear();
	}

	void Resource::load_property_data(const ::Vadon::Foundation::EditorModelMessageResourcePropertyEdited& resource_property_message, const char* data)
	{
		VADON_ASSERT(is_loaded() == true, "Resource is not loaded!");
		const void* data_start = data + sizeof(::Vadon::Foundation::EditorModelMessageResourcePropertyEdited);

		Vadon::Core::RawFileDataBuffer json_data_buffer;
		json_data_buffer.resize(resource_property_message.data_size);

		memcpy(std::to_address(json_data_buffer.begin()), data_start, resource_property_message.data_size);

		Vadon::Utilities::Serializer::Instance serializer_instance = Vadon::Utilities::Serializer::create_serializer(json_data_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::READ);
		if (serializer_instance->initialize() == false)
		{
			VADON_ERROR("Failed to deserialize message data!");
			return;
		}

		if (load_property_data(*serializer_instance) == false)
		{
			VADON_ERROR("Failed to load property data!");
			return;
		}

		if (serializer_instance->finalize() == false)
		{
			VADON_ERROR("Failed to finalize message serializer!");
			return;
		}
	}

	bool Resource::load_property_data(Vadon::Utilities::Serializer& serializer)
	{
		Vadon::Model::ResourceSystem& engine_resource_system = m_editor.get_engine_core().get_system<Vadon::Model::ResourceSystem>();
		Vadon::Model::Resource* engine_resource = engine_resource_system.get_base_resource(m_handle);

		Vadon::Utilities::VariantDictionary resource_properties;

		if (Vadon::Utilities::ObjectSerializer::serialize_object_properties(serializer, m_info.type_id, resource_properties) == false)
		{
			VADON_ERROR("Failed to deserialize resource property data!");
			return false;
		}

		Vadon::Utilities::ObjectWrapper resource_object_wrapper(m_info.type_id, engine_resource);
		if (Vadon::Utilities::ObjectSerializer::load_object_property_data(resource_object_wrapper, resource_properties) == false)
		{
			VADON_ERROR("Failed to load property data into resource!");
			return false;
		}

		const Vadon::Utilities::Serializer::KeyVector property_keys = serializer.get_keys();
		ResourceSystem& resource_system = m_editor.get_resource_system();

		ResourceEvent property_edit_event;
		property_edit_event.resource = m_id;
		property_edit_event.type = ResourceEventType::EDITED;

		for (const std::string& current_key : property_keys)
		{
			const Vadon::Foundation::UUID current_property_id = Vadon::Utilities::parse_labeled_uuid(current_key);

			property_edit_event.property_uuid = current_property_id;
			resource_system.broadcast_resource_event(property_edit_event);

			Vadon::Core::Logger::log_message(std::format("Modified resource {} property {}\n", Vadon::Utilities::uuid_to_string(get_id()).string, Vadon::Utilities::uuid_to_string(current_property_id).string));
		}

		return true;
	}

	bool Resource::add_embedded_resource(Resource* resource)
	{
		VADON_ASSERT(is_loaded() == true, "Resource is not loaded!");
		VADON_ASSERT(resource->is_loaded() == true, "Embedded resource is not loaded!");
		VADON_ASSERT(resource->is_embedded() == false, "Resource is already embedded!");

		// TODO: also check that embedded already exists in engine resource system?
		if (find_embedded_resource(resource->get_id()) != nullptr)
		{
			// TODO: log error?
			VADON_ERROR("Resource is already embedded in parent!");
			return false;
		}

		Vadon::Model::ResourceSystem& engine_resource_system = m_editor.get_engine_core().get_system<Vadon::Model::ResourceSystem>();
		engine_resource_system.add_embedded_resource(m_handle, resource->m_handle);
				
		m_embedded_resources.push_back(resource);
		return true;
	}

	void Resource::remove_embedded_resource(Resource* resource)
	{
		for (auto resource_it = m_embedded_resources.begin(); resource_it != m_embedded_resources.end(); ++resource_it)
		{
			Resource* current_embedded = *resource_it;
			if (current_embedded == resource)
			{
				m_embedded_resources.erase(resource_it);
				return;
			}
		}

		VADON_UNREACHABLE;
	}
}