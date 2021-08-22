#pragma once

#include "PixelFormat.h"

namespace milo {
	using ImageDeallocator = Function<void, void*>;

	class Image {
	private:
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
		PixelFormat m_Format = PixelFormat::Undefined;
		byte_t* m_Pixels = nullptr;
	private:
		ImageDeallocator m_Deallocator;
	public:
		Image(uint32_t width, uint32_t height, PixelFormat format, void *pixels, ImageDeallocator deallocator_ = free);
		Image(const Image& copy) = delete;
		Image(Image&& move) noexcept;
		~Image();
		Image& operator=(const Image& copy) = delete;
		Image& operator=(Image&& move) noexcept;
		inline size_t size() const {return m_Width * m_Height * PixelFormats::size(m_Format);}
		inline uint32_t width() const {return m_Width;}
		inline uint32_t height() const {return m_Height;}
		inline PixelFormat format() const {return m_Format;}
		inline const byte_t* pixels() const {return m_Pixels;}
		inline void* pixels() {return m_Pixels;}
	public:
		static Image* createWhiteImage(PixelFormat format, uint32_t width = 1, uint32_t height = 1);
		static Image* createBlackImage(PixelFormat format, uint32_t width = 1, uint32_t height = 1);
		static Image* createImage(const String& path, PixelFormat format, bool flipY = false);
		static Image* createImage(void* pixels, PixelFormat format, uint32_t width = 1, uint32_t height = 1);
		static Image* createImage(PixelFormat format, uint32_t width = 1, uint32_t height = 1,  uint32_t value = 0);
	};
}
