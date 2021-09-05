#include "milo/common/Common.h"

namespace milo {

	static SimulationState g_SimulationState = SimulationState::Editor;

	SimulationState getSimulationState() {
		return g_SimulationState;
	}

	void setSimulationState(SimulationState state) {
		g_SimulationState = state;
	}
}