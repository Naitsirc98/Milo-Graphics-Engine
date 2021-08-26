#include <milo/logging/Log.h>
#include "milo/assets/shaders/SPIRVCompiler.h"
#include "milo/io/Files.h"

namespace milo {

	SPIRVCompiler::SPIRVCompiler() {
		m_Compiler = shaderc_compiler_initialize();
	}

	SPIRVCompiler::~SPIRVCompiler() {
		shaderc_compiler_release(m_Compiler);
		m_Compiler = nullptr;
	}

	inline static shaderc_shader_kind toShaderKind(Shader::Type type) {
		switch(type) {
			case Shader::Type::Vertex: return shaderc_vertex_shader;
			case Shader::Type::Fragment: return shaderc_fragment_shader;
			case Shader::Type::Geometry: return shaderc_geometry_shader;
			case Shader::Type::TessControl: return shaderc_tess_control_shader;
			case Shader::Type::TessEvaluation: return shaderc_tess_evaluation_shader;
			case Shader::Type::Compute: return shaderc_compute_shader;
			case Shader::Type::Undefined: default: throw MILO_RUNTIME_EXCEPTION("Unknown shader type");
		}
	}

	SPIRV SPIRVCompiler::compile(const String& filename, Shader::Type type) {

		float start = Time::millis();

		String srcCode = Files::readAllText(filename);

		shaderc_compile_options_t options = shaderc_compile_options_initialize();
		shaderc_compile_options_set_source_language(options, shaderc_source_language_glsl);

		shaderc_compilation_result_t result = shaderc_compile_into_spv(m_Compiler, srcCode.c_str(), srcCode.size(),
																	   toShaderKind(type), filename.c_str(), "main", options);

		if(shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) {
			const char* errorMessage = shaderc_result_get_error_message(result);
			throw MILO_RUNTIME_EXCEPTION(str("Failed to compile shader ") + filename + ": " + errorMessage);
		}

		Log::debug("{} compiled in {} ms", filename, Time::millis() - start);

		return result;
	}

	SPIRV::SPIRV(shaderc_compilation_result_t result) : compilationResult(result) {
	}

	SPIRV::SPIRV(SPIRV&& other) : compilationResult(other.compilationResult) {
		other.compilationResult = nullptr;
	}

	SPIRV::~SPIRV() {
		if(compilationResult != nullptr) shaderc_result_release(compilationResult);
	}

	SPIRV& SPIRV::operator=(SPIRV&& other) {
		this->compilationResult = other.compilationResult;
		other.compilationResult = nullptr;
		return *this;
	}

	const byte_t* SPIRV::code() const {
		return (byte_t*)shaderc_result_get_bytes(compilationResult);
	}

	size_t SPIRV::length() const {
		return shaderc_result_get_length(compilationResult);
	}
}