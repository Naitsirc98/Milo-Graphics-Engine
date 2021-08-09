#pragma once

#include "milo/common/Common.h"

namespace milo {

	class MiloSubSystemManager {
		friend class MiloEngine;
	private:
		// TODO: subsystems in order
	private:
		static void init();
		static void shutdown();
	public:
		MiloSubSystemManager() = delete;
	};
}