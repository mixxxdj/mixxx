// Copyright (C) 2020-2026 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#include "Timing.h"
#include "Grain.h"
#include "Resample.h"
#include "log2.h"

#include "bungee/Bungee.h"

#include <cstdint>

namespace Bungee {

Timing::Timing(SampleRates sampleRates, int log2SynthesisHopAdjust) :
	log2SynthesisHop(log2<true>(sampleRates.input) - 6 + log2SynthesisHopAdjust),
	sampleRates(sampleRates)
{
}

namespace {
static constexpr auto maxPitchOctaves = 2;
}

int Timing::maxInputFrameCount(bool mayDownsampleInput) const
{
	const auto max = (int64_t(sampleRates.input) << (maxPitchOctaves + log2SynthesisHop + 3)) / sampleRates.output;
	return int(max + 1);
}

int Timing::maxOutputFrameCount(bool mayUpsampleOutput) const
{
	const auto max = (int64_t(sampleRates.output) << (maxPitchOctaves + log2SynthesisHop)) / sampleRates.input;
	return int(max + 1);
}

double Timing::calculateInputHop(const Request &request) const
{
	const double unitHop = (1 << log2SynthesisHop) * Resample::Operations().setup(sampleRates, request.pitch, request.resampleMode);
	return unitHop * request.speed;
}

void Timing::preroll(Request &request) const
{
	request.position -= calculateInputHop(request);
	request.reset = true;
}

void Timing::next(Request &request) const
{
	if (!std::isnan(request.speed) && !std::isnan(request.position))
	{
		request.position += calculateInputHop(request);
		request.reset = false;
	}
}

} // namespace Bungee
