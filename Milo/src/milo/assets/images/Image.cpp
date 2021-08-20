#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "milo/assets/images/Image.h"
#include <stb_image.h>
#include <stb_image_write.h>
#include "milo/io/Files.h"

namespace milo {

	Image::Image(uint32_t width, uint32_t height, PixelFormat format, void *pixels, ImageDeallocator deallocator_)
		: m_Width(width), m_Height(height), m_Format(format), m_Pixels((byte_t*)pixels), m_Deallocator(std::move(deallocator_)) {
	}

	Image::Image(Image&& move) noexcept {
		m_Width = move.m_Width;
		m_Height = move.m_Height;
		m_Format = move.format();
		m_Pixels = move.m_Pixels;
		move.m_Pixels = nullptr;
	}

	Image::~Image() {
		if(m_Pixels == nullptr) return;
		m_Deallocator(m_Pixels);
		m_Pixels = nullptr;
	}

	Image &Image::operator=(Image&& move) noexcept {
		if(&move == this) return *this;

		m_Width = move.m_Width;
		m_Height = move.m_Height;
		m_Format = move.m_Format;

		m_Pixels = move.m_Pixels;
		m_Deallocator = move.m_Deallocator;
		move.m_Pixels = nullptr;

		return *this;
	}

	inline void* allocate(PixelFormat format, uint32_t width, uint32_t height, uint32_t value) {
		size_t size = width * height * PixelFormats::size(format);
		void* pixels = malloc(size);
		memset(pixels, value, size);
		return pixels;
	}

	Image* ImageFactory::createWhiteImage(PixelFormat format, uint32_t width, uint32_t height) {
		return createImage(format, width, height, UINT32_MAX);
	}

	Image* ImageFactory::createBlackImage(PixelFormat format, uint32_t width, uint32_t height) {
		return createImage(format, width, height, 0);
	}

	Image* ImageFactory::createImage(PixelFormat format, uint32_t width, uint32_t height, uint32_t value) {
		return createImage(allocate(format, width, height, value), format, width, height);
	}

	Image* ImageFactory::createImage(void* pixels, PixelFormat format, uint32_t width, uint32_t height) {
		return new Image(width, height, format, pixels, free);
	}

	Image* ImageFactory::createImage(const String &path, PixelFormat format, bool flipY) {

		int32_t width;
		int32_t height;
		int32_t channels;
		int32_t desiredChannels = format == PixelFormat::Undefined ? STBI_default : PixelFormats::channels(format);
		void* pixels;

		const auto fileContents = Files::readAllBytes(path);
		const auto* rawData = reinterpret_cast<const stbi_uc*>(fileContents.data());

		if(rawData == nullptr) {
			throw MILO_RUNTIME_EXCEPTION(String("Could not read fileContents ").append(path));
		}

		stbi_set_flip_vertically_on_load(flipY);

		if(format != PixelFormat::Undefined && PixelFormats::floatingPoint(format))
			pixels = stbi_loadf_from_memory(rawData, (int32_t)fileContents.size(), &width, &height, &channels, desiredChannels);
		else
			pixels = stbi_load_from_memory(rawData, (int32_t)fileContents.size(), &width, &height, &channels, desiredChannels);

		if(pixels == nullptr)
			throw MILO_RUNTIME_EXCEPTION(String("Failed to create image from file: ").append(stbi_failure_reason()));

		if(format == PixelFormat::Undefined) format = PixelFormats::of(channels);

		return new Image(width, height, format, pixels, stbi_image_free);
	}
}