#pragma once

#include "milo/assets/shaders/Shader.h"
#include "milo/graphics/vulkan/VulkanAPI.h"

namespace milo {

	class VulkanShader : public Shader {
		friend class ShaderManager;
	private:
		const byte_t* m_Bytecode{nullptr};
		const size_t m_BytecodeLength{0};
		const VkShaderStageFlags m_Stage{VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM};
	private:
		VulkanShader(const String& filename, Shader::Type type, const byte_t* bytecode, size_t length);
		~VulkanShader();
	public:
		VkShaderStageFlags stage() const;
		const byte_t* bytecode() const;
		size_t bytecodeLength() const;
	};

}