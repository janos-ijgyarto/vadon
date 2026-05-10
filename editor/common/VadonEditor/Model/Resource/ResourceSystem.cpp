#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

namespace VadonEditor::Model
{
	Resource* ResourceSystem::get_resource(const Vadon::Model::ResourceID& /*resource_id*/)
	{
		// TODO!
		return nullptr;
	}

	void ResourceSystem::register_edit_callback(EditCallback callback)
	{
		m_edit_callbacks.push_back(callback);
	}
}