// Copyright (C) 2020-2026 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "Assert.h"
#include "Grains.h"
#include "Input.h"
#include "Instrumentation.h"
#include "Output.h"
#include "Synthesis.h"
#include "Timing.h"

#include <memory>

namespace Bungee::Internal {

struct Stretcher :
	Timing,
	Instrumentation
{
	Fourier::Transforms transforms;
	Input input;
	Grains grains;
	Output output;
	Synthesis synthesis;
	Eigen::ArrayXcf temporary;
	Eigen::ArrayXXcf transformed;

	Stretcher(SampleRates sampleRates, int channelCount, int log2SynthesisHopAdjust);

	void enableInstrumentation(bool enable);

	InputChunk specifyGrain(const Request &request, double bufferStartPosition);

	void analyseGrain(const float *inputAudio, std::ptrdiff_t stride, int muteFrameCountHead, int muteFrameCountTail);

	void synthesiseGrain(OutputChunk &outputChunk);

	bool isFlushed() const;
};

template <class S, const char *const *e, const char *const *v>
struct Functions :
	Bungee::Functions
{
	constexpr Functions()
	{
		edition = []() { return *e; };
		version = []() { return *v; };
		create = [](SampleRates sampleRates, int channelCount, int log2SynthesisHop) { return (void *)new S(sampleRates, channelCount, log2SynthesisHop); };
		destroy = [](void *stretcher) { delete reinterpret_cast<S *>(stretcher); };
		enableInstrumentation = [](void *stretcher, int enable) { reinterpret_cast<S *>(stretcher)->Instrumentation::enableInstrumentation(enable); };
		maxInputFrameCount = [](const void *stretcher) { return reinterpret_cast<const S *>(stretcher)->maxInputFrameCount(true); };
		preroll = [](const void *stretcher, Request *request) { reinterpret_cast<const S *>(stretcher)->preroll(*request); };
		next = [](const void *stretcher, Request *request) { reinterpret_cast<const S *>(stretcher)->next(*request); };
		specifyGrain = [](void *stretcher, const Request *request, double bufferStartPosition) { return reinterpret_cast<S *>(stretcher)->specifyGrain(*request, bufferStartPosition); };
		analyseGrain = [](void *stretcher, const float *data, intptr_t channelStride, int muteFrameCountHead, int muteFrameCountTail) { reinterpret_cast<S *>(stretcher)->analyseGrain(data, channelStride, muteFrameCountHead, muteFrameCountTail); };
		synthesiseGrain = [](void *stretcher, OutputChunk *outputChunk) { reinterpret_cast<S *>(stretcher)->synthesiseGrain(*outputChunk); };
		isFlushed = [](const void *stretcher) { return reinterpret_cast<const S *>(stretcher)->grains.flushed(); };
	}
};

} // namespace Bungee::Internal
