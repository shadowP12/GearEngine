#include "CAtmosphere.h"
#include "Entity/Entity.h"
#include "GearEngine.h"

namespace gear {
	CAtmosphere::CAtmosphere(Entity* entity)
		:Component(entity) {
	}

	CAtmosphere::~CAtmosphere() {
	}
}