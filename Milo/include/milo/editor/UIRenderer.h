#pragma once

#include "milo/common/Common.h"

namespace milo {

	class UIRenderer {
		friend class MiloEditor;
	protected:
		UIRenderer() = default;
		virtual ~UIRenderer() = default;
	public:
		virtual void begin() = 0;
		virtual void end() = 0;
		// TODO
	};

}