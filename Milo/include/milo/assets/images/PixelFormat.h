#pragma once

#include "milo/common/Common.h"

namespace milo {

	enum class PixelFormat {
		Undefined,

		R8,
		R8I,
		R8UI,
		R16,
		R16I,
		R16UI,
		R32I,
		R32UI,
		R16F,
		R32F,
		R8_SNORM,
		R16_SNORM,

		RG8,
		RG8I,
		RG8UI,
		RG8_SNORM,
		RG16,
		RG16I,
		RG16UI,
		RG16F,
		RG32I,
		RG32UI,
		RG32F,
		RG16_SNORM,

		RGB8,
		RGB8I,
		RGB8UI,
		RGB8_SNORM,
		RGB16,
		RGB16I,
		RGB16UI,
		RGB16F,
		RGB16_SNORM,
		RGB32,
		RGB32I,
		RGB32UI,
		RGB32F,

		RGBA8,
		RGBA8I,
		RGBA8UI,
		RGBA8_SNORM,
		RGBA16,
		RGBA16I,
		RGBA16UI,
		RGBA16F,
		RGBA16_SNORM,
		RGBA32,
		RGBA32I,
		RGBA32UI,
		RGBA32F,

		SRGB,
		SRGBA,

		DEPTH,

		MAX_ENUM
	};

	enum class FormatType {
		Undefined,
		Unorm,
		Int,
		UInt,
		Float,
		Snorm
	};

	struct PixelFormatInfo {
		uint32_t channels;
		uint32_t channelSize;
		FormatType dataType;
		constexpr uint32_t size() const noexcept {return channels * channelSize;}
	};

	class PixelFormats {
	public:
		static const PixelFormatInfo& getInfo(PixelFormat format) noexcept;
		static uint32_t channels(PixelFormat format) noexcept;
		static bool floatingPoint(PixelFormat format) noexcept;
		static FormatType type(PixelFormat format) noexcept;
		static uint32_t size(PixelFormat format) noexcept;
		static PixelFormat of(uint32_t channels, uint32_t channelSize = 8, FormatType type = FormatType::Unorm) noexcept;
	};
}