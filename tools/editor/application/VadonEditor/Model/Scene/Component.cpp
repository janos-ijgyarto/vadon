#include <VadonEditor/Model/Scene/Component.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Utilities/UUID.hpp>
#include <VadonEditor/Utilities/Data/Variant.hpp>

#include <Vadon/Foundation/Model/Scene/Scene.hpp>

namespace VadonEditor::Model
{
	Component::Component(Core::Application& application)
		: m_application(application)
		, m_data(application)
	{
	}

	bool Component::initialize(const QUuid& type_id)
	{
		return m_data.default_initialize(type_id);
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