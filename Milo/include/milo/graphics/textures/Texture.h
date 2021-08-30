#pragma once

#include "milo/logging/Log.h"
#include "milo/assets/images/Image.h"

#define AUTO_MIP_LEVELS 0

namespace milo {

	enum TextureUsageFlagBits {
		TEXTURE_USAGE_UNDEFINED_BIT = 0x0,
		TEXTURE_USAGE_SAMPLED_BIT = 0x1,
		TEXTURE_USAGE_COLOR_ATTACHMENT_BIT = 0x2,
		TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT = 0x4,
		TEXTURE_USAGE_STORAGE_BIT = 0x8
	};
	using TextureUsageFlags = uint32_t;

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
		virtual ~Texture2D() = default;
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

	class Cubemap {
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
		virtual ~Cubemap() = default;
		virtual uint32_t width() const = 0;
		virtual uint32_t height() const = 0;
		virtual void allocate(const AllocInfo& allocInfo) = 0;
		virtual void update(const UpdateInfo& updateInfo) = 0;
		virtual void generateMipmaps() = 0;
	public:
		static Cubemap* create();
	};
}