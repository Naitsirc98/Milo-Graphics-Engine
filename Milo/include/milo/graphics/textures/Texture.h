#pragma once

#include "milo/logging/Log.h"
#include "milo/assets/images/Image.h"

#define AUTO_MIP_LEVELS 0

namespace milo {

	using TextureId = uint32_t;

	enum TextureUsageFlagBits {
		TEXTURE_USAGE_UNDEFINED_BIT = 0x0,
		TEXTURE_USAGE_SAMPLED_BIT = 0x1,
		TEXTURE_USAGE_COLOR_ATTACHMENT_BIT = 0x2,
		TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT = 0x4,
		TEXTURE_USAGE_STORAGE_BIT = 0x8,
		TEXTURE_USAGE_UI_BIT = 0x10
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
	private:
		TextureId m_Id{NULL};
		TextureUsageFlags m_Usage{TEXTURE_USAGE_UNDEFINED_BIT};
		String m_Name;
	public:
		Texture2D(TextureUsageFlags usage);
		virtual ~Texture2D();
		inline TextureId id() const {return m_Id;}
		inline TextureUsageFlags usage() const {return m_Usage;};
		virtual uint32_t width() const = 0;
		virtual uint32_t height() const = 0;
		const String& name() const {return m_Name;}
		void setName(const String& name) {doSetName(name); m_Name = name;}
		Size size() const {return {(int32_t)width(), (int32_t)height()};}
		virtual void allocate(const AllocInfo& allocInfo) = 0;
		virtual void update(const UpdateInfo& updateInfo) = 0;
		virtual void generateMipmaps() = 0;
	protected:
		virtual void doSetName(const String& name) = 0;
	public:
		static Texture2D* create(TextureUsageFlags usage = TEXTURE_USAGE_SAMPLED_BIT);
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
	private:
		TextureId m_Id{NULL};
		TextureUsageFlags m_Usage{TEXTURE_USAGE_UNDEFINED_BIT};
		String m_Name;
	public:
		Cubemap(TextureUsageFlags usage);
		virtual ~Cubemap();
		inline TextureId id() const {return m_Id;}
		inline TextureUsageFlags usage() const {return m_Usage;};
		virtual uint32_t width() const = 0;
		virtual uint32_t height() const = 0;
		const String& name() const {return m_Name;}
		void setName(const String& name) {doSetName(name); m_Name = name;}
		Size size() const {return {(int32_t)width(), (int32_t)height()};}
		virtual void allocate(const AllocInfo& allocInfo) = 0;
		virtual void update(const UpdateInfo& updateInfo) = 0;
		virtual void generateMipmaps() = 0;
	protected:
		virtual void doSetName(const String& name) = 0;
	public:
		static Cubemap* create(TextureUsageFlags usage = TEXTURE_USAGE_SAMPLED_BIT);
	};
}