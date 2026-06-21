#include <VadonEditor/Model/Resource/Resource.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <Vadon/Model/Resource/ResourceSystem.hpp>

#include <Vadon/Utilities/Data/Object.hpp>
#include <Vadon/Utilities/Debugging/Assert.hpp>
#include <Vadon/Utilities/Serialization/Serializer.hpp>

namespace VadonEditor::Model
{
	Resource::Resource(Core::Editor& editor, const Vadon::Model::ResourceID& id)
		: m_editor(editor)
		, m_id(id)
		, m_owner(nullptr)
	{

	}

	bool Resource::internal_load()
	{
		VADON_ASSERT(m_handle.is_valid() == false, "Resource is already loaded!");

		Vadon::Model::ResourceSystem& engine_resource_system = m_editor.get_engine_core().get_system<Vadon::Model::ResourceSystem>();
		m_handle = engine_resource_system.load_resource_base(m_id);

		if (m_handle.is_valid() == false)
		{
			return false;
		}

		m_info = engine_resource_system.get_resource_info(m_handle);
		return true;
	}

	void Resource::load_property_data(const ::Vadon::Foundation::EditorModelMessageResourcePropertyEdited& resource_property_message, const char* data)
	{
		VADON_ASSERT(m_handle.is_valid() == true, "Resource is not loaded!");
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

		Vadon::Utilities::ObjectPointer resource_object_pointer{ .type = m_info.type_id, .data = engine_resource };

		if (Vadon::Utilities::DataObject::serialize_object_properties(serializer, resource_object_pointer) == false)
		{
			VADON_ERROR("Failed to deserialize resource properties!");
			return false;
		}

		const Vadon::Utilities::Serializer::KeyVector property_keys = serializer.get_keys();
		for (const std::string& current_key : property_keys)
		{
			const Vadon::Foundation::UUID current_property_id = Vadon::Utilities::parse_labeled_uuid(current_key);
			m_editor.get_resource_system().resource_property_edited(m_id, current_property_id);

			Vadon::Core::Logger::log_message(std::format("Modified resource {} property {}\n", Vadon::Utilities::uuid_to_string(get_id()).string, Vadon::Utilities::uuid_to_string(current_property_id).string));
		}

		return true;
	}
}