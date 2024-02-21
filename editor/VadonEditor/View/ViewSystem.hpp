#ifndef VADONEDITOR_VIEW_VIEWSYSTEM_HPP
#define VADONEDITOR_VIEW_VIEWSYSTEM_HPP
#include <memory>
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::View
{
	class ViewSystem
	{
	public:
		~ViewSystem();
	private:
		ViewSystem(Core::Editor& editor);

		bool initialize();
		void update();

		struct Internal;
		std::unique_ptr<Internal> m_internal;

		friend Core::Editor;
	};
}
#endif