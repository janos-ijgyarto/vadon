#ifndef VADONDEMO_UI_UI_HPP
#define VADONDEMO_UI_UI_HPP
#include <VadonDemo/VadonDemoCommon.hpp>
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/Render/Canvas/Material.hpp>
#include <Vadon/Render/Text/Font.hpp>
#include <functional>
namespace Vadon::ECS
{
	class World;
}
namespace VadonDemo::Core
{
	class Core;
}
namespace VadonDemo::UI
{
	// TODO: more advanced input handling!
	struct CursorState
	{
		Vadon::Utilities::Vector2 position = Vadon::Utilities::Vector2_Zero;
		bool clicked = false;
	};

	class UI
	{
	public:
		using SelectableCallback = std::function<void(std::string_view)>;

		static void register_types();

		VADONDEMO_API void register_selectable_callback(std::string_view key, SelectableCallback callback);

		VADONDEMO_API void update(Vadon::ECS::World& ecs_world, const CursorState& cursor);

		VADONDEMO_API void update_ui_element(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity);
		VADONDEMO_API void remove_ui_element(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity);
	private:
		VADONDEMO_API UI(VadonDemo::Core::Core& core);

		bool initialize();

		bool load_default_font();

		void update_selectables(Vadon::ECS::World& ecs_world, const CursorState& cursor);
		void signal_selectable_callbacks(std::string_view key);

		VadonDemo::Core::Core& m_core;

		Vadon::Render::FontHandle m_default_font;
		Vadon::Render::Canvas::MaterialHandle m_text_sdf_material;

		std::unordered_map<std::string, std::vector<SelectableCallback>> m_selectable_callbacks;

		friend Core::Core;
	};
}
#endif