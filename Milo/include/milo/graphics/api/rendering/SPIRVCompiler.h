#pragma once

#include <shaderc/shaderc.hpp>
#include "milo/common/Common.h"

namespace milo {

	class SPIRVCompiler {
	private:
		shaderc::Compiler m_Compiler;
	public:
		SPIRVCompiler();
		~SPIRVCompiler();
		ArrayList<uint32_t> compile(const String& filename, shaderc_shader_kind shaderKind);
	};
}