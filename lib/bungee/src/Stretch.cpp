// Copyright (C) 2020-2026 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#include "Stretch.h"
#include "Assert.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace Bungee::Stretch {

Frequency::Frequency(float speed)
{
	const Assert::FloatingPointExceptions floatingPointExceptions(FE_INEXACT);

	const auto preventDivideByZero = 1e-20f;
	speed = std::abs(speed) + preventDivideByZero;

	multiplier = (int32_t)std::max<float>(std::round((1 << shift) / -speed), std::numeric_limits<int16_t>::min());
	BUNGEE_ASSERT1(multiplier <= 0);
}

void Frequency::operator()(int n, Eigen::Ref<Eigen::ArrayX<Phase::Type>> rotation, const Eigen::Ref<Eigen::ArrayX<Phase::Type>> &phase) const
{
	rotation[0] = 0;
	for (int m = 1; m < n; ++m)
	{
		Phase::Type delta = phase[m - 1] - phase[m];

		int32_t x = delta;
		x *= multiplier;
		x >>= shift;

		rotation[m] = rotation[m - 1] + Phase::Type(x) + delta;
	}
}

} // namespace Bungee::Stretch
