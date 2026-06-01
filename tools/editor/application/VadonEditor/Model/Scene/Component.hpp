#ifndef VADONEDITOR_MODEL_SCENE_COMPONENT_HPP
#define VADONEDITOR_MODEL_SCENE_COMPONENT_HPP
#include <VadonEditor/Core/Data/Object.hpp>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::Model
{
	class Scene;
	class Entity;

	using ComponentID = QUuid;
	class Component
	{
	public:
		Component(Core::Application& application);

		bool initialize(const QUuid& type_id);

		Core::Application& get_application() { return m_application; }

		const QUuid& get_type_id() const { return m_data.get_type_id(); }

		QVariant get_property(const Core::PropertyID& property_id) const { return m_data.get_property(property_id); }
		void set_property(const Core::PropertyID& property_id, const QVariant& value) { m_data.set_property(property_id, value); }

		static bool is_type_component(Core::Application& application, const QUuid& type_id);
	private:
		Core::Application& m_application;
		Core::DataObject m_data;
	};
}
#endif