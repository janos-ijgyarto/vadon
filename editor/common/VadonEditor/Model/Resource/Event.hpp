#ifndef VADONEDITOR_MODEL_RESOURCE_EVENT_HPP
#define VADONEDITOR_MODEL_RESOURCE_EVENT_HPP
#include <Vadon/Model/Resource/Resource.hpp>
namespace VadonEditor::Model
{
	enum class ResourceEventType
	{
		ADDED,
		EDITED,
		RELOADED,
		REMOVED
	};

	struct ResourceEvent
	{
		Vadon::Model::ResourceID resource;
		ResourceEventType type;
		::Vadon::Foundation::UUID property_uuid;
	};
}
#endif