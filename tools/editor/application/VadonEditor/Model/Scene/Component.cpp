#include <VadonEditor/Model/Scene/Component.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Utilities/UUID.hpp>
#include <VadonEditor/Utilities/Data/Variant.hpp>

#include <Vadon/Foundation/Model/Scene/Scene.hpp>

namespace VadonEditor::Model
{
	bool Component::initialize(Core::Application& application)
	{
		properties.clear();

		const Core::DataSchema& data_schema = application.get_project_manager().get_project_data_schema();
		const Core::TypeData* component_type_data = data_schema.find_type_data(type_id);

		while (component_type_data != nullptr)
		{
			for (auto property_it = component_type_data->properties.begin(); property_it != component_type_data->properties.end(); ++property_it)
			{
				const ::Vadon::Foundation::BaseType base_type = Core::TypeData::get_base_type(Utilities::vadon_uuid_to_qt_uuid(property_it->info.type));
				properties.insert(property_it.key(), Utilities::get_base_type_default_value(base_type));
			}

			if (component_type_data->info.base_id.is_valid() == true)
			{
				component_type_data = data_schema.find_type_data(Utilities::vadon_uuid_to_qt_uuid(component_type_data->info.base_id));
			}
			else
			{
				component_type_data = nullptr;
			}
		}

		return true;
	}

	bool Component::save_data(Core::Application& application, QVariant& data) const
	{
		QVariantMap component_data_map;

		const Core::DataSchema& data_schema = application.get_project_manager().get_project_data_schema();
		const Core::TypeData* component_type_data = data_schema.find_type_data(type_id);
		Q_ASSERT_X(component_type_data != nullptr, "VadonEditor::Model::Component::save_data", "Type data not found");

		{
			QString component_type_string = component_type_data->find_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME);
			if (component_type_string.isEmpty() == false)
			{
				component_type_string = Utilities::serialize_labeled_uuid(component_type_string, type_id);
			}
			else
			{
				component_type_string = Utilities::uuid_to_base64_string(type_id);
			}

			const QUuid type_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntityComponentSchema::c_type_property.id);
			component_data_map.insert(Utilities::serialize_labeled_uuid("type", type_property_uuid), component_type_string);
		}

		{
			QVariantMap property_map;
			for (auto property_it = properties.begin(); property_it != properties.end(); ++property_it)
			{
				// TODO: check whether the current value is different from the default!
				const Core::PropertyData* property_data = component_type_data->find_property_data(property_it.key());
				Q_ASSERT_X(property_data != nullptr, "VadonEditor::Model::Component::save_data", "Property data not found");

				QString property_key_string = property_data->find_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME);
				if (property_key_string.isEmpty() == false)
				{
					property_key_string = Utilities::serialize_labeled_uuid(property_key_string, property_it.key());
				}
				else
				{
					property_key_string = Utilities::uuid_to_base64_string(property_it.key());
				}

				const QVariant serialized_property = data_schema.serialize_property_data(*property_data, property_it.value());
				property_map.insert(property_key_string, Utilities::save_variant_to_json_variant(serialized_property));
			}

			const QUuid component_properties_property = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntityComponentSchema::c_properties_property.id);
			component_data_map.insert(Utilities::serialize_labeled_uuid("properties", component_properties_property), property_map);
		}

		data = component_data_map;
		return true;
	}

	bool Component::load_data(Core::Application& application, const QVariant& data)
	{
		const QVariantMap component_data_map = data.toMap();

		QVariantMap component_properties;

		const QUuid type_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntityComponentSchema::c_type_property.id);
		const QUuid component_properties_property = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntityComponentSchema::c_properties_property.id);

		for(auto component_data_it = component_data_map.begin(); component_data_it != component_data_map.end(); ++component_data_it)
		{
			const QUuid entry_uuid = Utilities::parse_labeled_uuid(component_data_it.key());
			if (entry_uuid == type_property_uuid)
			{
				const QUuid component_type_uuid = Utilities::base64_string_to_uuid(component_data_it.value().toString());
				if (type_id.isNull() == false)
				{
					Q_ASSERT_X(type_id == component_type_uuid, "VadonEditor::Model::Component::load_data", "Invalid type UUID");
					continue;
				}
				else
				{
					type_id = component_type_uuid;
				}
			}
			else if (entry_uuid == component_properties_property)
			{
				component_properties = component_data_it.value().toMap();
			}
		}

		if (type_id.isNull() == true)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::Component::load_data", "Failed to load component type ID");
			return false;
		}

		if (initialize(application) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::Component::load_data", "Failed to initialize component");
			return false;
		}

		const Core::DataSchema& data_schema = application.get_project_manager().get_project_data_schema();
		const Core::TypeData* component_type_data = data_schema.find_type_data(type_id);

		for (auto property_it = component_properties.begin(); property_it != component_properties.end(); ++property_it)
		{
			// TODO: check whether the current value is different from the default!
			const QUuid property_uuid = Utilities::parse_labeled_uuid(property_it.key());
			auto property_data_it = properties.find(property_uuid);
			if (property_data_it != properties.end())
			{
				const Core::PropertyData* type_property_data = component_type_data->find_property_data(property_uuid);
				Q_ASSERT_X(type_property_data != nullptr, "VadonEditor::Model::Component::load_data", "Cannot find property data");

				const ::Vadon::Foundation::BaseType base_type = Core::TypeData::get_base_type(Utilities::vadon_uuid_to_qt_uuid(type_property_data->info.type));
				const QVariant property_variant = Utilities::get_variant_from_json_variant(Utilities::get_qt_typeid_from_base_type(base_type), property_it.value());

				property_data_it.value() = data_schema.deserialize_property_data(*type_property_data, property_variant);
			}
			else
			{
				// TODO: warn about obsolete data?
			}
		}

		return true;
	}

	QVariant Component::get_property(const QUuid& property_id) const
	{
		auto property_it = properties.find(property_id);
		Q_ASSERT_X(property_it != properties.end(), "VadonEditor::Model::Component::get_property", "Cannot find property");

		return property_it.value();
	}

	void Component::set_property(const QUuid& property_id, const QVariant& value)
	{
		auto property_it = properties.find(property_id);
		Q_ASSERT_X(property_it != properties.end(), "VadonEditor::Model::Component::set_property", "Cannot find property");
		Q_ASSERT_X(property_it.value().typeId() == value.typeId(), "VadonEditor::Model::Component::set_property", "Property value type mismatch");

		property_it.value() = value;
	}

	bool Component::is_type_component(Core::Application& application, const QUuid& type_id)
	{
		const Core::DataSchema& data_schema = application.get_project_manager().get_project_data_schema();
		const Core::TypeData* type_data = data_schema.find_type_data(type_id);
		
		if (type_data == nullptr)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::Component::is_type_component", "Cannot find type data");
			return false;
		}

		const QString component_metadata = type_data->find_metadata(::Vadon::Foundation::CommonTypeMetadata::COMPONENT);
		return component_metadata.isEmpty() == false;
	}
}