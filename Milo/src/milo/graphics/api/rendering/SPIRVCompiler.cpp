#include "milo/graphics/api/rendering/SPIRVCompiler.h"
#include "milo/io/Files.h"

namespace milo {

	SPIRVCompiler::SPIRVCompiler() = default;

	SPIRVCompiler::~SPIRVCompiler() = default;

	ArrayList<uint32_t> SPIRVCompiler::compile(const String& filename, shaderc_shader_kind shaderKind) {

		String sourceCode = Files::readAllText(filename);

		shaderc::CompilationResult<uint32_t> result = m_Compiler.CompileGlslToSpv(sourceCode, shaderKind, filename.c_str());

		if(result.GetCompilationStatus() != shaderc_compilation_status_success) {
			throw MILO_RUNTIME_EXCEPTION(str("Failed to compile shader ") + filename + ": " + result.GetErrorMessage());
		}

		ArrayList<uint32_t> binary(result.begin(), result.end());
		return binary;
	}
}