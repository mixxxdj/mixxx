// Copyright (C) 2020-2026 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#include "Fourier.h"
#include "Assert.h"

#include "../submodules/pffft/pffft.h"
namespace Bungee::Fourier {

#ifndef BUNGEE_USE_PFFFT
#	define BUNGEE_USE_PFFFT 1
#endif

#if BUNGEE_USE_PFFFT

namespace {

struct Pffft
{
	struct Kernel
	{
		void *p;

		Kernel(int log2TransformLength);
		~Kernel();

		void forward(int log2TransformLength, float *t, std::complex<float> *f) const;
		void inverse(int log2TransformLength, float *t, std::complex<float> *f) const;
	};

	typedef Kernel Forward;
	typedef Kernel Inverse;
};

Pffft::Kernel::Kernel(int log2TransformLength) :
	p(pffft_new_setup(1 << log2TransformLength, PFFFT_REAL))
{
}

Pffft::Kernel::~Kernel()
{
	pffft_destroy_setup((PFFFT_Setup *)p);
}

void Pffft::Kernel::forward(int log2TransformLength, float *t, std::complex<float> *f) const
{
	pffft_transform_ordered((PFFFT_Setup *)p, t, (float *)f, nullptr, PFFFT_FORWARD);
	const auto transformLength = 1 << log2TransformLength;
	f[transformLength / 2] = f[0].imag();
	f[0].imag(0.f);
}

void Pffft::Kernel::inverse(int log2TransformLength, float *t, std::complex<float> *f) const
{
	const auto transformLength = 1 << log2TransformLength;
	const auto backup = f[0].imag();
	f[0].imag(f[transformLength / 2].real());
	pffft_transform_ordered((PFFFT_Setup *)p, (float *)f, t, nullptr, PFFFT_BACKWARD);
	f[0].imag(backup);
}

} // namespace

typedef Cache<Pffft, 16> Implementation;

Transforms::Transforms()
{
	p = new Implementation;
}

Transforms::~Transforms()
{
	delete reinterpret_cast<Implementation *>(p);
}

void Transforms::prepareForward(int log2TransformLength)
{
	reinterpret_cast<Implementation *>(p)->prepareForward(log2TransformLength);
}

void Transforms::prepareInverse(int log2TransformLength)
{
	reinterpret_cast<Implementation *>(p)->prepareInverse(log2TransformLength);
}

void Transforms::forward(int log2TransformLength, const Eigen::Ref<const Eigen::ArrayXXf> &t, Eigen::Ref<Eigen::ArrayXXcf> f)
{
	reinterpret_cast<Implementation *>(p)->forward(log2TransformLength, t, f);
}

void Transforms::inverse(int log2TransformLength, Eigen::Ref<Eigen::ArrayXXf> t, const Eigen::Ref<const Eigen::ArrayXXcf> &f)
{
	reinterpret_cast<Implementation *>(p)->inverse(log2TransformLength, t, f);
}

#endif

} // namespace Bungee::Fourier
