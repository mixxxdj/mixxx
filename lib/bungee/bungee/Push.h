// Copyright (C) 2020-2026 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <cassert>
#include <vector>

#include <bungee/Bungee.h>

namespace Bungee::Push {

// Bungee::Push::InputBuffer is an optional component that assists users of Bungee::Stretcher
// in applications where function calls "push" audio downstream.
//
// There are two fundamental philosophies for how function calls propagate audio data
// when processing streamed audio data:
// "Pull": processing nodes in the pipeline call upstream to request chunks of audio data
// "Push": processing nodes call downstream to deliver chunks of audio data
//
// Bungee's native API is pull-based. It has a number of advantages:
// * it permits ultimate flexibility in frozen or reverse play,
// * it simplifies real-time, low-latency operation,
// * it minimises the need for buffers and copying audio data,
// * it allows clear association of metadata and timestamps with audio chunks, and,
// * it allows granular, low latency control of speed and pitch.
//
// That said, many existing audio pipeline designs use a push philosophy. This adapter is
// provided to assist developers integrate Bungee into an application where function calls
// are used to push audio downstream ("push" operation). In such situations, Bungee
// cannot reverse through the audio stream, it can only progress forwards although
// speed and pitch controls will work as usual.
//
// This adapter buffers audio in order to provide the overlapping input grains
// required by Bungee::Stretcher. Example usage may be found in ../cmd/main.cpp.
//
struct InputBuffer
{
	std::vector<float> vector;
	int maxInputFrameCount;
	int begin = 0;
	int end = -1;
	int endRequired = 0;

	InputBuffer(int maxInputFrameCount, int channelCount) :
		vector(maxInputFrameCount * channelCount),
		maxInputFrameCount(maxInputFrameCount)
	{
	}

	void grain(const InputChunk &inputChunk)
	{
		const bool firstCall = end - begin < 0;
		if (firstCall)
		{
			begin = inputChunk.begin;
			end = 0;
		}

		const int overlap = end - inputChunk.begin;
		if (overlap <= 0)
		{
			begin = end = inputChunk.begin;
		}
		else
		{
			const int offset = inputChunk.begin - begin;

			// loop over channels, move lapped segment to start of buffer
			for (int x = 0; x < (int)vector.size(); x += stride())
				std::move(
					&vector[x + offset],
					&vector[x + offset + overlap],
					&vector[x]);

			begin = inputChunk.begin;
		}
		endRequired = inputChunk.end;

		assert(inputFrameCountRequired() <= inputFrameCountMax());
		assert(inputFrameCountMax() >= 0);
	}

	void deliver(int frameCount)
	{
		assert(frameCount >= 0);
		assert(frameCount <= inputFrameCountMax());
		end += frameCount;
	}

	float *inputData()
	{
		return &vector[end - begin];
	}

	int inputFrameCountRequired() const
	{
		return endRequired - end;
	}

	int inputFrameCountMax() const
	{
		return stride() - (end - begin);
	}

	const float *outputData() const
	{
		return vector.data();
	}

	int stride() const
	{
		return maxInputFrameCount;
	}
};

} // namespace Bungee::Push
