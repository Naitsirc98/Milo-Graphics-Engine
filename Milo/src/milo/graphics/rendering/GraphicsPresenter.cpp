#include "milo/graphics/rendering/GraphicsPresenter.h"
#include "milo/graphics/Graphics.h"

namespace milo {

	GraphicsPresenter* GraphicsPresenter::get() {
		return Graphics::graphicsContext()->presenter();
	}
}