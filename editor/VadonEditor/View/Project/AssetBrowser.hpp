#ifndef VADONEDITOR_VIEW_PROJECT_ASSETBROWSER_HPP
#define VADONEDITOR_VIEW_PROJECT_ASSETBROWSER_HPP
#include <VadonEditor/Core/Project/Asset/Asset.hpp>
#include <VadonEditor/View/Scene/Resource/Resource.hpp>
#include <Vadon/Utilities/TypeInfo/TypeInfo.hpp>
#include <array>
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::Model
{
	class Resource;
}
namespace VadonEditor::View
{
	class AssetBrowser
	{
	private:
		struct TreeState
		{
			const Core::AssetNode* clicked_node;
			bool double_clicked;
			std::string node_label;

			void reset()
			{
				clicked_node = nullptr;
				double_clicked = false;
				node_label.clear();
			}
		};

		enum class CreateMenuOption
		{
			RESOURCE,
			SCENE,
			OPTION_COUNT
		};

		AssetBrowser(Core::Editor& editor);

		void draw(UI::Developer::GUISystem& dev_gui);

		void draw_asset_node(const Core::AssetNode* node, VadonApp::UI::Developer::GUISystem& dev_gui);

		// TODO: controls for creating scenes!
		void create_resource();

		void save_asset();
		bool save_resource(std::string_view path);

		void import_file();

		void open_asset(const Core::AssetNode* node);

		Core::Editor& m_editor;

		UI::Developer::Window m_window;
		UI::Developer::Button m_import_button;
		UI::Developer::ChildWindow m_asset_tree;
		TreeState m_asset_tree_state;

		const Core::AssetNode* m_selected_asset = nullptr;

		CreateResourceDialog m_create_resource_dialog;
		UI::Developer::FileBrowserDialog m_save_file_dialog;
		UI::Developer::FileBrowserDialog m_import_file_dialog;

		Core::AssetType m_pending_asset_type;
		std::array<UI::Developer::MenuItem, Vadon::Utilities::to_integral(CreateMenuOption::OPTION_COUNT)> m_create_asset_context_menus;

		friend class MainWindow;
	};
}
#endif