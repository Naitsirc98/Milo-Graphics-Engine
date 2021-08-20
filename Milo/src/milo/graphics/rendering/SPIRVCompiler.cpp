#include <milo/logging/Log.h>
#include "milo/graphics/rendering/SPIRVCompiler.h"
#include "milo/io/Files.h"

namespace milo {

	SPIRVCompiler::SPIRVCompiler() {
		m_Compiler = shaderc_compiler_initialize();
	}

	SPIRVCompiler::~SPIRVCompiler() {
		shaderc_compiler_release(m_Compiler);
		m_Compiler = nullptr;
	}

	SPIRV SPIRVCompiler::compile(const String& filename, shaderc_shader_kind shaderKind) {

		float start = Time::millis();

		String srcCode = Files::readAllText(filename);

		shaderc_compile_options_t options = shaderc_compile_options_initialize();
		shaderc_compile_options_set_source_language(options, shaderc_source_language_glsl);

		shaderc_compilation_result_t result = shaderc_compile_into_spv(m_Compiler, srcCode.c_str(), srcCode.size(), shaderKind, filename.c_str(), "main", options);

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

	const uint32_t* SPIRV::code() const {
		return (uint32_t*)shaderc_result_get_bytes(compilationResult);
	}

	size_t SPIRV::length() const {
		return shaderc_result_get_length(compilationResult);
	}
}