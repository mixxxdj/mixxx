// Copyright (C) 2020-2026 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "Assert.h"
#include "Phase.h"

#include <Eigen/Core>
#include <cstdint>

namespace Bungee::Stretch {

struct Frequency
{
	static constexpr auto shift = 8;
	int32_t multiplier;

	Frequency(float speed);

	void operator()(int n, Eigen::Ref<Eigen::ArrayX<Phase::Type>> rotation, const Eigen::Ref<Eigen::ArrayX<Phase::Type>> &phase) const;
};

template <bool reverse, bool reversePrevious>
struct Time
{
	static constexpr auto log2SynthesisHopRevolution = -3;

	int32_t a;
	int32_t multiplier = 0;

	Time(int log2SynthesisHop, int analysisHop, [[maybe_unused]] int analysisHopPrevious)
	{
		BUNGEE_ASSERT1(reverse ^ (analysisHop >= 0));
		BUNGEE_ASSERT1(reversePrevious ^ (analysisHopPrevious >= 0));

		const auto log2TransformLength = log2SynthesisHop - log2SynthesisHopRevolution;
		a = int32_t(analysisHop) << (32 - log2TransformLength);

		const auto dividend = int32_t(1 << log2SynthesisHop) << 16;
		const auto divisor = int32_t(analysisHop) << 1;
		if (divisor)
			multiplier = (dividend + std::abs(divisor) / 2) / divisor;
	}

	static inline constexpr Phase::Type offset(Phase::Type phase, Phase::Type previous)
	{
		return (reverse ? -phase : phase) - (reversePrevious ? -previous : previous);
	}

	inline int32_t delta(int32_t phase, int32_t previous, int m) const
	{
		constexpr auto logS = 32 + log2SynthesisHopRevolution;
		const int32_t da = (phase - previous) - m * a;
		return (m << logS) + (da >> 15) * multiplier;
	}

	inline int16_t delta(int16_t phase, int16_t previous, int m) const
	{
		return delta(int32_t(phase) << 16, int32_t(previous) << 16, m) >> 16;
	}
};

} // namespace Bungee::Stretch
