// Copyright (C) 2020-2026 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#define BUNGEE_BASIC_CPP

#include "Stretcher.h"
#include "Assert.h"
#include "Instrumentation.h"
#include "Resample.h"
#include "Synthesis.h"
#include "log2.h"

namespace Bungee {

Internal::Stretcher::Stretcher(SampleRates sampleRates, int channelCount, int log2SynthesisHopAdjust) :
	Timing(sampleRates, log2SynthesisHopAdjust),
	input(log2SynthesisHop, channelCount, transforms),
	grains(4),
	output(transforms, log2SynthesisHop, channelCount, maxOutputFrameCount(true), 0.25f, {1.f, 0.5f}),
	synthesis(log2SynthesisHop + 3)
{
	for (auto &grain : grains.vector)
		grain = std::make_unique<Grain>(log2SynthesisHop, channelCount);

	grains.prepare();

	Fourier::resize<true>(log2SynthesisHop + 3, 1, temporary);
	Fourier::resize<true>(log2SynthesisHop + 3, channelCount, transformed);
}

InputChunk Internal::Stretcher::specifyGrain(const Request &request, double bufferStartPosition)
{
	Instrumentation::Call call(*this, 0);
	const Assert::FloatingPointExceptions floatingPointExceptions(0);

	grains.rotate();

	auto &grain = grains[0];
	auto &previous = grains[1];

	return grain.specify(request, previous, sampleRates, log2SynthesisHop, bufferStartPosition, *this);
}

void Internal::Stretcher::analyseGrain(const float *data, std::ptrdiff_t stride, int muteFrameCountHead, int muteFrameCountTail)
{
	Instrumentation::Call call(*this, 1);
	const Assert::FloatingPointExceptions floatingPointExceptions(FE_INEXACT | FE_UNDERFLOW | FE_DENORMALOPERAND);

	auto &grain = grains[0];
	const auto &previous = grains[1];

	Instrumentation::log("analyseGrain: position=%f speed=%f pitch=%f reset=%s data=%p stride=%d mute=%d:%d", grain.request.position, grain.request.speed, grain.request.pitch, grain.request.reset ? "true" : "false", data, stride, muteFrameCountHead, muteFrameCountTail);

	grain.muteFrameCountHead = muteFrameCountHead;
	grain.muteFrameCountTail = muteFrameCountTail;

	grain.validBinCount = 0;
	if (grain.valid())
	{
		auto m = grain.inputChunkMap(data, stride, muteFrameCountHead, muteFrameCountTail, previous, *this);

		auto ref = grain.resampleInput(m, log2SynthesisHop + 3, muteFrameCountHead, muteFrameCountTail, input.resampled);

		auto log2TransformLength = input.applyAnalysisWindow(ref, muteFrameCountHead, muteFrameCountTail);

		transforms.forward(log2TransformLength, input.windowedInput, transformed);

		const auto n = Fourier::binCount(grain.log2TransformLength) - 1;
		grain.validBinCount = std::min<int>(std::ceil(n / grain.resampleOperations.output.ratio), n) + 1;
		transformed.middleRows(grain.validBinCount, n + 1 - grain.validBinCount).setZero();

		grain.log2TransformLength = log2TransformLength;

		for (int i = 0; i < grain.validBinCount; ++i)
		{
			const auto x = transformed.row(i).sum();
			grain.energy[i] = x.real() * x.real() + x.imag() * x.imag();
			grain.phase[i] = Phase::fromRadians(std::arg(x));
		}

		Partials::enumerate(grain.partials, grain.validBinCount, grain.energy);

		if (grain.continuous)
			Partials::suppressTransientPartials(grain.partials, grain.energy, grains[1].energy);
	}
}

void Internal::Stretcher::synthesiseGrain(OutputChunk &outputChunk)
{
	Instrumentation::Call call(*this, 2);

	const Assert::FloatingPointExceptions floatingPointExceptions(FE_INEXACT);

	auto &grain = grains[0];
	if (grain.valid())
	{
		BUNGEE_ASSERT1(!grain.passthrough || grain.analysis.speed == grain.passthrough);

		synthesis.synthesise(log2SynthesisHop, grain, grains[1]);

		BUNGEE_ASSERT2(!grain.passthrough || grain.rotation.topRows(grain.validBinCount).isZero());

		auto t = temporary.topRows(grain.validBinCount);

		t = grain.rotation.topRows(grain.validBinCount).cast<float>() * (std::complex<float>{0, std::numbers::pi_v<float> / 0x8000});
		t = t.exp();

		if (grain.reverse())
			transformed.topRows(grain.validBinCount) = transformed.topRows(grain.validBinCount).conjugate().colwise() * t;
		else
			transformed.topRows(grain.validBinCount) = transformed.topRows(grain.validBinCount).colwise() * t;

		transforms.inverse(grain.log2TransformLength, output.inverseTransformed, transformed);
	}

	output.applySynthesisWindow(log2SynthesisHop, grains, output.synthesisWindow);

	outputChunk = output.resample(grains[2].resampleOperations.output, grains[1].resampleOperations.output);

	outputChunk.request[OutputChunk::begin] = &grains[2].request;
	outputChunk.request[OutputChunk::end] = &grains[1].request;
}

extern const char *versionDescription;
} // namespace Bungee

const Bungee::Functions *getFunctionsBungeeBasic()
{
	static const char *edition = "Basic";
	static const Bungee::Internal::Functions<Bungee::Internal::Stretcher, &edition, &Bungee::versionDescription> functions{};
	return &functions;
}
