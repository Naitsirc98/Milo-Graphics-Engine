#pragma once

#include "milo/logging/Log.h"

namespace milo {

	class Buffer {
	public:
		enum class Type { Undefined, Vertex, Index, Uniform, Storage, Indirect };

		struct AllocInfo {
			uint64_t size = 0;
			const void* data = nullptr;
			const void* apiInfo = nullptr;
		};

		struct UpdateInfo {
			uint64_t offset = 0;
			uint64_t size = 0;
			const void* data = nullptr;
			const void* apiInfo = nullptr;
		};

		enum class MapState {
			Unavailable, Unmapped, Mapped, AlwaysMapped
		};

	public:
		Buffer() = default;
		virtual ~Buffer() = default;
		virtual uint64_t size() const = 0;
		virtual void allocate(const AllocInfo& allocInfo) = 0;
		virtual void update(const UpdateInfo& updateInfo) = 0;
		virtual MapState mapState() = 0;
		virtual void* mappedMemory() = 0;
		virtual void flush(uint64_t offset, uint64_t size) = 0;
		virtual void flush() = 0;
		virtual void* map() = 0;
		virtual void unmap() = 0;
		virtual void mapAndRun(Function<void, void*> func) = 0;
	};
}