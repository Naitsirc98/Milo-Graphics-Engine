#pragma once

#include "milo/common/Common.h"
#include "milo/assets/images/PixelFormat.h"
#include "milo/graphics/buffer/Buffer.h"
#include "milo/graphics/textures/Texture.h"

namespace milo {

	struct FramebufferColorAttachment {

		PixelFormat format = PixelFormat::SRGBA;

		inline bool operator==(const FramebufferColorAttachment& rhs) const {
			return format == rhs.format;
		}

		inline bool operator!=(const FramebufferColorAttachment& rhs) const {
			return !(rhs == *this);
		}
	};

	struct FramebufferDepthStencilAttachment {

		PixelFormat format = PixelFormat::DEPTH;

		inline bool operator==(const FramebufferDepthStencilAttachment& rhs) const {
			return format == rhs.format;
		}

		inline bool operator!=(const FramebufferDepthStencilAttachment& rhs) const {
			return !(rhs == *this);
		}
	};

	struct FramebufferDescription {

		uint32_t width = 0;
		uint32_t height = 0;
		FramebufferColorAttachment colorAttachments[32]{};
		uint32_t colorAttachmentCount = 0;
		FramebufferDepthStencilAttachment depthStencilAttachment;
		bool useDepthStencilAttachment = true;

		inline bool operator==(const FramebufferDescription& rhs) const {
			return width == rhs.width &&
				   height == rhs.height &&
				   colorAttachments == rhs.colorAttachments &&
				   colorAttachmentCount == rhs.colorAttachmentCount &&
				   depthStencilAttachment == rhs.depthStencilAttachment &&
				   useDepthStencilAttachment == rhs.useDepthStencilAttachment;
		}

		inline bool operator!=(const FramebufferDescription& rhs) const {
			return  !(rhs == *this);
		}
	};

	class FrameGraphResourcePool;

	struct ResourceMetaData {
		FrameGraphResourcePool* pool = nullptr;
		uint16_t useCount = 0;
	};

	// ====

	struct BufferDescription {

		Buffer::Type type = Buffer::Type::Undefined;
		uint64_t size = 0;

		inline bool operator==(const BufferDescription& rhs) const {
			return type == rhs.type &&
				   size == rhs.size;
		}

		inline bool operator!=(const BufferDescription& rhs) const {
			return !(rhs == *this);
		}
	};

	// ====

	struct Texture2DDescription {

		TextureUsageFlags usageFlags = TEXTURE_USAGE_UNDEFINED_BIT;
		uint32_t width = 0;
		uint32_t height = 0;
		PixelFormat format = PixelFormat::Undefined;
		uint32_t mipLevels = 1;

		inline bool operator==(const Texture2DDescription& rhs) const {
			return width == rhs.width &&
				   height == rhs.height &&
				   format == rhs.format &&
				   mipLevels == rhs.mipLevels;
		}

		inline bool operator!=(const Texture2DDescription& rhs) const {
			return !(rhs == *this);
		}
	};

	// ===

	using ResourceHandle = uint64_t;

	struct FrameGraphBuffer {
		friend class FrameGraphResourcePool;
		friend class VulkanFrameGraphResourcePool;

		ResourceHandle handle = NULL;
		BufferDescription description = {};
		Buffer* buffer = nullptr;

		bool operator==(const FrameGraphBuffer& rhs) const {
			return handle == rhs.handle;
		}

		bool operator!=(const FrameGraphBuffer& rhs) const {
			return !(rhs == *this);
		}

		inline bool operator==(ResourceHandle handle) const noexcept {
			return this->handle == handle;
		}

		inline bool operator!=(ResourceHandle handle) const noexcept {
			return this->handle != handle;
		}

	private:
		mutable uint16_t useCount = 0;
	};

	struct FrameGraphTexture2D {
		friend class FrameGraphResourcePool;
		friend class VulkanFrameGraphResourcePool;

		ResourceHandle handle = NULL;
		Texture2DDescription description = {};
		Texture2D* texture = nullptr;

		bool operator==(const FrameGraphTexture2D& rhs) const {
			return handle == rhs.handle;
		}

		bool operator!=(const FrameGraphTexture2D& rhs) const {
			return !(rhs == *this);
		}

		inline bool operator==(ResourceHandle handle) const noexcept {
			return this->handle == handle;
		}

		inline bool operator!=(ResourceHandle handle) const noexcept {
			return this->handle != handle;
		}

	private:
		mutable uint16_t useCount = 0;
	};

	struct RenderTarget {
		Size size{0, 0};
		Texture2D* colorAttachment{nullptr};
		Texture2D* depthAttachment{nullptr};
	};

	class FrameGraphResourcePool {
		friend class WorldRenderer;
	protected:
		FrameGraphResourcePool() = default;
		virtual ~FrameGraphResourcePool() = default;
	public:
		virtual void update() = 0;

		virtual void clearReferences() = 0;

		virtual FrameGraphBuffer getBuffer(ResourceHandle handle) = 0;
		virtual FrameGraphBuffer getBuffer(const BufferDescription& description) = 0;
		virtual void destroyBuffer(ResourceHandle handle) = 0;

		virtual FrameGraphTexture2D getTexture2D(ResourceHandle handle) = 0;
		virtual FrameGraphTexture2D getTexture2D(const Texture2DDescription& description) = 0;
		virtual void destroyTexture(ResourceHandle handle) = 0;

		virtual const RenderTarget& getRenderTarget(uint32_t index = UINT32_MAX) const = 0;

		virtual void freeUnreferencedResources() = 0;
	public:
		static FrameGraphResourcePool* create();
	};

}