#include "milo/assets/shaders/ShaderManager.h"
#include "milo/graphics/Graphics.h"
#include "milo/graphics/vulkan/shaders/VulkanShader.h"
#include "milo/logging/Log.h"

namespace milo {

	ShaderManager::ShaderManager() {
		m_Shaders.reserve(64);
		loadCachedShaders();
	}

	ShaderManager::~ShaderManager() {
		for(auto& [filename, shader] : m_Shaders) {
			DELETE_PTR(shader);
		}
	}

	Shader* ShaderManager::load(const String& filename) {
		if(exists(filename)) return m_Shaders[filename];
		Shader* shader;
		m_Mutex.lock();
		{
			shader = createShader(filename);
			m_Shaders[filename] = shader;
			createShaderCache(filename, shader);
		}
		m_Mutex.unlock();
		return shader;
	}

	bool ShaderManager::exists(const String& filename) const {
		return m_Shaders.find(filename) != m_Shaders.end();
	}

	Shader* ShaderManager::find(const String& filename) const {
		if(exists(filename)) return m_Shaders.at(filename);
		return nullptr;
	}

	void ShaderManager::destroy(const String& filename) {
		if(!exists(filename)) return;
		m_Mutex.lock();
		{
			Shader* shader = m_Shaders[filename];
			DELETE_PTR(shader);
			m_Shaders.erase(filename);
		}
		m_Mutex.unlock();
	}

	Shader* ShaderManager::createShader(const String& filename) {

		Shader::Type type = getShaderTypeByFilename(filename);

		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {

			SPIRV spirv = m_SPIRVCompiler.compile(filename, type);

			size_t length = spirv.length();
			byte_t* bytecode = new byte_t[length];
			memcpy(bytecode, spirv.code(), length);

			return new VulkanShader(filename, type, bytecode, length);
		}

		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}

	Shader* ShaderManager::loadCache(const String& originalFilename, const String& filename) {

		Shader::Type type = getShaderTypeByFilename(originalFilename);

		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {

			const auto& bytes = Files::readAllBytes(filename);

			size_t length = bytes.size();
			byte_t* bytecode = new byte_t[length];
			memcpy(bytecode, bytes.data(), length);

			return new VulkanShader(filename, type, bytecode, length);
		}

		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}

	void ShaderManager::loadCachedShaders() {
		static const String CACHE_DIR = Files::resource("cache/shaders");
		if(!Files::exists(CACHE_DIR)) return;

		Log::info("Loading shader cache...");

		ArrayList<String> cacheFiles = Files::listFiles(CACHE_DIR);

		for(const String& cacheFile : cacheFiles) {
			const String filename = getFilenameFromCache(cacheFile);
			auto start = Time::millis();
			m_Shaders[filename] = loadCache(filename, cacheFile);
			Log::debug("Shader cache of {} loaded ({} ms)", Files::getName(filename), Time::millis() - start);
		}
	}

	void ShaderManager::createShaderCache(const String& filename, Shader* shader) {
		static const size_t RESOURCE_PREFIX_LENGTH = strlen("resources/");

		String cacheFile = Files::resource("cache/" + filename.substr(RESOURCE_PREFIX_LENGTH) + ".cache");

		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			VulkanShader* vulkanShader = dynamic_cast<VulkanShader*>(shader);
			Files::writeAllBytes(cacheFile, vulkanShader->m_Bytecode, vulkanShader->m_BytecodeLength);
		} else {
			throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
		}
	}

	Shader::Type ShaderManager::getShaderTypeByFilename(const String& filename) {

		const String extension = Files::extension(filename);

		if(extension == ".vert") return Shader::Type::Vertex;
		if(extension == ".frag") return Shader::Type::Fragment;
		if(extension == ".geo") return Shader::Type::Geometry;
		if(extension == ".comp") return Shader::Type::Compute;
		if(extension == ".tesc") return Shader::Type::TessControl;
		if(extension == ".tese") return Shader::Type::TessEvaluation;

		throw MILO_RUNTIME_EXCEPTION(fmt::format("Unknown shader extension '{}' of {}", extension, filename));
	}

	String ShaderManager::getFilenameFromCache(const String& cacheFilename) {
		static const size_t CACHE_EXTENSION_LENGTH = strlen(".cache");
		static const Regex REGEX{R"(cache/)"};
		String filename = cacheFilename.substr(0, cacheFilename.length() - CACHE_EXTENSION_LENGTH);
		return replace(filename, REGEX, "");
	}
}