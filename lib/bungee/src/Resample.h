// Copyright (C) 2020-2026 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "Assert.h"

#include "bungee/Bungee.h"
#include "bungee/Modes.h"

#include <Eigen/Core>

namespace Bungee::Resample {

// To resample from external buffer at input sample rate to internal buffer at (Fourier transformed) sample rate
class Input;

// To resample from internal buffer at (Fourier transformed) sample rate to external buffer at output sample rate
class Output;

// The internal buffer (as used by the phase vocoder's input and output)
struct Internal
{
	static constexpr auto align = std::max<int>(EIGEN_DEFAULT_ALIGN_BYTES / sizeof(float), 1);
	static constexpr auto padding = (32 + align - 1) / align * align;

	Eigen::ArrayXXf array;
	int frameCount{};
	double offset{};

	Internal(int maxFrameCount, int channelCount) :
		array(padding + maxFrameCount + padding, channelCount)
	{
	}

	inline Eigen::Ref<Eigen::ArrayXXf> unpadded()
	{
		return array.middleRows(padding, array.rows() - 2 * padding);
	}
};

// The external buffer: contains input or output samples.
struct External
{
	External(Eigen::Ref<Eigen::ArrayXXf> ref, ptrdiff_t muteHead, ptrdiff_t muteTail) :
		ref(ref),
		unmutedBegin(muteHead),
		unmutedEnd(ref.rows() - muteTail),
		activeFrameCount(ref.rows())
	{
		check();
	}

	inline void check()
	{
		BUNGEE_ASSERT1(0 <= unmutedBegin);
		BUNGEE_ASSERT1(unmutedBegin <= unmutedEnd);
		BUNGEE_ASSERT1(unmutedEnd <= activeFrameCount);
		BUNGEE_ASSERT1(activeFrameCount <= ref.rows());
	}

	Eigen::Ref<Eigen::ArrayXXf> ref;
	ptrdiff_t unmutedBegin;
	ptrdiff_t unmutedEnd;
	ptrdiff_t activeFrameCount;
};

// The core behaviour of the interpolation
template <class Mode, bool first>
static inline void filterTap(float &__restrict internal, float &__restrict external, float coefficient, float gain)
{
	if constexpr (std::is_same_v<Mode, Input>)
		internal += external * (coefficient * gain);
	else if constexpr (first)
		external = internal * coefficient;
	else
		external += internal * coefficient;
}

struct Nearest
{
	template <class Mode>
	static inline void step(float x, size_t channelCount, float *internal, ptrdiff_t internalStride, float *external, ptrdiff_t externalStride, float gain)
	{
		BUNGEE_ASSERT2(x >= 0);
		const auto integer = intptr_t(x + 0.5f);
		BUNGEE_ASSERT2(integer < internalStride);

		for (size_t c = 0; c < channelCount; ++c)
			filterTap<Mode, true>(internal[integer + c * internalStride], external[c * externalStride], 1.f, gain);
	}
};

struct Bilinear
{
	template <class Mode>
	static inline void step(float x, size_t channelCount, float *internal, ptrdiff_t internalStride, float *external, ptrdiff_t externalStride, float gain)
	{
		const Assert::FloatingPointExceptions floatingPointExceptions(FE_INEXACT);
		BUNGEE_ASSERT2(x >= 0);
		const auto integer = intptr_t(x);
		const float fraction = x - integer;
		BUNGEE_ASSERT2(0 <= fraction && fraction <= 1);
		BUNGEE_ASSERT2(integer + 1 < internalStride);

		for (int c = 0; c < channelCount; ++c)
		{
			filterTap<Mode, true>(internal[integer + 1 + c * internalStride], external[c * externalStride], fraction, gain);
			filterTap<Mode, false>(internal[integer + 0 + c * internalStride], external[c * externalStride], 1.f - fraction, gain);
		}
	}
};

template <bool ratioIsConstant>
struct RatioState;

template <>
struct RatioState<false>
{
	const double ratioGradient;
	double ratio;
	double x;

	inline RatioState(double offset, double ratioBegin, double ratioGradient) :
		ratioGradient(ratioGradient),
		ratio(ratioBegin + 0.5 * ratioGradient),
		x(offset)
	{
	}

	inline void step()
	{
		x += ratio;
		ratio += ratioGradient;
	}
};

template <>
struct RatioState<true>
{
	const double ratio;
	double x;

	inline RatioState(double offset, double ratio, double ratioGradient) :
		ratio(ratio),
		x(offset)
	{
		BUNGEE_ASSERT1(ratioGradient == 0);
	}

	inline void step()
	{
		x += ratio;
	}
};

template <class Interpolation, class Mode>
struct Loop
{
	template <bool ratioIsConstant>
	static __attribute__((noinline)) void run(RatioState<ratioIsConstant> &ratioState, Internal &internal, External external) // const & ext
	{
		const Assert::FloatingPointExceptions floatingPointExceptions(FE_INEXACT | FE_UNDERFLOW);

		if constexpr (std::is_same_v<Mode, Input>)
			internal.array.setZero();

		BUNGEE_ASSERT1(internal.array.cols() == external.ref.cols());

		for (size_t row = 0; row < external.activeFrameCount; ++row)
		{
			BUNGEE_ASSERT2(ratioState.x >= 0.);
			BUNGEE_ASSERT2(ratioState.ratio > 0.);

			if (row >= external.unmutedBegin && row < external.unmutedEnd)
				Interpolation::template step<Mode>(ratioState.x, external.ref.cols(), internal.array.data(), internal.array.colStride(), external.ref.data() + row, external.ref.colStride(), ratioState.ratio);

			ratioState.step();
		}
	}
};

template <class Interpolation, class Mode, bool ratioIsConstant>
inline void resampleSpecial(Internal &internal, External external, double ratioBegin, double ratioEnd)
{
	const auto ratioGradient = (ratioEnd - ratioBegin) / external.activeFrameCount;
	RatioState<ratioIsConstant> ratioState(Internal::padding + internal.offset, ratioBegin, ratioGradient);

	Loop<Interpolation, Mode>::run(ratioState, internal, external);

	internal.offset = ratioState.x - Internal::padding;
}

template <class Interpolation, class Mode>
void resample(Internal &internal, External &external, double ratioBegin, double ratioEnd, bool alignEnd)
{
	const auto idealFrameCount = (ptrdiff_t)std::round(2 * (internal.frameCount - internal.offset) / (ratioBegin + ratioEnd));

	const bool truncate = idealFrameCount > external.ref.rows();
	BUNGEE_ASSERT1(!truncate);
	if (!truncate)
		external.activeFrameCount = idealFrameCount;

	if (external.activeFrameCount > 0)
	{
		if (alignEnd)
		{
			const auto meanRatio = (internal.frameCount - internal.offset) / external.activeFrameCount;
			ratioEnd = 2 * meanRatio - ratioBegin;
			BUNGEE_ASSERT1(ratioEnd > 0);
		}

		external.unmutedBegin = std::clamp<ptrdiff_t>(external.unmutedBegin, 0, external.activeFrameCount);
		external.unmutedEnd = std::clamp<ptrdiff_t>(external.unmutedEnd, external.unmutedBegin, external.activeFrameCount);

		if (ratioBegin == ratioEnd)
			resampleSpecial<Interpolation, Mode, true>(internal, external, ratioBegin, ratioEnd);
		else
			resampleSpecial<Interpolation, Mode, false>(internal, external, ratioBegin, ratioEnd);

		internal.offset -= internal.frameCount;

		if (std::abs(internal.offset) > (alignEnd ? 1e-2 : ratioEnd * 1.1))
		{
			BUNGEE_ASSERT1(!"Resample::resample: landed badly");
			internal.offset = 0.;
		}
	}
}

typedef decltype(&resample<Nearest, Output>) Function;

struct Operation
{
	Function function{};
	double ratio{1.};
};

struct Operations
{
	Operation input, output;

	double setup(const SampleRates &sampleRates, double pitch, ResampleMode resampleMode)
	{
		const double resampleRatio = pitch * sampleRates.input / sampleRates.output;
		input.ratio = 1. / resampleRatio;
		output.ratio = resampleRatio;

		input.function = &resample<Bilinear, Input>;
		output.function = &resample<Bilinear, Output>;

		if (resampleMode == resampleMode_forceOut)
			input.function = nullptr;
		else if (resampleMode == resampleMode_forceIn)
			output.function = nullptr;
		else if (resampleRatio == 1.)
			input.function = output.function = nullptr;
		else if (resampleMode == resampleMode_autoIn)
			output.function = nullptr;
		else if (resampleMode == resampleMode_autoOut)
			input.function = nullptr;
		else if (resampleMode == resampleMode_autoInOut && resampleRatio > 1.)
			output.function = nullptr;
		else if (resampleMode == resampleMode_autoInOut && resampleRatio < 1.)
			input.function = nullptr;
		else
		{
			BUNGEE_ASSERT1(false);
			input.function = nullptr;
		}

		if (!input.function)
			input.ratio = 1.;

		if (output.function)
			return ((double)sampleRates.input / sampleRates.output) / output.ratio;
		else
		{
			output.ratio = 1.;
			return (double)sampleRates.input / sampleRates.output;
		}
	}
};

} // namespace Bungee::Resample
