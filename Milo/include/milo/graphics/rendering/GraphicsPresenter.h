#pragma once

#include "milo/common/Memory.h"

namespace milo {

	class GraphicsPresenter {
		friend class MiloEngine;
	public:
		virtual bool begin() = 0;
		virtual void end() = 0;
	public:
		static GraphicsPresenter* get();
	};
}