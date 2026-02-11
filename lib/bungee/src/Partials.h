// Copyright (C) 2020-2026 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "Assert.h"

#include <Eigen/Core>

#include <cstdint>
#include <vector>

namespace Bungee::Partials {

struct Partial
{
	int16_t peak;
	int16_t end;
};

void enumerate(std::vector<Partial> &partials, int n, Eigen::Ref<Eigen::ArrayX<float>> energy);

void suppressTransientPartials(std::vector<Partial> &partials, const Eigen::Ref<const Eigen::ArrayX<float>> energy, const Eigen::Ref<const Eigen::ArrayX<float>> previousEnergy);

} // namespace Bungee::Partials
