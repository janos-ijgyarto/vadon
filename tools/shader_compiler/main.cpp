#include <iostream>
#include <format>
#include <fstream>
#include <filesystem>
#include <vector>
#include <unordered_set>

#ifdef VADON_D3D_COMPILER
#include <d3dcompiler.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;
#endif

namespace
{
	enum class ExportType
	{
		SHADER_FILE,
		CPP_CODE,
		DEFAULT = SHADER_FILE
	};

	enum class ShaderTarget
	{
		VERTEX,
		PIXEL,
		INVALID
	};

	ShaderTarget parse_shader_target(const char* target_str)
	{
		std::string_view target_str_view(target_str);
		if (target_str_view == "VERTEX")
		{
			return ShaderTarget::VERTEX;
		}
		else if (target_str_view == "PIXEL")
		{
			return ShaderTarget::PIXEL;
		}

		return ShaderTarget::INVALID;
	}

	struct Configuration
	{
		std::filesystem::path root_path;
		std::filesystem::path input_path;
		std::filesystem::path output_path;
		
		ShaderTarget target;
		std::string entrypoint;

		ExportType export_type = ExportType::DEFAULT;
		std::string namespace_title;
		// TODO: anything else?

		bool validate_paths() const
		{
			if ((root_path.is_absolute() == false)
				|| (input_path.is_absolute() == false)
				|| (output_path.is_absolute() == false))
			{
				return false;
			}

			// TODO: also check that root and input are set correctly w.r.t each other, they don't try to overwrite each other, etc.

			if (std::filesystem::is_directory(root_path) == false)
			{
				return false;
			}

			if (std::filesystem::is_regular_file(input_path) == false)
			{
				return false;
			}

			// TODO: more extensive validation of output path?
			if (output_path.empty() == true)
			{
				return false;
			}

			return true;
		}
	};

	enum class ErrorCode : int
	{
		SUCCESS,
		INVALID_ARGS,
		INVALID_PATH,
		FILE_ERROR,
		FAILED_COMPILATION,
		FAILED_OUTPUT
	};

	struct CompiledShader
	{
		std::string api;
		std::string extension;
		std::vector<char> data;
	};

	struct ShaderBundle
	{
		std::vector<CompiledShader> shaders;
		std::unordered_set<std::filesystem::path> dependencies;
	};

	struct ShaderDefine
	{
		std::string name;
		std::string value;
	};

	ErrorCode read_file_to_buffer(const std::filesystem::path& path, size_t file_size, char* buffer)
	{
		std::ifstream input_file(path, std::ios::binary);
		if (input_file.is_open() == false)
		{
			return ErrorCode::FILE_ERROR;
		}

		input_file.read(buffer, file_size);
		input_file.close();

		return ErrorCode::SUCCESS;
	}

#ifdef VADON_D3D_COMPILER
	class D3DIncludeImpl : public ID3DInclude
	{
	public:
		D3DIncludeImpl(const Configuration& config) : m_config(config) {}

		HRESULT Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID /*pParentData*/, LPCVOID* ppData, UINT* pBytes) override
		{
			std::filesystem::path include_path;
			if (IncludeType == D3D_INCLUDE_LOCAL)
			{
				include_path = pFileName;
			}
			else if (IncludeType == D3D_INCLUDE_SYSTEM)
			{
				include_path = (m_config.root_path / pFileName).generic_string();
			}
			else
			{
				return E_FAIL;
			}

			if (std::filesystem::is_regular_file(include_path) == false)
			{
				std::cerr << "No valid file at path: " << include_path << std::endl;
				return E_FAIL;
			}

			m_includes.insert(include_path);

			const size_t file_size = std::filesystem::file_size(include_path);
			char* data = reinterpret_cast<char*>(std::malloc(file_size));

			ErrorCode err = read_file_to_buffer(include_path, file_size, data);
			if (err != ErrorCode::SUCCESS)
			{
				std::free(data);
				std::cerr << "Failed to read file data: " << include_path << std::endl;
				return E_FAIL;
			}

			*pBytes = static_cast<UINT>(file_size);
			*ppData = data;
			return S_OK;
		}

		HRESULT Close(LPCVOID pData) override
		{
			std::free(const_cast<void*>(pData));
			return S_OK;
		}

		const std::unordered_set<std::filesystem::path>& get_includes() const { return m_includes; }

	public:
		const Configuration& m_config;
		std::unordered_set<std::filesystem::path> m_includes;
	};

	using D3DBlob = ComPtr<ID3DBlob>;

	std::string get_d3d_error_blob_message(const D3DBlob& error_blob)
	{
		const char* error_message_ptr = static_cast<const char*>(error_blob->GetBufferPointer());
		return std::string(error_message_ptr, error_message_ptr + error_blob->GetBufferSize());
	}

	using D3DShaderMacroList = std::vector<D3D_SHADER_MACRO>;
	D3DShaderMacroList get_shader_macros(const std::vector<ShaderDefine>& shader_defines)
	{
		D3DShaderMacroList shader_macros;
		shader_macros.reserve(shader_defines.size() + 1);

		for (const ShaderDefine& current_define : shader_defines)
		{
			D3D_SHADER_MACRO& current_macro = shader_macros.emplace_back();
			ZeroMemory(&current_macro, sizeof(D3D_SHADER_MACRO));

			current_macro.Name = current_define.name.c_str();
			current_macro.Definition = current_define.value.c_str();
		}

		D3D_SHADER_MACRO& end_macro = shader_macros.emplace_back();
		ZeroMemory(&end_macro, sizeof(D3D_SHADER_MACRO));

		return shader_macros;
	}

	constexpr const char* get_d3d_shader_target_string(ShaderTarget target)
	{
		switch (target)
		{
		case ShaderTarget::VERTEX:
			return "vs_4_0";
		case ShaderTarget::PIXEL:
			return "ps_4_0";
		default:
			return "INVALID";
		}
	}

	bool compile_shader_d3d(const Configuration& config, const std::vector<char>& input_data, ShaderBundle& shader_bundle)
	{
		using D3DShaderMacroList = std::vector<D3D_SHADER_MACRO>;

		D3DBlob shader_blob;
		D3DBlob error_blob;

		UINT compile_flags = 0;
#ifndef NDEBUG
		compile_flags |= D3DCOMPILE_DEBUG;
#endif

		const char* source_data = input_data.data();
		const std::string file_name = config.input_path.filename().string();
		const char* source_name = file_name.c_str();

		D3DShaderMacroList shader_macros;
		// TODO: get shader macros!
		const D3D_SHADER_MACRO* shader_macro_ptr = (shader_macros.empty() == false) ? shader_macros.data() : nullptr;

		D3DIncludeImpl d3d_include(config);

		const char* shader_target = get_d3d_shader_target_string(config.target);

		HRESULT result = D3DCompile(source_data, input_data.size(), source_name, shader_macro_ptr, &d3d_include, config.entrypoint.c_str(), shader_target, compile_flags, 0, shader_blob.ReleaseAndGetAddressOf(), error_blob.ReleaseAndGetAddressOf());
		if (FAILED(result))
		{
			if (error_blob != nullptr)
			{
				std::cerr << get_d3d_error_blob_message(error_blob);
			}
			return false;
		}

		// Add dependencies
		auto& shader_includes = d3d_include.get_includes();
		for (auto it = shader_includes.begin(); it != shader_includes.end(); ++it)
		{
			shader_bundle.dependencies.insert(*it);
		}

		CompiledShader& compiled_shader = shader_bundle.shaders.emplace_back();
		compiled_shader.api = "D3D11";
		compiled_shader.extension = ".cso";
		compiled_shader.data.resize(shader_blob->GetBufferSize());

		memcpy(compiled_shader.data.data(), shader_blob->GetBufferPointer(), shader_blob->GetBufferSize());
		return true;
	}
#endif

	bool compile_all(const Configuration& config, const std::vector<char>& input_data, ShaderBundle& shader_bundle)
	{
#ifdef VADON_D3D_COMPILER
		if (compile_shader_d3d(config, input_data, shader_bundle) == false)
		{
			return false;
		}
#endif
		return true;
	}

	ErrorCode export_shader_files(const Configuration& /*config*/, const ShaderBundle& /*shader_bundle*/)
	{
		// FIXME: revise implementation
		// Args decide whether this is "bundled" (i.e all in one file with a "header" for offsets)
		// or put into separate files using suffixes
#if 0
		const std::filesystem::path shader_name = config.input_path.stem();
		for (const CompiledShader& current_shader : shader_bundle.shaders)
		{
			std::filesystem::path current_output_path = std::filesystem::path(config.output_path).concat(current_shader.api);
			current_output_path.replace_extension(current_shader.extension);

			std::ofstream output_file(current_output_path);
			if (output_file.is_open() == false)
			{
				return ErrorCode::FILE_ERROR;
			}

			output_file.write(current_shader.data.data(), current_shader.data.size());
			output_file.close();
		}
#endif
		return ErrorCode::FAILED_OUTPUT;
	}

	ErrorCode export_cpp_code(const Configuration& config, const ShaderBundle& shader_bundle)
	{
		constexpr const char* file_template_string = 
R"(namespace {} {{
	static constexpr size_t c_shader_count = {}; 
	static constexpr const char* c_shader_apis[] = {};
	static constexpr size_t c_shader_data_sizes[] = {};
	static constexpr char c_shader_data[] = {};
}})";

		std::string shader_apis;
		std::string shader_data_sizes;
		std::string shader_data;

		constexpr auto array_entry_prefix = +[](std::string& string)
			{
				if (string.empty() == true)
				{
					string += "{ ";
				}
				else
				{
					string += ", ";
				}
			};

		constexpr size_t c_temp_buffer_size = 1024;
		char temp_buffer[c_temp_buffer_size]{};

		for (const CompiledShader& current_shader : shader_bundle.shaders)
		{
			array_entry_prefix(shader_apis);
			shader_apis += std::format("\"{}\"", current_shader.api);

			array_entry_prefix(shader_data_sizes);
			{
				std::to_chars_result result = std::to_chars(temp_buffer, temp_buffer + c_temp_buffer_size, current_shader.data.size());

				if (result.ec != std::errc())
				{
					std::cerr << std::make_error_code(result.ec).message() << '\n';
					return ErrorCode::FAILED_OUTPUT;
				}
				else
				{
					shader_data_sizes += std::string_view(temp_buffer, result.ptr - temp_buffer);
				}
			}

			constexpr size_t column_wrap_limit = 15;
			size_t column_counter = 0;
			for (char current_byte : current_shader.data)
			{
				if (column_counter >= column_wrap_limit)
				{
					shader_data += ",\n\t";
					column_counter = 0;
				}
				else
				{
					array_entry_prefix(shader_data);
				}

				std::to_chars_result result = std::to_chars(temp_buffer, temp_buffer + c_temp_buffer_size, current_byte);

				if (result.ec != std::errc())
				{
					std::cerr << std::make_error_code(result.ec).message() << '\n';
					return ErrorCode::FAILED_OUTPUT;
				}
				else
				{
					shader_data += std::string_view(temp_buffer, result.ptr - temp_buffer);
				}

				++column_counter;
			}
		}

		shader_apis += " }";
		shader_data_sizes += " }";
		shader_data += " }";

		std::filesystem::path output_path = (config.output_path).generic_string();
		output_path.replace_extension("hpp");

		const std::string namespace_title = config.namespace_title.empty() == false ? config.namespace_title : output_path.stem().string();

		std::string output_string = std::format(file_template_string, namespace_title, shader_bundle.shaders.size(), shader_apis, shader_data_sizes, shader_data);

		std::ofstream output_file(output_path);
		if (output_file.is_open() == false)
		{
			return ErrorCode::FILE_ERROR;
		}

		output_file.write(output_string.c_str(), output_string.length());
		output_file.close();

		return ErrorCode::SUCCESS;
	}

	ErrorCode export_dependencies(const Configuration& config, const ShaderBundle& shader_bundle)
	{
		std::string dependency_list_string;

		for (const auto& current_path : shader_bundle.dependencies)
		{
			if (dependency_list_string.empty() == false)
			{
				dependency_list_string += " ";
			}
			dependency_list_string += current_path.string();
		}

		dependency_list_string = std::format("{} : {}", config.input_path.string(), dependency_list_string);

		const std::filesystem::path output_path = std::filesystem::path(config.output_path).replace_extension(".d").generic_string();

		std::ofstream output_file(output_path);
		if (output_file.is_open() == false)
		{
			return ErrorCode::FILE_ERROR;
		}

		output_file.write(dependency_list_string.c_str(), dependency_list_string.length());
		output_file.close();

		return ErrorCode::SUCCESS;
	}

	ErrorCode main_internal(int argc, char* argv[])
	{
		// At minimum, we need the root dir, input, and output paths, target, and entrypoint
		if (argc < 6)
		{
			// TODO: error log!
			return ErrorCode::INVALID_ARGS;
		}

		// FIXME: the argument parsing should be a lot cleaner, this is just a quick-and-dirty solution
		Configuration config;
		config.root_path = std::filesystem::path(argv[1]).generic_string();
		config.input_path = std::filesystem::path(argv[2]).generic_string();
		config.output_path = std::filesystem::path(argv[3]).generic_string();

		config.target = parse_shader_target(argv[4]);
		if (config.target == ShaderTarget::INVALID)
		{
			return ErrorCode::INVALID_ARGS;
		}

		config.entrypoint = argv[5];

		if (argc >= 7)
		{
			{
				std::string_view output_setting_str = argv[6];
				if (output_setting_str == "shader")
				{
					config.export_type = ExportType::SHADER_FILE;
				}
				else if (output_setting_str == "cpp")
				{
					config.export_type = ExportType::CPP_CODE;
				}
			}
			if((config.export_type == ExportType::CPP_CODE) && (argc >= 8))
			{
				config.namespace_title = argv[7];
			}
		}

		if (config.validate_paths() == false)
		{
			return ErrorCode::INVALID_PATH;
		}

		std::vector<char> input_data;
		{
			const size_t input_size = std::filesystem::file_size(config.input_path);
			input_data.resize(input_size);

			ErrorCode err = read_file_to_buffer(config.input_path, input_size, input_data.data());
			if (err != ErrorCode::SUCCESS)
			{
				return err;
			}
		}

		ShaderBundle shader_bundle;
		if (compile_all(config, input_data, shader_bundle) == false)
		{
			// TODO: error log!
			return ErrorCode::FAILED_COMPILATION;
		}

		ErrorCode result = ErrorCode::SUCCESS;
		switch (config.export_type)
		{
		case ExportType::SHADER_FILE:
			result = export_shader_files(config, shader_bundle);
			break;
		case ExportType::CPP_CODE:
			result = export_cpp_code(config, shader_bundle);
			break;
		}

		if (result == ErrorCode::SUCCESS)
		{
			result = export_dependencies(config, shader_bundle);
		}

		// TODO: log success!
		return result;
	}
}

int main(int argc, char* argv[])
{
	return static_cast<int>(main_internal(argc, argv));
}