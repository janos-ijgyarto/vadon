#ifndef VADON_PRIVATE_CORE_FILE_FILESYSTEM_HPP
#define VADON_PRIVATE_CORE_FILE_FILESYSTEM_HPP
#include <Vadon/Core/File/FileSystem.hpp>
#include <Vadon/Private/Core/File/Path.hpp>
#include <Vadon/Private/Core/File/RootDirectory.hpp>

#include <Vadon/Utilities/Container/ObjectPool/Pool.hpp>

#include <filesystem>

namespace Vadon::Private::Core
{
	class FileSystem : public Vadon::Core::FileSystem
	{
	public:
		RootDirectoryHandle add_root_directory(const RootDirectoryInfo& info) override;
		RootDirectoryInfo get_root_info(RootDirectoryHandle root_handle) const override;
		void remove_root_directory(RootDirectoryHandle root_handle) override;

		RootDirectoryHandle get_default_root() const override { return m_default_root; }

		std::string get_absolute_path(const FileSystemPath& path) const override;		
		std::string get_relative_path(std::string_view path, RootDirectoryHandle root_handle = RootDirectoryHandle()) const override;
		std::vector<FileSystemPath> get_files_of_type(const FileSystemPath& root_path, std::string_view type, bool recursive) const override;

		size_t get_file_size(const FileSystemPath& path) const override;
		bool does_file_exist(const FileSystemPath& path) const override;

		bool save_file(const FileSystemPath& path, const RawFileDataBuffer& file_data) override;
		bool load_file(const FileSystemPath& path, RawFileDataBuffer& file_data) override;
	protected:
		struct RootDirectoryData
		{
			RootDirectoryInfo info;
			std::filesystem::path path;
			// TODO: anything else?
		};

		FileSystem(Vadon::Core::EngineCoreInterface& core);

		bool initialize();

		std::filesystem::path internal_get_absolute_path(const FileSystemPath& path) const;
		size_t internal_get_file_size(const std::filesystem::path& absolute_path) const;

		Vadon::Utilities::ObjectPool<Vadon::Core::RootDirectory, RootDirectoryData> m_root_pool;
		RootDirectoryHandle m_default_root;

		friend class EngineCore;
	};
}
#endif