// Copyright (C) 2020-2026 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#include "Synthesis.h"

#include "Dispatch.h"
#include "Grains.h"
#include "Stretch.h"
#include "log2.h"

namespace Bungee {

static constexpr int flagReverse0 = 1 << 0;
static constexpr int flagReverse1 = 1 << 1;

struct Synthesis::Temporal
{
	template <int index>
	static void special(int log2SynthesisHop, Grain &grain, Grain &previous, Eigen::ArrayX<Phase::Type> &delta)
	{
		typedef Stretch::Time<!!(index & flagReverse0), !!(index & flagReverse1)> StretchTime;

		const StretchTime stretchTime(log2SynthesisHop, grain.analysis.hop, previous.analysis.hop);

		BUNGEE_ASSERT1(grain.partials.back().end == grain.validBinCount);

		for (int i = 0; i < grain.partials.size(); ++i)
		{
			const auto peak = grain.partials[i].peak;

			const Phase::Type offset = StretchTime::offset(grain.phase[peak], previous.phase[peak]);
			const Phase::Type stretched = stretchTime.delta(grain.phase[peak], previous.phase[peak], peak);
			delta[i] = previous.rotation[peak] - offset + stretched;
			BUNGEE_ASSERT2(!grain.passthrough || !delta[i]);

			delta[i] -= grain.rotation[peak];
		}
	}
};

void Synthesis::synthesise(int log2SynthesisHop, Grain &grain, Grain &previous)
{
	Stretch::Frequency(grain.analysis.speed)(grain.validBinCount, grain.rotation, grain.phase);
	BUNGEE_ASSERT2(!grain.passthrough || grain.rotation.topRows(grain.validBinCount).isZero());

	if (grain.continuous)
	{
		int index = 0;
		if (grain.reverse())
			index |= flagReverse0;
		if (previous.reverse())
			index |= flagReverse1;

		static constexpr Dispatch<Temporal, 4> dispatch;
		dispatch[index](log2SynthesisHop, grain, previous, delta);
	}
	else
	{
		for (int i = 0; i < grain.partials.size(); ++i)
			delta[i] = -grain.rotation[grain.partials[i].peak];
	}

	for (int i = 0, n = 0; i < grain.partials.size(); ++i)
		do
		{
			grain.rotation[n] += delta[i];
			BUNGEE_ASSERT1(!grain.passthrough || !grain.rotation[n]);
		} while (++n < grain.partials[i].end);

	BUNGEE_ASSERT2(!grain.passthrough || grain.rotation.topRows(grain.validBinCount).isZero());

	const auto mNyquist = Fourier::binCount(grain.log2TransformLength) - 1;
	grain.rotation[mNyquist] = grain.rotation[mNyquist - 1];
}

} // namespace Bungee
