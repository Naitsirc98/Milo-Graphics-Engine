#pragma once

#include <memory>
#include <milo/time/Time.h>
#include "Exceptions.h"
#include "Collections.h"
#include "Concurrency.h"

#define DELETE_PTR(ptr) {delete ptr; ptr = nullptr;}
#define DELETE_ARRAY(arr) {delete[] arr; arr = nullptr;}

#ifdef _DEBUG

void* operator new(size_t size);
void* operator new[](size_t size);
void operator delete(void* ptr);
void operator delete[](void* ptr);

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

	// TODO: make this thread safe
	class MemoryTracker {
		friend class MiloSubSystemManager;
	private:
		static AtomicBool s_Active;
		static HashMap<uint64_t, size_t>* s_Allocations;
		static AtomicULong s_TotalAllocations;
		static AtomicULong s_TotalAllocationSize;
	public:
		static void add(uint64_t address, size_t size);
		static void remove(uint64_t address);
		static uint64_t aliveAllocationsCount();
		static uint64_t totalAllocations();
		static uint64_t totalAllocationSize();
		static String totalAllocationSizeStr();
	private:
		static void init();
		static void shutdown();
		static HashMap<uint64_t, size_t>& allocations();
	};
}