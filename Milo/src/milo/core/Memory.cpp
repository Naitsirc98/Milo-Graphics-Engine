#include "milo/common/Memory.h"
#include "milo/common/Exceptions.h"
#include "milo/logging/Log.h"
#include <iostream>

#ifdef _DEBUG
void *operator new(size_t size) {
	void* ptr = malloc(size);
	milo::MemoryTracker::add((uint64_t)(ptr), size);
	return ptr;
}

void *operator new[](size_t size) {
	void* ptr = malloc(size);
	milo::MemoryTracker::add((uint64_t)(ptr), size);
	return ptr;
}

void operator delete(void *ptr) {
	milo::MemoryTracker::remove((uint64_t)ptr);
	free(ptr);
}

void operator delete[](void *ptr) {
	milo::MemoryTracker::remove((uint64_t)ptr);
	free(ptr);
}
#endif

namespace milo {

	static const size_t KB = 1024;
	static const size_t MB = 1024 * 1024;
	static const size_t GB = 1024 * 1024 * 1024;

	static String memoryStr(size_t bytes) {
		if(bytes < KB) return fmt::format("{} bytes", bytes);
		if(bytes < MB) return fmt::format("{} KB", bytes / (float)KB);
		if(bytes < GB) return fmt::format("{} MB", bytes / (float)MB);
		return fmt::format("{} GB", bytes / (float)GB);
	}

	AtomicBool MemoryTracker::s_Active = false;
	HashMap<uint64_t, size_t>* MemoryTracker::s_Allocations = nullptr;
	AtomicULong MemoryTracker::s_TotalAllocations = 0;
	AtomicULong MemoryTracker::s_TotalAllocationSize = 0;

	void MemoryTracker::add(uint64_t address, size_t size) {
		if(!s_Active) return;
		++s_TotalAllocations;
		s_TotalAllocationSize += size;
		s_Active = false;
		allocations()[address] = size;
		s_Active = true;
	}

	void MemoryTracker::remove(uint64_t address) {
		if(!s_Active) return;
		if(allocations().find(address) == allocations().end()) return;
		size_t size = allocations()[address];
		allocations().erase(address);
		s_TotalAllocationSize -= size;
	}

	uint64_t MemoryTracker::aliveAllocationsCount() {
		return s_Allocations->size();
	}

	uint64_t MemoryTracker::totalAllocations() {
		return s_TotalAllocations;
	}

	uint64_t MemoryTracker::totalAllocationSize() {
		return s_TotalAllocationSize;
	}

	String MemoryTracker::totalAllocationSizeStr() {
		return memoryStr(totalAllocationSize());
	}

	void MemoryTracker::init() {
		s_Allocations = new HashMap<uint64_t, size_t>();
		s_Allocations->reserve(1024 * 1024);
		s_Active = true;
	}

	void MemoryTracker::shutdown() {
		s_Active = false;
		DELETE_PTR(s_Allocations);
	}

	inline HashMap<uint64_t, size_t>& MemoryTracker::allocations() {
		return *s_Allocations;
	}
}