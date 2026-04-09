#include <VadonEditor/Model/Scene/Component.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

#include <Vadon/Foundation/Model/Scene/Scene.hpp>

namespace VadonEditor::Model
{
	bool Component::save_data(Core::Application& application, const QUuid& type_id, QVariant& data) const
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

				// TODO: if it's a UUID, convert to base64 string!
				// Also make use of the type metadata (e.g add label to UUID)

				property_map.insert(property_key_string, property_it.value());
			}
		}

		data = component_data_map;
		return true;
	}

	QUuid Component::load_data(Core::Application& application, const QVariant& data)
	{
		Q_UNUSED(application);
		Q_UNUSED(data);
		return QUuid();
	}
}