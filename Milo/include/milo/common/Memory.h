#pragma once

#include <memory>
#include <milo/time/Time.h>
#include "Exceptions.h"
#include "Collections.h"
#include "Concurrency.h"

#define DELETE_PTR(ptr) {delete ptr; ptr = nullptr;}
#define DELETE_ARRAY(arr) {delete[] arr; arr = nullptr;}

#ifdef _DEBUG
#define DELETE_REF(ref) {if(ref.unique()) {delete ref.get(); ref.reset();} else { LOG_WARN(str("Reference ") + str(#ref) + " requested to be deleted, but there are still alive references to that object");} }
#else
#define DELETE_REF(ref) delete ref.get()
#endif

#ifdef _DEBUG


void* operator new(size_t size);
void* operator new[](size_t size);
void operator delete(void* ptr);
void operator delete[](void* ptr);

#endif

namespace milo {

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