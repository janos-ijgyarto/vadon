#include <Vadon/Private/PCH/Core.hpp>
#include <Vadon/Private/Core/File/FileSystem.hpp>

#include <format>
#include <fstream>

namespace Vadon::Private::Core
{
	std::string FileSystem::get_absolute_path(std::string_view relative_path) const
	{
		return internal_get_absolute_path(relative_path).string();
	}

	std::vector<std::string> FileSystem::get_files_of_type(std::string_view path, std::string_view type, bool recursive) const
	{
		std::vector<std::string> result;

		const std::filesystem::path root_path = path.empty() ? m_root_path : (m_root_path / path);
		if(recursive == true)
		{
			for (const auto& directory_entry : std::filesystem::recursive_directory_iterator(root_path))
			{
				if ((directory_entry.is_regular_file() == true) && (directory_entry.path().extension() == type))
				{
					result.push_back(std::filesystem::relative(directory_entry.path(), root_path).string());
				}
			}
		}
		else
		{
			for (const auto& directory_entry : std::filesystem::directory_iterator(root_path))
			{
				if ((directory_entry.is_regular_file() == true) && (directory_entry.path().extension() == type))
				{
					result.push_back(std::filesystem::relative(directory_entry.path(), root_path).string());
				}
			}
		}

		return result;
	}

	size_t FileSystem::get_file_size(std::string_view relative_path) const
	{
		return internal_get_file_size(get_absolute_path(relative_path));
	}

	bool FileSystem::does_file_exist(std::string_view relative_path) const
	{
		const std::filesystem::path absolute_path = internal_get_absolute_path(relative_path);
		return std::filesystem::exists(absolute_path);
	}

	bool FileSystem::save_file(std::string_view relative_path, const RawFileDataBuffer& file_data)
	{
		const std::filesystem::path absolute_path = internal_get_absolute_path(relative_path);

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

	bool FileSystem::load_file(std::string_view relative_path, RawFileDataBuffer& file_data)
	{
		const std::filesystem::path absolute_path = internal_get_absolute_path(relative_path);
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
		// TODO: using executable location for now
		// Need to replace this with using project files!
		m_root_path = std::filesystem::current_path();
		return true;
	}

	std::filesystem::path FileSystem::internal_get_absolute_path(std::string_view relative_path) const
	{
		return m_root_path / std::filesystem::path(relative_path);
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