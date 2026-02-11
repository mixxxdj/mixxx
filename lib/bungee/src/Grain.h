// Copyright (C) 2020-2026 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "Assert.h"
#include "Fourier.h"
#include "Instrumentation.h"
#include "Output.h"
#include "Partials.h"
#include "Phase.h"
#include "Stretch.h"
#include "Window.h"

#include "bungee/Bungee.h"

#include <Eigen/Core>

#include <array>
#include <complex>
#include <memory>
#include <numbers>

namespace Bungee {

struct Grain
{
	struct Analysis
	{
		double positionError;
		double hopIdeal;
		double speed;
		int hop; // rounded
	};

	int log2TransformLength;
	int channelCount;
	Request request;

	double requestHop{};
	bool continuous{};
	int passthrough{};
	int validBinCount{};
	int muteFrameCountHead{};
	int muteFrameCountTail{};

	Resample::Operations resampleOperations{};

	double inputPosition;
	InputChunk inputChunk{};
	Analysis analysis{};

	Eigen::ArrayX<Phase::Type> phase;
	Eigen::ArrayXf energy;
	Eigen::ArrayX<Phase::Type> rotation;
	std::vector<Partials::Partial> partials;
	Eigen::ArrayXXf inputCopy;

	Grain(int log2SynthesisHop, int channelCount);

	InputChunk specify(const Request &request, Grain &previous, SampleRates sampleRates, int log2SynthesisHop, double bufferStartPosition, Internal::Instrumentation &instrumentation);

	bool reverse() const
	{
		return analysis.hop < 0;
	}

	bool valid() const
	{
		return !std::isnan(request.position);
	}

	void applyEnvelope();

	auto inputChunkMap(const float *data, std::ptrdiff_t stride, int &muteFrameCountHead, int &muteFrameCountTail, const Grain &previous, Internal::Instrumentation &instrumentation)
	{
		const auto frameCount = inputChunk.end - inputChunk.begin;

		if (!data)
		{
			muteFrameCountHead = frameCount;
			muteFrameCountTail = 0;
		}

		typedef Eigen::OuterStride<Eigen::Dynamic> Stride;
		typedef Eigen::Map<Eigen::ArrayXXf, 0, Stride> Map;

		muteFrameCountHead = std::clamp<int>(muteFrameCountHead, 0, frameCount);
		muteFrameCountTail = std::clamp<int>(muteFrameCountTail, 0, frameCount);

		Map m((float *)data, frameCount, channelCount, Stride(stride));
		BUNGEE_ASSERT2(!m.middleRows(muteFrameCountHead, m.rows() - muteFrameCountHead - muteFrameCountTail).hasNaN());

		if (instrumentation.enabled || Bungee::Assert::level)
			overlapCheck(m, muteFrameCountHead, muteFrameCountTail, previous, instrumentation);

		BUNGEE_ASSERT1(m.rows() % 2 == 0);
		return m;
	}

	void overlapCheck(Eigen::Ref<Eigen::ArrayXXf> input, int muteFrameCountHead, int muteFrameCountTail, const Grain &previous, Internal::Instrumentation &instrumentation);

	Eigen::Ref<Eigen::ArrayXXf> resampleInput(Eigen::Ref<Eigen::ArrayXXf> input, int log2WindowLength, int &muteFrameCountHead, int &muteFrameCountTail, Resample::Internal &resampled);
};

} // namespace Bungee
