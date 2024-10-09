#pragma once

#include <utility>
#include "TypesCLike.hpp"
#include "CardTemplate.h"

namespace capi {
	inline CardId CardIdWithUpgrade(card_templates::CardTemplate ct, Upgrade u) {
		return CardId(ct + u);
	}
	inline Position2D to2D(capi::Position p) {
		auto pos2d = Position2D();
		pos2d.x = p.x;
		pos2d.y = p.z;
		return pos2d;
	}
}
