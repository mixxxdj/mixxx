// Copyright (C) 2020-2026 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#include "Grain.h"
#include "Fourier.h"
#include "Instrumentation.h"

#include "bungee/Bungee.h"

#include <limits>

namespace Bungee {

using namespace Internal;

Grain::Grain(int log2SynthesisHop, int channelCount) :
	channelCount(channelCount),
	log2TransformLength(log2SynthesisHop + 3)
{
	request.position = request.speed = std::numeric_limits<float>::quiet_NaN();
	request.pitch = 1.;
	Fourier::resize<true>(log2TransformLength, 1, energy);
	Fourier::resize<true>(log2TransformLength, 1, rotation);
	partials.reserve(1 << log2TransformLength);
}

InputChunk Grain::specify(const Request &r, Grain &previous, SampleRates sampleRates, int log2SynthesisHop, double bufferStartPosition, Internal::Instrumentation &instrumentation)
{
	request = r;
	BUNGEE_ASSERT1(request.pitch > 0.);

	const Assert::FloatingPointExceptions floatingPointExceptions(FE_INEXACT);
	const auto unitHop = (1 << log2SynthesisHop) * resampleOperations.setup(sampleRates, request.pitch, request.resampleMode);

	requestHop = request.position - previous.request.position;

	if (instrumentation.firstGrain)
		instrumentation.log("Stretcher: sampleRates=[%d, %d] channelCount=%d  synthesisHop=%d", sampleRates.input, sampleRates.output, inputCopy.cols(), 1 << log2TransformLength >> 3);
	instrumentation.firstGrain = false;

	if (!request.reset && !std::isnan(request.speed) && !std::isnan(requestHop) && std::abs(request.speed * unitHop - requestHop) > 1.)
		instrumentation.log("specifyGrain: speed=%f implies hop of %f/%d but position has advanced by %f/%d since previous grain", request.speed, request.speed * unitHop, sampleRates.input, requestHop, sampleRates.input);

	if (std::isnan(requestHop) || request.reset)
		requestHop = request.speed * unitHop;

	analysis.hopIdeal = requestHop * resampleOperations.input.ratio;

	continuous = !request.reset && !std::isnan(previous.request.position);
	if (continuous)
	{
		analysis.positionError = previous.analysis.positionError - analysis.hopIdeal;
		analysis.hop = std::round(-analysis.positionError);
		analysis.positionError += analysis.hop;
	}
	else
	{
		analysis.hop = std::round(analysis.hopIdeal);
		analysis.positionError = std::round(request.position) - request.position;
	}

	analysis.speed = analysis.hopIdeal / (1 << log2SynthesisHop);

	{
		passthrough = std::abs(analysis.speed) == 1. ? int(analysis.speed) : 0;
		if (continuous && passthrough != previous.passthrough)
			passthrough = 0;
	}

	log2TransformLength = log2SynthesisHop + 3;

	{
		int halfInputFrameCount = 1 << log2TransformLength >> 1;
		if (resampleOperations.input.ratio != 1.f)
			halfInputFrameCount = int(std::ceil(halfInputFrameCount / resampleOperations.input.ratio)) + 2;

		inputChunk.begin = -halfInputFrameCount;
		inputChunk.end = +halfInputFrameCount;

		if (std::isnan(request.position))
			return InputChunk{};

		const int offset = int(std::round(request.position - bufferStartPosition));
		inputChunk.begin += offset;
		inputChunk.end += offset;
		inputPosition = bufferStartPosition + offset;
		return inputChunk;
	}
}

void Grain::overlapCheck(Eigen::Ref<Eigen::ArrayXXf> input, int muteFrameCountHead, int muteFrameCountTail, const Grain &previous, Internal::Instrumentation &instrumentation)
{
	const auto frameCount = inputChunk.end - inputChunk.begin;
	const auto activeRows = frameCount - muteFrameCountHead - muteFrameCountTail;

#ifdef EIGEN_RUNTIME_NO_MALLOC
	Eigen::internal::set_is_malloc_allowed(true);
#endif
	inputCopy.resize(frameCount, input.cols());
#ifdef EIGEN_RUNTIME_NO_MALLOC
	Eigen::internal::set_is_malloc_allowed(false);
#endif

	inputCopy.topRows(muteFrameCountHead).setZero();
	inputCopy.middleRows(muteFrameCountHead, activeRows) = input.middleRows(muteFrameCountHead, activeRows);
	inputCopy.bottomRows(muteFrameCountTail).setZero();

	if (!inputCopy.isFinite().all())
		instrumentation.log("BAD INPUT: input audio is not all finite samples");

	const auto overlapStart = std::max(inputChunk.begin, previous.inputChunk.begin);
	const auto overlapEnd = std::min(inputChunk.end, previous.inputChunk.end);
	const auto overlapFrames = overlapEnd - overlapStart;

	if (overlapFrames > 0 && previous.inputCopy.rows() > 0)
	{
		const auto overlapCurrent = inputCopy.middleRows(overlapStart - inputChunk.begin, overlapFrames);
		const auto overlapPrevious = previous.inputCopy.middleRows(overlapStart - previous.inputChunk.begin, overlapFrames);

		if (!(overlapCurrent == overlapPrevious).all())
		{
			instrumentation.log("UNEXPECTED INPUT: the %s %d frames of this grain's input audio chunk are different to the %s %d frames of the previous grain's audio audio input chunk",
				overlapStart == inputChunk.begin ? "first" : "last",
				overlapFrames,
				overlapStart == inputChunk.begin ? "last" : "first",
				overlapFrames);
		}
	}
}

Eigen::Ref<Eigen::ArrayXXf> Grain::resampleInput(Eigen::Ref<Eigen::ArrayXXf> input, int log2WindowLength, int &muteFrameCountHead, int &muteFrameCountTail, Resample::Internal &resampled)
{
	if (resampleOperations.input.function)
	{
		BUNGEE_ASSERT1(input.rows() % 2 == 0);

		resampled.offset = inputPosition - request.position - input.rows() / 2;
		resampled.offset *= resampleOperations.input.ratio;
		resampled.offset += 1 << (log2WindowLength - 1);
		resampled.offset -= analysis.positionError;

		Resample::External external(input, muteFrameCountHead, muteFrameCountTail);
		resampleOperations.input.function(resampled, external, resampleOperations.input.ratio, resampleOperations.input.ratio, false);

		muteFrameCountHead = muteFrameCountTail = 0;

		return resampled.unpadded().topRows(resampled.frameCount);
	}
	else
	{
		return input;
	}
}

} // namespace Bungee
