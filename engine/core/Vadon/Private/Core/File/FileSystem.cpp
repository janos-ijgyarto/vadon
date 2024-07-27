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

	std::vector<std::string> Vadon::Private::Core::FileSystem::get_files_of_type(std::string_view path, std::string_view type, bool recursive) const
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

	bool FileSystem::load_file(std::string_view relative_path, RawFileDataBuffer& file_data)
	{
		const std::filesystem::path absolute_path = internal_get_absolute_path(relative_path);
		if (std::filesystem::exists(absolute_path) != true)
		{
			return false;
		}
		
		std::ifstream file_stream(absolute_path, std::ios::binary);
		if (file_stream.good() == false)
		{
			Vadon::Core::Logger::log_error(std::format("File system error: unable to open file \"{}\"!\n", absolute_path.string()));
			return false;
		}

		file_stream.seekg(0, std::ios::end);
		std::streampos file_size = file_stream.tellg();
		file_stream.seekg(0, std::ios::beg);

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
}