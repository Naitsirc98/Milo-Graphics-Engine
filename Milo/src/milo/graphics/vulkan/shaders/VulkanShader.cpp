#include "milo/graphics/vulkan/shaders/VulkanShader.h"

namespace milo {

	VulkanShader::VulkanShader(const String& filename, Shader::Type type, const byte_t* bytecode, size_t length)
		: Shader(filename, type), m_Bytecode(bytecode), m_BytecodeLength(length), m_Stage(mvk::fromShaderType(type)) {
	}

	VulkanShader::~VulkanShader() {
		DELETE_PTR(m_Bytecode);
	}

	VkShaderStageFlags VulkanShader::stage() const {
		return m_Stage;
	}

	const byte_t* VulkanShader::bytecode() const {
		return m_Bytecode;
	}

	size_t VulkanShader::bytecodeLength() const {
		return m_BytecodeLength;
	}
}