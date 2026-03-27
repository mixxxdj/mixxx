#ifndef SIGNALSMITH_LINEAR_PLATFORM_FFT_PFFFTDOUBLE_H
#define SIGNALSMITH_LINEAR_PLATFORM_FFT_PFFFTDOUBLE_H

#if defined(__has_include) && !__has_include("pffft/pffft_double.h")
#	include "pffft_double.h"
#else
#	include "pffft/pffft_double.h"
#endif

#include <memory>
#include <cmath>
#include <complex>
#include <cassert>

namespace signalsmith { namespace linear {

template<>
struct Pow2FFT<double> {
	static constexpr bool prefersSplit = false;

	using Complex = std::complex<double>;

	Pow2FFT(size_t size=0) {
		resize(size);
	}
	~Pow2FFT() {
		resize(0); // frees everything
	}
	// Allow move, but not copy
	Pow2FFT(const Pow2FFT &other) = delete;
	Pow2FFT(Pow2FFT &&other) : _size(other._size), hasSetup(other.hasSetup), fftSetup(other.fftSetup), fallback(std::move(other.fallback)), work(other.work), tmpAligned(other.tmpAligned) {
		// Avoid double-free
		other.hasSetup = false;
		other.work = nullptr;
		other.tmpAligned = nullptr;
	}

	void resize(size_t size) {
		_size = size;
		fallback = nullptr;
		if (hasSetup) pffftd_destroy_setup(fftSetup);
		if (work) pffftd_aligned_free(work);
		work = nullptr;
		if (tmpAligned) pffftd_aligned_free(tmpAligned);
		tmpAligned = nullptr;

		// We use this for split-real, even if there's no PFFFT setup
		tmpAligned = (double *)pffftd_aligned_malloc(sizeof(double)*size*2);

		if (size < 16) {
			// PFFFT doesn't support smaller sizes
			hasSetup = false;
			fallback = std::unique_ptr<SimpleFFT<double>>{
				new SimpleFFT<double>(size)
			};
			return;
		}
		
		work = (double *)pffftd_aligned_malloc(sizeof(double)*size*2);
		fftSetup = pffftd_new_setup(int(size), PFFFT_COMPLEX);
		hasSetup = fftSetup;
	}

	void fft(const Complex* input, Complex* output) {
		if (fallback) return fallback->fft(input, output);
		fftInner(input, output, PFFFT_FORWARD);
	}
	void fft(const double *inR, const double *inI, double *outR, double *outI) {
		if (fallback) return fallback->fft(inR, inI, outR, outI);
		fftInner(inR, inI, outR, outI, PFFFT_FORWARD);
	}

	void ifft(const Complex* input, Complex* output) {
		if (fallback) return fallback->ifft(input, output);
		fftInner(input, output, PFFFT_BACKWARD);
	}
	void ifft(const double *inR, const double *inI, double *outR, double *outI) {
		if (fallback) return fallback->ifft(inR, inI, outR, outI);
		fftInner(inR, inI, outR, outI, PFFFT_BACKWARD);
	}

private:
	void fftInner(const Complex *input, Complex *output, pffft_direction_t direction) {
		// 32-byte alignment
		if (size_t(input)&0x1F) {
			// `tmpAligned` is always aligned, so copy into that
			std::memcpy(tmpAligned, input, sizeof(Complex)*_size);
			input = (const Complex *)tmpAligned;
		}
		if (size_t(output)&0x1F) {
			// Output to `tmpAligned` - might be in-place if input is unaligned, but that's fine
			pffftd_transform_ordered(fftSetup, (const double *)input, tmpAligned, work, direction);
			std::memcpy(output, tmpAligned, sizeof(Complex)*_size);
		} else {
			pffftd_transform_ordered(fftSetup, (const double *)input, (double *)output, work, direction);
		}
	}
	void fftInner(const double *inR, const double *inI, double *outR, double *outI, pffft_direction_t direction) {
		for (size_t i = 0; i < _size; ++i) {
			tmpAligned[2*i] = inR[i];
			tmpAligned[2*i + 1] = inI[i];
		}
		// PFFFT supports in-place transforms
		fftInner((const Complex *)tmpAligned, (Complex *)tmpAligned, direction);
		// Un-interleave
		for (size_t i = 0; i < _size; ++i) {
			outR[i] = tmpAligned[2*i];
			outI[i] = tmpAligned[2*i + 1];
		}
	}

	size_t _size = 0;
	bool hasSetup = false;
	PFFFTD_Setup *fftSetup = nullptr;
	std::unique_ptr<SimpleFFT<double>> fallback;
	double *work = nullptr, *tmpAligned = nullptr;
};

template<>
struct Pow2RealFFT<double> {
private:
	using FallbackFFT = SimpleRealFFT<double>; // this wraps the complex one
public:
	static constexpr bool prefersSplit = false;

	using Complex = std::complex<double>;

	Pow2RealFFT(size_t size=0) {
		resize(size);
	}
	~Pow2RealFFT() {
		resize(0);
	}
	// Allow move, but not copy
	Pow2RealFFT(const Pow2RealFFT &other) = delete;
	Pow2RealFFT(Pow2RealFFT &&other) : _size(other._size), hasSetup(other.hasSetup), fftSetup(other.fftSetup), fallback(std::move(other.fallback)), work(other.work), tmpAligned(other.tmpAligned) {
		// Avoid double-free
		other.hasSetup = false;
		other.work = nullptr;
		other.tmpAligned = nullptr;
	}

	void resize(size_t size) {
		_size = size;
		fallback = nullptr;
		if (hasSetup) pffftd_destroy_setup(fftSetup);
		if (work) pffftd_aligned_free(work);
		work = nullptr;
		if (tmpAligned) pffftd_aligned_free(tmpAligned);
		tmpAligned = nullptr;

		// We use this for split-real, even if there's no PFFFT setup
		tmpAligned = (double *)pffftd_aligned_malloc(sizeof(double)*size*2);

		// TODO: just go for it, and check for success before allocating `work`
		if (size < 32) {
			// PFFFT doesn't support smaller sizes
			hasSetup = false;
			fallback = std::unique_ptr<FallbackFFT>{
				new FallbackFFT(size)
			};
			return;
		}
		
		work = (double *)pffftd_aligned_malloc(sizeof(double)*size);
		fftSetup = pffftd_new_setup(int(size), PFFFT_REAL);
		hasSetup = fftSetup;
	}

	void fft(const double *input, Complex *output) {
		if (fallback) return fallback->fft(input, output);
		fftInner(input, (double *)output, PFFFT_FORWARD);
	}
	void fft(const double *inR, double *outR, double *outI) {
		if (fallback) return fallback->fft(inR, outR, outI);
		fftInner(inR, tmpAligned, PFFFT_FORWARD);
		for (size_t i = 0; i < _size/2; ++i) {
			outR[i] = tmpAligned[2*i];
			outI[i] = tmpAligned[2*i + 1];
		}
	}

	void ifft(const Complex *input, double *output) {
		if (fallback) return fallback->ifft(input, output);
		fftInner((const double *)input, output, PFFFT_BACKWARD);
	}
	void ifft(const double *inR, const double *inI, double *outR) {
		if (fallback) return fallback->ifft(inR, inI, outR);
		for (size_t i = 0; i < _size/2; ++i) {
			tmpAligned[2*i] = inR[i];
			tmpAligned[2*i + 1] = inI[i];
		}
		fftInner(tmpAligned, outR, PFFFT_BACKWARD);
	}

private:
	void fftInner(const double *input, double *output, pffft_direction_t direction) {
		// 32-byte alignment
		if (size_t(input)&0x1F) {
			// `tmpAligned` is always aligned, so copy into that
			std::memcpy(tmpAligned, input, sizeof(double)*_size);
			input = tmpAligned;
		}
		if (size_t(output)&0x1F) {
			// Output to `tmpAligned` - might be in-place if input is unaligned, but that's fine
			pffftd_transform_ordered(fftSetup, input, tmpAligned, work, direction);
			std::memcpy(output, tmpAligned, sizeof(double)*_size);
		} else {
			pffftd_transform_ordered(fftSetup, input, output, work, direction);
		}
	}

	size_t _size = 0;
	bool hasSetup = false;
	PFFFTD_Setup *fftSetup = nullptr;
	std::unique_ptr<FallbackFFT> fallback;
	double *work = nullptr, *tmpAligned = nullptr;
};

}} // namespace
#endif // include guard
