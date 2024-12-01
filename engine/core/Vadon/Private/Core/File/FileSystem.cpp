#include <Vadon/Private/PCH/Core.hpp>
#include <Vadon/Private/Core/File/FileSystem.hpp>

#include <format>
#include <fstream>

namespace Vadon::Private::Core
{
	RootDirectoryHandle FileSystem::add_root_directory(const RootDirectoryInfo& info)
	{
		// TODO: allow relative path from executable?
		const std::filesystem::path root_path(info.path);
		if (root_path.is_relative() == true)
		{
			log_error("File system: cannot add relative path as root directory!\n");
			return RootDirectoryHandle();
		}
		else if (std::filesystem::is_directory(root_path) == false)
		{
			log_error("File system: root directory path must point to a directory!\n");
			return RootDirectoryHandle();
		}

		const RootDirectoryHandle root_handle = m_root_pool.add();

		RootDirectoryData& root_data = m_root_pool.get(root_handle);
		root_data.info = info;
		root_data.path = root_path;

		return root_handle;
	}

	RootDirectoryInfo FileSystem::get_root_info(RootDirectoryHandle root_handle) const
	{
		return m_root_pool.get(root_handle).info;
	}

	void FileSystem::remove_root_directory(RootDirectoryHandle root_handle)
	{
		// TODO: any other cleanup?
		m_root_pool.remove(root_handle);
	}

	std::string FileSystem::get_absolute_path(const FileSystemPath& path) const
	{
		return internal_get_absolute_path(path).string();
	}

	std::string FileSystem::get_relative_path(std::string_view path, RootDirectoryHandle root_handle) const
	{
		const RootDirectoryData& root_dir = m_root_pool.get(root_handle.is_valid() ? root_handle : m_default_root);
		return std::filesystem::relative(path, root_dir.path).string();
	}

	std::vector<FileSystemPath> FileSystem::get_files_of_type(const FileSystemPath& root_path, std::string_view type, bool recursive) const
	{
		std::vector<FileSystemPath> result;

		const std::filesystem::path root_fs_path = internal_get_absolute_path(root_path);
		if(recursive == true)
		{
			for (const auto& directory_entry : std::filesystem::recursive_directory_iterator(root_fs_path))
			{
				// FIXME: the extension check is super cursed, should split and check equality with each entry!
				if ((directory_entry.is_regular_file() == true) && (type.find(directory_entry.path().extension().string()) != std::string::npos))
				{					
					result.push_back(FileSystemPath{ .root_directory = root_path.root_directory, .path = std::filesystem::relative(directory_entry.path(), root_fs_path).string() });
				}
			}
		}
		else
		{
			for (const auto& directory_entry : std::filesystem::directory_iterator(root_fs_path))
			{
				if ((directory_entry.is_regular_file() == true) && (directory_entry.path().extension() == type))
				{
					result.push_back(FileSystemPath{ .root_directory = root_path.root_directory, .path = std::filesystem::relative(directory_entry.path(), root_fs_path).string() });
				}
			}
		}

		return result;
	}

	size_t FileSystem::get_file_size(const FileSystemPath& path) const
	{
		return internal_get_file_size(get_absolute_path(path));
	}

	bool FileSystem::does_file_exist(const FileSystemPath& path) const
	{
		return std::filesystem::exists(internal_get_absolute_path(path));
	}

	bool FileSystem::save_file(const FileSystemPath& path, const RawFileDataBuffer& file_data)
	{
		const std::filesystem::path absolute_path = internal_get_absolute_path(path);

		// Create directories as necessary
		const std::filesystem::path parent_path = absolute_path.parent_path();
		std::error_code fs_error;
		if (std::filesystem::create_directories(parent_path, fs_error) == false)
		{
			// Check error code
			if (fs_error)
			{
				// TODO: log the specific error?
				log_error(std::format("File system error: unable to create path to file \"{}\"!\n", absolute_path.string()));
				return false;
			}
		}

		std::ofstream file_stream(absolute_path, std::ios::binary);
		if (file_stream.good() == false)
		{
			log_error(std::format("File system error: unable to write file \"{}\"!\n", absolute_path.string()));
			return false;
		}

		file_stream.write(reinterpret_cast<const char*>(file_data.data()), file_data.size());
		file_stream.close();

		return true;
	}

	bool FileSystem::load_file(const FileSystemPath& path, RawFileDataBuffer& file_data)
	{
		const std::filesystem::path absolute_path = internal_get_absolute_path(path);
		if (std::filesystem::exists(absolute_path) != true)
		{
			log_error(std::format("File system error: file \"{}\" does not exist!\n", absolute_path.string()));
			return false;
		}

		const size_t file_size = internal_get_file_size(absolute_path);
		std::ifstream file_stream(absolute_path, std::ios::binary);
		if (file_stream.good() == false)
		{
			log_error(std::format("File system error: unable to read file \"{}\"!\n", absolute_path.string()));
			return false;
		}

		const size_t buffer_prev_size = file_data.size();
		file_data.resize(buffer_prev_size + file_size);

		file_stream.read(reinterpret_cast<char*>(file_data.data() + buffer_prev_size), file_size);
		file_stream.close();

		return true;
	}

	FileSystem::FileSystem(Vadon::Core::EngineCoreInterface& core)
		: Vadon::Core::FileSystem(core)
	{

	}

	bool FileSystem::initialize()
	{
		const RootDirectoryInfo default_root_info {
			.path = std::filesystem::current_path().string()
		};

		m_default_root = add_root_directory(default_root_info);
		if (m_default_root.is_valid() == false)
		{
			log_error("File system error: failed to set default root directory!\n");
			return false;
		}

		return true;
	}

	std::filesystem::path FileSystem::internal_get_absolute_path(const FileSystemPath& path) const
	{
		std::filesystem::path fs_path(path.path);
		if (fs_path.is_absolute() == true)
		{
			// Absolute paths are just used as-is
			return fs_path;
		}

		// Append to root path
		const RootDirectoryHandle root_dir_handle = path.root_directory.is_valid() ? path.root_directory : m_default_root;
		const RootDirectoryData& root_dir = m_root_pool.get(root_dir_handle);

		return path.path.empty() == false ? root_dir.path / fs_path : root_dir.path;
	}

	size_t FileSystem::internal_get_file_size(const std::filesystem::path& absolute_path) const
	{
		try
		{
			const auto file_size = std::filesystem::file_size(absolute_path);
			return static_cast<size_t>(file_size);
		}
		catch (std::filesystem::filesystem_error& e)
		{
			log_error(std::format("File system error: \"{}\"\n", e.what()));
		}
		return 0;
	}
}