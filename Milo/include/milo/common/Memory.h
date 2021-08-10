#pragma once

#include <memory>
#include <milo/time/Time.h>
#include "Collections.h"

#define DELETE_PTR(ptr) {delete ptr; ptr = nullptr;}
#define DELETE_ARRAY(arr) {delete arr; arr = nullptr;}

#ifdef _DEBUG
#define NEW new(__FILE__, __LINE__)
#define DELETE delete
#else
#define NEW new
#define DELETE new
#endif

#ifdef _DEBUG

void* operator new(size_t size, const char* file, size_t line);
void* operator new[](size_t size, const char* file, size_t line);
void operator delete(void* ptr) noexcept;
void operator delete[](void* ptr) noexcept;

#endif

namespace milo {

	template<typename T>
	using SharedPtr = std::shared_ptr<T>;
	using std::make_shared;

	template<typename T>
	using UniquePtr = std::unique_ptr<T>;
	using std::make_unique;

	template<typename T>
	using WeakPtr = std::weak_ptr<T>;

	struct Allocation {
		const char* file;
		size_t line;
		uint64_t address;
		size_t size;
		TimePoint timePoint;
	};

	class MemoryTracker {
		friend class MiloSubSystemManager;
	private:
		static SortedMap<uint64_t, Allocation> s_Allocations;
		static uint64_t s_TotalAllocations;
		static uint64_t s_TotalAllocationSize;
	public:
		static void add(const char *file, size_t line, uint64_t address, size_t size);
		static void add(uint64_t address, size_t size);
		static void remove(uint64_t address);
		static const SortedMap<uint64_t, Allocation>& allocations();
		static uint64_t aliveAllocationsCount();
		static uint64_t totalAllocations();
		static uint64_t totalAllocationSize();
		static String totalAllocationSizeStr();
	private:
		static void init();
		static void shutdown();
	};
}