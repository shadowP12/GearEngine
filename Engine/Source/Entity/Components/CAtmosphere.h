#pragma once
#include "Component.h"
#include "Math/Math.h"

namespace gear {
	class CAtmosphere : public Component {
	public:
		CAtmosphere(Entity* entity);

		virtual ~CAtmosphere();

		static ComponentType GetClassType() { return ComponentType::Atmosphere; }

		ComponentType GetType() override { return ComponentType::Atmosphere; }

		void ShouldRenderAtmosphere(bool enable) { should_render_atmosphere = enable; };

	private:
		friend class Scene;
		bool should_render_atmosphere = true;
	};
}