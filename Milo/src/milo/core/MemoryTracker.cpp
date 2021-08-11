#include "milo/common/Memory.h"
#include "milo/logging/Log.h"
#include <iostream>

#ifdef _DEBUG

void *operator new(size_t size, const char *file, size_t line) {
	void* ptr = malloc(size);
	milo::MemoryTracker::add(file, line, (uint64_t)ptr, size);
	return ptr;
}

void *operator new[](size_t size, const char *file, size_t line) {
	void* ptr = malloc(size);
	milo::MemoryTracker::add(file, line, (uint64_t)ptr, size);
	return ptr;
}

void operator delete(void *ptr) noexcept {
	milo::MemoryTracker::remove((uint64_t)ptr);
	free(ptr);
}

void operator delete[](void *ptr) noexcept {
	milo::MemoryTracker::remove((uint64_t)ptr);
	free(ptr);
}
#endif

namespace milo {

	static const size_t KB = 1024;
	static const size_t MB = 1024 * 1024;
	static const size_t GB = 1024 * 1024 * 1024;

	static String byteSize(size_t bytes) {
		if(bytes < KB) return fmt::format("{} bytes", bytes);
		if(bytes < MB) return fmt::format("{} KB", bytes / (float)KB);
		if(bytes < GB) return fmt::format("{} MB", bytes / (float)MB);
		return fmt::format("{} GB", bytes / (float)GB);
	}

	SortedMap<uint64_t, Allocation> MemoryTracker::s_Allocations;
	uint64_t MemoryTracker::s_TotalAllocations = 0;
	uint64_t MemoryTracker::s_TotalAllocationSize = 0;

	void MemoryTracker::add(const char *file, size_t line, uint64_t address, size_t size) {
		Allocation allocation = {file, line, address, size, std::chrono::high_resolution_clock::now()};
		s_Allocations[address] = allocation;
		++s_TotalAllocations;
		s_TotalAllocationSize += size;
		String filename = String(allocation.file);
		filename.erase(0, filename.find_last_of("milo")-3);
		Log::warn("Milo Allocation: size={}, file={}({})", byteSize(allocation.size), filename, allocation.line);
	}

	void MemoryTracker::add(uint64_t address, size_t size) {
		Allocation allocation = {"EXTERNAL", 0, address, size, std::chrono::high_resolution_clock::now()};
		s_Allocations[address] = allocation;
		++s_TotalAllocations;
		s_TotalAllocationSize += size;
	}

	void MemoryTracker::remove(uint64_t address) {
		if(s_Allocations.find(address) == s_Allocations.end()) return;
		const size_t size = s_Allocations[address].size;
		s_Allocations.erase(address);
		s_TotalAllocationSize -= size;
	}

	uint64_t MemoryTracker::aliveAllocationsCount() {
		return s_Allocations.size();
	}

	uint64_t MemoryTracker::totalAllocations() {
		return s_TotalAllocations;
	}

	uint64_t MemoryTracker::totalAllocationSize() {
		return s_TotalAllocationSize;
	}

	String MemoryTracker::totalAllocationSizeStr() {
		return byteSize(totalAllocationSize());
	}

	void MemoryTracker::init() {
	}

	void MemoryTracker::shutdown() {
	}

	const SortedMap<uint64_t, Allocation> &MemoryTracker::allocations() {
		return s_Allocations;
	}
}