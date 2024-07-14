#ifndef VADONEDITOR_CORE_SYSTEM_SYSTEM_HPP
#define VADONEDITOR_CORE_SYSTEM_SYSTEM_HPP
#include <VadonEditor/Core/Editor.hpp>
#include <Vadon/Core/Logger.hpp>
#include <Vadon/Utilities/System/Singleton/Singleton.hpp>
namespace VadonEditor::Core
{
	class Editor;

	class SystemBase : public Vadon::Core::Logger
	{
	public:
		void log(std::string_view message) override;
		void warning(std::string_view message) override;
		void error(std::string_view message) override;

		Editor& get_editor() { return m_editor; }
	protected:
		SystemBase(Editor& editor)
			: m_editor(editor)
		{}

		// FIXME: this is a hack, should have a proper way to give access to the registry!
		Vadon::Utilities::SingletonRegistry& get_registry() { return m_editor.m_singleton_registry; }

		Editor& m_editor;
	};

	template<typename Module, typename SysImpl>
	class System : public SystemBase, public Vadon::Utilities::Singleton<Module, SysImpl>
	{
	protected:
		System(Editor& editor)
			: SystemBase(editor)
			, Vadon::Utilities::Singleton<Module, SysImpl>(get_registry())
		{
		}
	};
}
#endif