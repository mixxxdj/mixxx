// Copyright (C) 2020-2026 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#include "Output.h"
#include "Grains.h"
#include "Window.h"

namespace Bungee {

Output::Output(Fourier::Transforms &transforms, int log2SynthesisHop, int channelCount, int maxOutputChunkSize, float windowGain, std::initializer_list<float> windowCoefficients) :
	synthesisWindow{Window::fromFrequencyDomainCoefficients(transforms, log2SynthesisHop + 2, windowGain, windowCoefficients)},
	inverseTransformed(8 << log2SynthesisHop, channelCount),
	bufferResampled(maxOutputChunkSize, channelCount),
	lappedSynthesisBuffer(1 << (log2SynthesisHop + 3), channelCount)
{
	transforms.prepareInverse(log2SynthesisHop + 3);
	lappedSynthesisBuffer.array.setZero();
	lappedSynthesisBuffer.frameCount = 1 << log2SynthesisHop;
}

void Output::applySynthesisWindow(int log2SynthesisHop, Grains &grains, const Eigen::Ref<const Eigen::ArrayXf> &window)
{
	BUNGEE_ASSERT1(lappedSynthesisBuffer.frameCount == window.rows() / 4);

	lappedSynthesisBuffer.array.topRows(Bungee::Resample::Internal::padding) = lappedSynthesisBuffer.array.middleRows(window.rows() / 4, Bungee::Resample::Internal::padding);

	const auto quadrantSize = (int)window.rows() / 4;
	const auto hopsPerTransform = 1 << (grains[0].log2TransformLength - log2SynthesisHop);

	if (grains[0].valid())
	{
		for (int i = 0; i < 4; ++i)
		{
			auto windowSegment = window.segment(quadrantSize * (i ^ 2), quadrantSize);

			auto j = (i + hopsPerTransform - 2) % hopsPerTransform;
			auto inputSegment = inverseTransformed.middleRows(quadrantSize * j, quadrantSize);

			if (i < 3)
				lappedSynthesisBuffer.unpadded().middleRows(i * quadrantSize, quadrantSize) = inputSegment.colwise() * windowSegment + lappedSynthesisBuffer.unpadded().middleRows((i + 1) * quadrantSize, quadrantSize);
			else
				lappedSynthesisBuffer.unpadded().middleRows(i * quadrantSize, quadrantSize) = inputSegment.colwise() * windowSegment;
		}
	}
	else
	{
		lappedSynthesisBuffer.unpadded().topRows(3 * lappedSynthesisBuffer.frameCount) = lappedSynthesisBuffer.unpadded().middleRows(lappedSynthesisBuffer.frameCount, 3 * lappedSynthesisBuffer.frameCount);
		lappedSynthesisBuffer.unpadded().middleRows(3 * lappedSynthesisBuffer.frameCount, lappedSynthesisBuffer.frameCount).setZero();
	}
}

inline auto makeOutputChunk(Eigen::Ref<Eigen::ArrayXXf> ref)
{
	OutputChunk outputChunk{};
	outputChunk.data = ref.data();
	outputChunk.frameCount = (int)ref.rows();
	outputChunk.channelStride = ref.stride();
	return outputChunk;
}

OutputChunk Output::resample(Resample::Operation resampleOperationBegin, Resample::Operation resampleOperationEnd)
{
	const auto resampleFunction = resampleOperationBegin.function ? resampleOperationBegin.function : resampleOperationEnd.function;
	if (resampleFunction)
	{
		Resample::External external(bufferResampled, 0, 0);
		resampleFunction(lappedSynthesisBuffer, external, resampleOperationBegin.ratio, resampleOperationEnd.ratio, !resampleOperationEnd.function);
		return makeOutputChunk(bufferResampled.topRows(external.activeFrameCount));
	}
	else
	{
		return makeOutputChunk(lappedSynthesisBuffer.unpadded().topRows(lappedSynthesisBuffer.frameCount));
	}
}

} // namespace Bungee
