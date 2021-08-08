#pragma once

#include "milo/common/Common.h"

namespace milo {

	struct EngineExitResult {
		int32 exitCode;
		String message;

		inline bool success() noexcept {return exitCode == MILO_SUCCESS;}
	};

	class MiloEngine {
	private:
		static MiloEngine* s_Instance;
	public:
	};
}