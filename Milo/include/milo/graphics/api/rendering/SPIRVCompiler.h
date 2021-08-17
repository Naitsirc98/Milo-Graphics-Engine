#pragma once

#include <shaderc/shaderc.h>
#include "milo/common/Common.h"

namespace milo {

	struct SPIRV {
		shaderc_compilation_result_t compilationResult;

		SPIRV(shaderc_compilation_result_t compilationResult);
		SPIRV(const SPIRV& other) = delete;
		SPIRV(SPIRV&& other);
		~SPIRV();
		SPIRV& operator=(const SPIRV& other) = delete;
		SPIRV& operator=(SPIRV&& other);

		const uint32_t* code() const;
		size_t length() const;
	};

	class SPIRVCompiler {
	private:
		shaderc_compiler_t m_Compiler = nullptr;
	public:
		SPIRVCompiler();
		~SPIRVCompiler();
		SPIRV compile(const String& filename, shaderc_shader_kind shaderKind);
	};
}