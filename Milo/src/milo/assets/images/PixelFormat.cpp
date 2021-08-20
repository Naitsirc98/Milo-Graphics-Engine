#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"
#include "milo/assets/images/PixelFormat.h"

namespace milo {

	class Builder {
	private:
		PixelFormatInfo info;
	public:
		constexpr Builder& channels(uint32_t channels) { info.channels = channels; return *this;}
		constexpr Builder& channelsSize(uint32_t channelsSize) { info.channelSize = channelsSize / 8; return *this;};
		constexpr Builder& type(FormatType type) { info.dataType = type; return *this;}
		constexpr PixelFormatInfo build() {return info;}
	};

	const PixelFormatInfo UNDEFINED_INFO = Builder().channels(0).channelsSize(0).type(FormatType::Undefined).build();

	const PixelFormatInfo R8_INFO = Builder().channels(1).channelsSize(8).type(FormatType::Unorm).build();
	const PixelFormatInfo R8I_INFO = Builder().channels(1).channelsSize(8).type(FormatType::Int).build();
	const PixelFormatInfo R8UI_INFO = Builder().channels(1).channelsSize(8).type(FormatType::UInt).build();
	const PixelFormatInfo R8_SNORM_INFO = Builder().channels(1).channelsSize(8).type(FormatType::Snorm).build();

	const PixelFormatInfo R16_INFO = Builder().channels(1).channelsSize(16).type(FormatType::Unorm).build();
	const PixelFormatInfo R16I_INFO = Builder().channels(1).channelsSize(16).type(FormatType::Int).build();
	const PixelFormatInfo R16UI_INFO = Builder().channels(1).channelsSize(16).type(FormatType::UInt).build();
	const PixelFormatInfo R16F_INFO = Builder().channels(1).channelsSize(16).type(FormatType::Float).build();
	const PixelFormatInfo R16_SNORM_INFO = Builder().channels(1).channelsSize(16).type(FormatType::Snorm).build();
	const PixelFormatInfo R32I_INFO = Builder().channels(1).channelsSize(32).type(FormatType::Int).build();
	const PixelFormatInfo R32UI_INFO = Builder().channels(1).channelsSize(32).type(FormatType::UInt).build();
	const PixelFormatInfo R32F_INFO = Builder().channels(1).channelsSize(32).type(FormatType::Float).build();

	const PixelFormatInfo RG8_INFO = Builder().channels(2).channelsSize(8).type(FormatType::Unorm).build();
	const PixelFormatInfo RG8I_INFO = Builder().channels(2).channelsSize(8).type(FormatType::Int).build();
	const PixelFormatInfo RG8UI_INFO = Builder().channels(2).channelsSize(8).type(FormatType::UInt).build();
	const PixelFormatInfo RG8_SNORM_INFO = Builder().channels(2).channelsSize(8).type(FormatType::Snorm).build();
	const PixelFormatInfo RG16_INFO = Builder().channels(2).channelsSize(16).type(FormatType::Unorm).build();
	const PixelFormatInfo RG16I_INFO = Builder().channels(2).channelsSize(16).type(FormatType::Int).build();
	const PixelFormatInfo RG16UI_INFO = Builder().channels(2).channelsSize(16).type(FormatType::UInt).build();
	const PixelFormatInfo RG16F_INFO = Builder().channels(2).channelsSize(16).type(FormatType::Float).build();
	const PixelFormatInfo RG16_SNORM_INFO = Builder().channels(2).channelsSize(16).type(FormatType::Snorm).build();
	const PixelFormatInfo RG32I_INFO = Builder().channels(2).channelsSize(32).type(FormatType::Int).build();
	const PixelFormatInfo RG32UI_INFO = Builder().channels(2).channelsSize(32).type(FormatType::UInt).build();
	const PixelFormatInfo RG32F_INFO = Builder().channels(2).channelsSize(32).type(FormatType::Float).build();

	const PixelFormatInfo RGB8_INFO = Builder().channels(3).channelsSize(8).type(FormatType::Unorm).build();
	const PixelFormatInfo RGB8I_INFO = Builder().channels(3).channelsSize(8).type(FormatType::Int).build();
	const PixelFormatInfo RGB8UI_INFO = Builder().channels(3).channelsSize(8).type(FormatType::UInt).build();
	const PixelFormatInfo RGB8_SNORM_INFO = Builder().channels(3).channelsSize(8).type(FormatType::Snorm).build();
	const PixelFormatInfo RGB16_INFO = Builder().channels(3).channelsSize(16).type(FormatType::Unorm).build();
	const PixelFormatInfo RGB16I_INFO = Builder().channels(3).channelsSize(16).type(FormatType::Int).build();
	const PixelFormatInfo RGB16UI_INFO = Builder().channels(3).channelsSize(16).type(FormatType::UInt).build();
	const PixelFormatInfo RGB16F_INFO = Builder().channels(3).channelsSize(16).type(FormatType::Float).build();
	const PixelFormatInfo RGB16_SNORM_INFO = Builder().channels(3).channelsSize(16).type(FormatType::Snorm).build();
	const PixelFormatInfo RGB32_INFO = Builder().channels(3).channelsSize(32).type(FormatType::Unorm).build();
	const PixelFormatInfo RGB32I_INFO = Builder().channels(3).channelsSize(32).type(FormatType::Int).build();
	const PixelFormatInfo RGB32UI_INFO = Builder().channels(3).channelsSize(32).type(FormatType::UInt).build();
	const PixelFormatInfo RGB32F_INFO = Builder().channels(3).channelsSize(32).type(FormatType::Float).build();

	const PixelFormatInfo RGBA8_INFO = Builder().channels(4).channelsSize(8).type(FormatType::Unorm).build();
	const PixelFormatInfo RGBA8I_INFO = Builder().channels(4).channelsSize(8).type(FormatType::Int).build();
	const PixelFormatInfo RGBA8UI_INFO = Builder().channels(4).channelsSize(8).type(FormatType::UInt).build();
	const PixelFormatInfo RGBA8_SNORM_INFO = Builder().channels(4).channelsSize(8).type(FormatType::Snorm).build();
	const PixelFormatInfo RGBA16_INFO = Builder().channels(4).channelsSize(16).type(FormatType::Unorm).build();
	const PixelFormatInfo RGBA16I_INFO = Builder().channels(4).channelsSize(16).type(FormatType::Int).build();
	const PixelFormatInfo RGBA16UI_INFO = Builder().channels(4).channelsSize(16).type(FormatType::UInt).build();
	const PixelFormatInfo RGBA16F_INFO = Builder().channels(4).channelsSize(16).type(FormatType::Float).build();
	const PixelFormatInfo RGBA16_SNORM_INFO = Builder().channels(4).channelsSize(16).type(FormatType::Snorm).build();
	const PixelFormatInfo RGBA32_INFO = Builder().channels(4).channelsSize(32).type(FormatType::Unorm).build();
	const PixelFormatInfo RGBA32I_INFO = Builder().channels(4).channelsSize(32).type(FormatType::Int).build();
	const PixelFormatInfo RGBA32UI_INFO = Builder().channels(4).channelsSize(32).type(FormatType::UInt).build();
	const PixelFormatInfo RGBA32F_INFO = Builder().channels(4).channelsSize(32).type(FormatType::Float).build();

	const PixelFormatInfo SRGB_INFO = Builder().channels(3).channelsSize(8).type(FormatType::Int).build();
	const PixelFormatInfo SRGBA_INFO = Builder().channels(4).channelsSize(8).type(FormatType::Int).build();

	const PixelFormatInfo DEPTH_INFO = Builder().channels(4).channelsSize(32).type(FormatType::Float).build();


	const PixelFormatInfo& PixelFormats::getInfo(PixelFormat format) noexcept {
		switch (format) {
			case PixelFormat::R8:
				return R8_INFO;
			case PixelFormat::R8I:
				return R8I_INFO;
			case PixelFormat::R8UI:
				return R8UI_INFO;
			case PixelFormat::R8_SNORM:
				return R8_SNORM_INFO;
			case PixelFormat::R16:
				return R16_INFO;
			case PixelFormat::R16I:
				return R16I_INFO;
			case PixelFormat::R16UI:
				return R16UI_INFO;
			case PixelFormat::R16F:
				return R16F_INFO;
			case PixelFormat::R16_SNORM:
				return R16_SNORM_INFO;
			case PixelFormat::R32I:
				return R32I_INFO;
			case PixelFormat::R32UI:
				return R32UI_INFO;
			case PixelFormat::R32F:
				return R32F_INFO;


			case PixelFormat::RG8:
				return RG8_INFO;
			case PixelFormat::RG8I:
				return RG8I_INFO;
			case PixelFormat::RG8UI:
				return RG8UI_INFO;
			case PixelFormat::RG8_SNORM:
				return RG8_SNORM_INFO;
			case PixelFormat::RG16:
				return RG16_INFO;
			case PixelFormat::RG16I:
				return RG16I_INFO;
			case PixelFormat::RG16UI:
				return RG16UI_INFO;
			case PixelFormat::RG16F:
				return RG16F_INFO;
			case PixelFormat::RG16_SNORM:
				return RG16_SNORM_INFO;
			case PixelFormat::RG32I:
				return RG32I_INFO;
			case PixelFormat::RG32UI:
				return RG32UI_INFO;
			case PixelFormat::RG32F:
				return RG32F_INFO;


			case PixelFormat::RGB8:
				return RGB8_INFO;
			case PixelFormat::RGB8I:
				return RGB8I_INFO;
			case PixelFormat::RGB8UI:
				return RGB8UI_INFO;
			case PixelFormat::RGB8_SNORM:
				return RGB8_SNORM_INFO;
			case PixelFormat::RGB16:
				return RGB16_INFO;
			case PixelFormat::RGB16I:
				return RGB16I_INFO;
			case PixelFormat::RGB16UI:
				return RGB16UI_INFO;
			case PixelFormat::RGB16F:
				return RGB16F_INFO;
			case PixelFormat::RGB16_SNORM:
				return RGB16_SNORM_INFO;
			case PixelFormat::RGB32:
				return RGB32_INFO;
			case PixelFormat::RGB32I:
				return RGB32I_INFO;
			case PixelFormat::RGB32UI:
				return RGB32UI_INFO;
			case PixelFormat::RGB32F:
				return RGB32F_INFO;


			case PixelFormat::RGBA8:
				return RGBA8_INFO;
			case PixelFormat::RGBA8I:
				return RGBA8I_INFO;
			case PixelFormat::RGBA8UI:
				return RGBA8UI_INFO;
			case PixelFormat::RGBA8_SNORM:
				return RGBA8_SNORM_INFO;
			case PixelFormat::RGBA16:
				return RGBA16_INFO;
			case PixelFormat::RGBA16I:
				return RGBA16I_INFO;
			case PixelFormat::RGBA16UI:
				return RGBA16UI_INFO;
			case PixelFormat::RGBA16F:
				return RGBA16F_INFO;
			case PixelFormat::RGBA16_SNORM:
				return RGBA16_SNORM_INFO;
			case PixelFormat::RGBA32:
				return RGBA32_INFO;
			case PixelFormat::RGBA32I:
				return RGBA32I_INFO;
			case PixelFormat::RGBA32UI:
				return RGBA32UI_INFO;
			case PixelFormat::RGBA32F:
				return RGBA32F_INFO;


			case PixelFormat::SRGB:
				return SRGB_INFO;
			case PixelFormat::SRGBA:
				return SRGBA_INFO;

			case PixelFormat::DEPTH:
				return DEPTH_INFO;

			default:
				return UNDEFINED_INFO;
		}
	}

	uint32_t PixelFormats::channels(PixelFormat format) noexcept {
		const PixelFormatInfo& info = getInfo(format);
		return info.channels;
	}

	bool PixelFormats::floatingPoint(PixelFormat format) noexcept {
		const PixelFormatInfo& info = getInfo(format);
		return info.dataType == FormatType::Float || info.dataType == FormatType::Snorm;
	}

	PixelFormat PixelFormats::of(uint32_t channels, uint32_t channelSize, FormatType type) noexcept {
		int32_t count = static_cast<int32_t>(PixelFormat::MAX_ENUM);
		for(int32_t i = 1;i < count;++i) {
			PixelFormat format = static_cast<PixelFormat>(i);
			const PixelFormatInfo& info = getInfo(format);
			if(info.channels == channels && info.channelSize == channelSize && info.dataType == type) return format;
		}
		return PixelFormat::Undefined;
	}

	uint32_t PixelFormats::size(PixelFormat format) noexcept {
		const PixelFormatInfo& info = getInfo(format);
		return info.size();
	}

	FormatType PixelFormats::type(PixelFormat format) noexcept {
		const PixelFormatInfo& info = getInfo(format);
		return info.dataType;
	}
}
#pragma clang diagnostic pop