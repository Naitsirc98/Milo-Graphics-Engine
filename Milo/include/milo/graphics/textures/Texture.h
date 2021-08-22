#pragma once

#include "milo/logging/Log.h"
#include "milo/assets/images/Image.h"

#define AUTO_MIP_LEVELS 0

namespace milo {

	class Texture2D {
	public:
		struct AllocInfo {
			uint32_t width = 0;
			uint32_t height = 0;
			PixelFormat format = PixelFormat::Undefined;
			uint32_t mipLevels = AUTO_MIP_LEVELS;
			const void* pixels = nullptr;
			const void* apiInfo = nullptr;
		};

		struct UpdateInfo {
			uint64_t size = 0;
			const void* pixels = nullptr;
			const void* apiInfo = nullptr;
		};
	public:
		virtual uint32_t width() const = 0;
		virtual uint32_t height() const = 0;
		virtual void allocate(const AllocInfo& allocInfo) = 0;
		virtual void update(const UpdateInfo& updateInfo) = 0;
		virtual void generateMipmaps() = 0;
	public:
		static Texture2D* create();
		static Texture2D* createColorAttachment();
		static Texture2D* createDepthAttachment();
	};

}