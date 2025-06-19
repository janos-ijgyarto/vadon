#ifndef VADONAPP_UI_DEVELOPER_GUIELEMENTS_HPP
#define VADONAPP_UI_DEVELOPER_GUIELEMENTS_HPP
#include <Vadon/Utilities/Enum/EnumClassBitFlag.hpp>
#include <Vadon/Utilities/Math/Color.hpp>
#include <Vadon/Utilities/Math/Vector.hpp>
#include <string>
#include <memory>
#include <span>

namespace VadonApp::UI::Developer
{
	enum class WindowFlags
	{
		NONE = 0,
		ENABLE_CLOSE = 1 << 0,
		HORIZONTAL_SCROLLBAR = 1 << 1,
		DEFAULT = NONE
	};
}

namespace Vadon::Utilities
{
	template<>
	struct EnableEnumBitwiseOperators<VadonApp::UI::Developer::WindowFlags> : public std::true_type
	{

	};
}

namespace VadonApp::UI::Developer
{
	struct Window
	{
		std::string title;
		WindowFlags flags = WindowFlags::DEFAULT;
		bool open = false;
	};

	struct ChildWindow
	{
		std::string id;
		Vadon::Utilities::Vector2 size = { 0, 0 };
		bool border = false;
		WindowFlags flags = WindowFlags::DEFAULT;
	};

	struct InputBase
	{
		std::string label;
	};

	template<typename Type>
	struct Input : public InputBase
	{
		Type input;
	};

	using InputInt = Input<int>;
	using InputInt2 = Input<Vadon::Utilities::Vector2i>;
	using InputFloat = Input<float>;
	using InputFloat2 = Input<Vadon::Utilities::Vector2>;
	using InputFloat3 = Input<Vadon::Utilities::Vector3>;

	struct ColorEdit
	{
		std::string label;
		Vadon::Utilities::ColorRGBA value;
	};

	struct SliderBase
	{
		std::string label;
		std::string format;
	};

	template<typename Value, typename MinMax>
	struct Slider : public SliderBase
	{
		Value value;
		MinMax min;
		MinMax max;
	};

	using SliderInt = Slider<int, int>;
	using SliderInt2 = Slider<Vadon::Utilities::Vector2i, int>;
	using SliderFloat = Slider<float, float>;
	using SliderFloat2 = Slider<Vadon::Utilities::Vector2, float>;

	struct InputText
	{
		std::string label;
		std::string input;
		bool multiline = false;
	};

	struct Button
	{
		std::string label;
	};

	struct Checkbox
	{
		std::string label;
		bool checked = false;
	};

	// FIXME: merge these two?
	struct ListBox
	{
		std::string label;
		std::vector<std::string> items;
		int32_t selected_item = -1;
		Vadon::Utilities::Vector2 size = { 0, 0 };

		bool has_valid_selection() const
		{
			if ((selected_item >= 0) && (selected_item < items.size()))
			{
				return true;
			}
			
			return false;
		}

		void deselect()
		{
			selected_item = -1;
		}
	};

	struct ComboBox
	{
		// TODO: flags!
		std::string label;
		std::vector<std::string> items;
		int32_t selected_item = -1;

		void deselect()
		{
			selected_item = -1;
		}
	};

	struct Table
	{
		// TODO: flags!
		std::string label;
		int32_t column_count = 0;
	};

	struct MenuItem
	{
		std::string label;
		// TODO: shortcut
		bool selected = false;
		bool enabled = true;
	};

	struct TextBuffer
	{
		std::string_view buffer;
		std::span<const size_t> line_offsets;
	};
}
#endif