#pragma once

#include "SPIRVCompiler.h"

namespace milo {

	class ShaderManager {
		friend class AssetManager;
	private:
		HashMap<String, Shader*> m_Shaders;
		SPIRVCompiler m_SPIRVCompiler;
		Mutex m_Mutex;
	private:
		ShaderManager();
		~ShaderManager();
	public:
		Shader* load(const String& filename);
		bool exists(const String& filename) const;
		Shader* find(const String& filename) const;
		void destroy(const String& filename);
	private:
		Shader* createShader(const String& filename);
		Shader* loadCache(const String& originalFilename, const String& cacheFilename);
		void loadCachedShaders();
		void createShaderCache(const String& filename, Shader* shader);
		static Shader::Type getShaderTypeByFilename(const String& filename);
		static String getFilenameFromCache(const String& cacheFilename);
	};

}