#ifndef VADONAPP_UI_DEVELOPER_GUIELEMENTS_HPP
#define VADONAPP_UI_DEVELOPER_GUIELEMENTS_HPP
#include <Vadon/Math/Color.hpp>
#include <Vadon/Math/Vector.hpp>
#include <Vadon/Utilities/Enum/EnumClassBitFlag.hpp>
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

	enum class InputFlags
	{
		NONE = 0,
		ENTER_RETURNS_TRUE = 1 << 0,
		READ_ONLY = 1 << 1,
		DEFAULT = NONE
	};
}

namespace Vadon::Utilities
{
	template<>
	struct EnableEnumBitwiseOperators<VadonApp::UI::Developer::WindowFlags> : public std::true_type
	{

	};

	template<>
	struct EnableEnumBitwiseOperators<VadonApp::UI::Developer::InputFlags> : public std::true_type
	{

	};
}

namespace VadonApp::UI::Developer
{
	// TODO: at the moment, we are actually storing strings/arrays/etc. in each of these objects
	// instead of using lightweight structs that just have views (so creating it for each GUI call is more expensive)
	// Change it so we have a "view" version of each object where needed, all GUI commands only use these
	// The "data" object then uses a "get_view" function where it maps to and from the members of the data object
	// Client code can then either use these data objects, or create views directly
	struct Window
	{
		std::string title;
		WindowFlags flags = WindowFlags::DEFAULT;
		bool open = false;
	};

	struct ChildWindow
	{
		int32_t int_id;
		std::string string_id;
		Vadon::Math::Vector2 size = { 0, 0 };
		bool border = false;
		WindowFlags flags = WindowFlags::DEFAULT;
	};

	struct InputBase
	{
		std::string label;
		InputFlags flags = InputFlags::DEFAULT;
	};

	template<typename Type>
	struct Input : public InputBase
	{
		Type input;
	};

	using InputInt = Input<int>;
	using InputInt2 = Input<Vadon::Math::Vector2i>;
	using InputFloat = Input<float>;
	using InputFloat2 = Input<Vadon::Math::Vector2>;
	using InputFloat3 = Input<Vadon::Math::Vector3>;

	struct ColorEdit
	{
		std::string label;
		Vadon::Math::ColorRGBA value;
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
	using SliderInt2 = Slider<Vadon::Math::Vector2i, int>;
	using SliderFloat = Slider<float, float>;
	using SliderFloat2 = Slider<Vadon::Math::Vector2, float>;

	struct InputText
	{
		std::string label;
		std::string input;
		bool multiline = false;
		InputFlags flags = InputFlags::DEFAULT;
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

	// NOTE: this is for simplified ListBox behavior
	// If client code needs more complex handling for list items, use begin/end_list_box
	struct ListBox
	{
		std::string label;
		std::vector<std::string> items;
		int32_t selected_item = -1;
		Vadon::Math::Vector2 size = Vadon::Math::Vector2_Zero;

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

	struct Table
	{
		// TODO: flags!
		std::string label;
		int32_t column_count = 0;
		Vadon::Math::Vector2 outer_size = Vadon::Math::Vector2_Zero;
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