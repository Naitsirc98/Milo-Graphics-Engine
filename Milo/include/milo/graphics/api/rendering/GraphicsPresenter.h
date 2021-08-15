#pragma once

namespace milo {

	class GraphicsPresenter {
		friend class MiloEngine;
	public:
		virtual bool begin() = 0;
		virtual void end() = 0;
	};
}