#ifndef VADON_PRIVATE_CORE_FILE_FILESYSTEM_HPP
#define VADON_PRIVATE_CORE_FILE_FILESYSTEM_HPP
#include <Vadon/Core/File/FileSystem.hpp>

#include <filesystem>

namespace Vadon::Private::Core
{
	class FileSystem : public Vadon::Core::FileSystem
	{
	public:
		std::string get_absolute_path(std::string_view relative_path) const override;
		std::vector<std::string> get_files_of_type(std::string_view path, std::string_view type, bool recursive) const override;

		bool load_file(std::string_view relative_path, RawFileDataBuffer& file_data) override;
	protected:
		FileSystem(Vadon::Core::EngineCoreInterface& core);

		bool initialize();

		std::filesystem::path internal_get_absolute_path(std::string_view relative_path) const;

		std::filesystem::path m_root_path;

		friend class EngineCore;
	};
}
#endif