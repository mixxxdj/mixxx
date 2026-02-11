// Copyright (C) 2020-2026 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "Fourier.h"
#include "Grain.h"
#include "Phase.h"

#include <Eigen/Core>

namespace Bungee {

struct Synthesis
{
	struct Temporal;

	Eigen::ArrayX<Phase::Type> delta;

	Synthesis(int log2TransformLength)
	{
		Fourier::resize<true>(log2TransformLength, 1, delta);
	}

	void synthesise(int log2SynthesisHop, Grain &grain, Grain &previous);
};

} // namespace Bungee
