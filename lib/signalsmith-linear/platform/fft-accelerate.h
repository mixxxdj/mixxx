// If possible, only include vecLib, since JUCE has conflicts with vImage
#if defined(__has_include) && __has_include(<vecLib/vecLib.h>)
#	include <vecLib/vecLib.h>
#else
#	include <Accelerate/Accelerate.h>
#endif

namespace signalsmith { namespace linear {

namespace _impl {
	template<>
	inline void complexMul<float>(std::complex<float> *a, const std::complex<float> *b, const std::complex<float> *c, size_t size) {
		DSPSplitComplex aSplit = {(float *)a, (float *)a + 1};
		DSPSplitComplex bSplit = {(float *)b, (float *)b + 1};
		DSPSplitComplex cSplit = {(float *)c, (float *)c + 1};
		vDSP_zvmul(&cSplit, 2, &bSplit, 2, &aSplit, 2, size, 1);
	}
	template<>
	inline void complexMulConj<float>(std::complex<float> *a, const std::complex<float> *b, const std::complex<float> *c, size_t size) {
		DSPSplitComplex aSplit = {(float *)a, (float *)a + 1};
		DSPSplitComplex bSplit = {(float *)b, (float *)b + 1};
		DSPSplitComplex cSplit = {(float *)c, (float *)c + 1};
		vDSP_zvmul(&cSplit, 2, &bSplit, 2, &aSplit, 2, size, -1);
	}
	template<>
	inline void complexMul<float>(float *ar, float *ai, const float *br, const float *bi, const float *cr, const float *ci, size_t size) {
		DSPSplitComplex aSplit = {ar, ai};
		DSPSplitComplex bSplit = {(float *)br, (float *)bi};
		DSPSplitComplex cSplit = {(float *)cr, (float *)ci};
		vDSP_zvmul(&cSplit, 1, &bSplit, 1, &aSplit, 1, size, 1);
	}
	template<>
	inline void complexMulConj<float>(float *ar, float *ai, const float *br, const float *bi, const float *cr, const float *ci, size_t size) {
		DSPSplitComplex aSplit = {ar, ai};
		DSPSplitComplex bSplit = {(float *)br, (float *)bi};
		DSPSplitComplex cSplit = {(float *)cr, (float *)ci};
		vDSP_zvmul(&cSplit, 1, &bSplit, 1, &aSplit, 1, size, -1);
	}
	
	// doubles
	template<>
	inline void complexMul<double>(std::complex<double> *a, const std::complex<double> *b, const std::complex<double> *c, size_t size) {
		DSPDoubleSplitComplex aSplit = {(double *)a, (double *)a + 1};
		DSPDoubleSplitComplex bSplit = {(double *)b, (double *)b + 1};
		DSPDoubleSplitComplex cSplit = {(double *)c, (double *)c + 1};
		vDSP_zvmulD(&cSplit, 2, &bSplit, 2, &aSplit, 2, size, 1);
	}
	template<>
	inline void complexMulConj<double>(std::complex<double> *a, const std::complex<double> *b, const std::complex<double> *c, size_t size) {
		DSPDoubleSplitComplex aSplit = {(double *)a, (double *)a + 1};
		DSPDoubleSplitComplex bSplit = {(double *)b, (double *)b + 1};
		DSPDoubleSplitComplex cSplit = {(double *)c, (double *)c + 1};
		vDSP_zvmulD(&cSplit, 2, &bSplit, 2, &aSplit, 2, size, -1);
	}
	template<>
	inline void complexMul<double>(double *ar, double *ai, const double *br, const double *bi, const double *cr, const double *ci, size_t size) {
		DSPDoubleSplitComplex aSplit = {ar, ai};
		DSPDoubleSplitComplex bSplit = {(double *)br, (double *)bi};
		DSPDoubleSplitComplex cSplit = {(double *)cr, (double *)ci};
		vDSP_zvmulD(&cSplit, 1, &bSplit, 1, &aSplit, 1, size, 1);
	}
	template<>
	inline void complexMulConj<double>(double *ar, double *ai, const double *br, const double *bi, const double *cr, const double *ci, size_t size) {
		DSPDoubleSplitComplex aSplit = {ar, ai};
		DSPDoubleSplitComplex bSplit = {(double *)br, (double *)bi};
		DSPDoubleSplitComplex cSplit = {(double *)cr, (double *)ci};
		vDSP_zvmulD(&cSplit, 1, &bSplit, 1, &aSplit, 1, size, -1);
	}
}

template<>
struct Pow2FFT<float> {
	static constexpr bool prefersSplit = true;

	using Complex = std::complex<float>;

	Pow2FFT(size_t size = 0) {
		resize(size);
	}
	~Pow2FFT() {
		if (hasSetup) vDSP_destroy_fftsetup(fftSetup);
	}
	// Allow move, but not copy
	Pow2FFT(const Pow2FFT &other) = delete;
	Pow2FFT(Pow2FFT &&other) : _size(other._size), hasSetup(other.hasSetup), fftSetup(other.fftSetup), log2(other.log2), splitReal(std::move(other.splitReal)), splitImag(std::move(other.splitImag)) {
		// Avoid double-free
		other.hasSetup = false;
	}

	void resize(size_t size) {
		_size = size;
		if (hasSetup) vDSP_destroy_fftsetup(fftSetup);
		if (!size) {
			hasSetup = false;
			return;
		}

		splitReal.resize(size);
		splitImag.resize(size);
		log2 = std::round(std::log2(size));
		fftSetup = vDSP_create_fftsetup(log2, FFT_RADIX2);
		hasSetup = fftSetup;
	}

	void fft(const Complex* input, Complex* output) {
		DSPSplitComplex splitComplex{ splitReal.data(), splitImag.data() };
		vDSP_ctoz((DSPComplex*)input, 2, &splitComplex, 1, _size);
		vDSP_fft_zip(fftSetup, &splitComplex, 1, log2, kFFTDirection_Forward);
		vDSP_ztoc(&splitComplex, 1, (DSPComplex*)output, 2, _size);
	}
	void fft(const float *inR, const float *inI, float *outR, float *outI) {
		DSPSplitComplex inSplit{(float *)inR, (float *)inI};
		DSPSplitComplex outSplit{outR, outI};
		vDSP_fft_zop(fftSetup, &inSplit, 1, &outSplit, 1, log2, kFFTDirection_Forward);
	}

	void ifft(const Complex* input, Complex* output) {
		DSPSplitComplex splitComplex{ splitReal.data(), splitImag.data() };
		vDSP_ctoz((DSPComplex*)input, 2, &splitComplex, 1, _size);
		vDSP_fft_zip(fftSetup, &splitComplex, 1, log2, kFFTDirection_Inverse);
		vDSP_ztoc(&splitComplex, 1, (DSPComplex*)output, 2, _size);
	}
	void ifft(const float *inR, const float *inI, float *outR, float *outI) {
		DSPSplitComplex inSplit{(float *)inR, (float *)inI};
		DSPSplitComplex outSplit{outR, outI};
		vDSP_fft_zop(fftSetup, &inSplit, 1, &outSplit, 1, log2, kFFTDirection_Inverse);
	}

private:
	size_t _size = 0;
	bool hasSetup = false;
	FFTSetup fftSetup;
	int log2 = 0;
	std::vector<float> splitReal, splitImag;
};

template<>
struct Pow2RealFFT<float> {
	static constexpr bool prefersSplit = true;

	using Complex = std::complex<float>;

	Pow2RealFFT(size_t size = 0) {
		resize(size);
	}
	~Pow2RealFFT() {
		if (hasSetup) vDSP_destroy_fftsetup(fftSetup);
	}
	// Allow move, but not copy
	Pow2RealFFT(const Pow2RealFFT &other) = delete;
	Pow2RealFFT(Pow2RealFFT &&other) : _size(other._size), hasSetup(other.hasSetup), fftSetup(other.fftSetup), log2(other.log2), splitReal(std::move(other.splitReal)), splitImag(std::move(other.splitImag)) {
		// Avoid double-free
		other.hasSetup = false;
	}

	void resize(size_t size) {
		_size = size;
		if (hasSetup) vDSP_destroy_fftsetup(fftSetup);
		if (!size) {
			hasSetup = false;
			return;
		}

		splitReal.resize(size);
		splitImag.resize(size);
		log2 = std::log2(size);
		fftSetup = vDSP_create_fftsetup(log2, FFT_RADIX2);
		hasSetup = fftSetup;
	}

	void fft(const float* input, Complex* output) {
		float mul = 0.5f;
		vDSP_vsmul(input, 2, &mul, splitReal.data(), 1, _size/2);
		vDSP_vsmul(input + 1, 2, &mul, splitImag.data(), 1, _size/2);
		DSPSplitComplex tmpSplit{splitReal.data(), splitImag.data()};
		vDSP_fft_zrip(fftSetup, &tmpSplit, 1, log2, kFFTDirection_Forward);
		vDSP_ztoc(&tmpSplit, 1, (DSPComplex *)output, 2, _size/2);
	}
	void fft(const float *inR, float *outR, float *outI) {
		DSPSplitComplex outputSplit{outR, outI};
		float mul = 0.5f;
		vDSP_vsmul(inR, 2, &mul, outR, 1, _size/2);
		vDSP_vsmul(inR + 1, 2, &mul, outI, 1, _size/2);
		vDSP_fft_zrip(fftSetup, &outputSplit, 1, log2, kFFTDirection_Forward);
	}

	void ifft(const Complex * input, float * output) {
		DSPSplitComplex tmpSplit{splitReal.data(), splitImag.data()};
		vDSP_ctoz((DSPComplex*)input, 2, &tmpSplit, 1, _size/2);
		vDSP_fft_zrip(fftSetup, &tmpSplit, 1, log2, kFFTDirection_Inverse);
		DSPSplitComplex outputSplit{output, output + 1};
		vDSP_zvmov(&tmpSplit, 1, &outputSplit, 2, _size/2);
	}
	void ifft(const float *inR, const float *inI, float *outR) {
		DSPSplitComplex inputSplit{(float *)inR, (float *)inI};
		DSPSplitComplex tmpSplit{splitReal.data(), splitImag.data()};
		vDSP_fft_zrop(fftSetup, &inputSplit, 1, &tmpSplit, 1, log2, kFFTDirection_Inverse);
		DSPSplitComplex outputSplit{outR, outR + 1};
		// We can't use vDSP_ztoc without knowing the alignment
		vDSP_zvmov(&tmpSplit, 1, &outputSplit, 2, _size/2);
	}

private:
	size_t _size = 0;
	bool hasSetup = false;
	FFTSetup fftSetup;
	int log2 = 0;
	std::vector<float> splitReal, splitImag;
};

template<>
struct Pow2FFT<double> {
	static constexpr bool prefersSplit = true;

	using Complex = std::complex<double>;

	Pow2FFT(size_t size=0) {
		resize(size);
	}
	~Pow2FFT() {
		if (hasSetup) vDSP_destroy_fftsetupD(fftSetup);
	}

	void resize(size_t size) {
		_size = size;
		if (hasSetup) vDSP_destroy_fftsetupD(fftSetup);
		if (!size) {
			hasSetup = false;
			return;
		}

		log2 = std::round(std::log2(size));
		fftSetup = vDSP_create_fftsetupD(log2, FFT_RADIX2);
		hasSetup = fftSetup;

		splitReal.resize(size);
		splitImag.resize(size);
	}

	void fft(const Complex* input, Complex* output) {
		DSPDoubleSplitComplex splitComplex{ splitReal.data(), splitImag.data() };
		vDSP_ctozD((DSPDoubleComplex*)input, 2, &splitComplex, 1, _size);
		vDSP_fft_zipD(fftSetup, &splitComplex, 1, log2, kFFTDirection_Forward);
		vDSP_ztocD(&splitComplex, 1, (DSPDoubleComplex*)output, 2, _size);
	}
	void fft(const double *inR, const double *inI, double *outR, double *outI) {
		DSPDoubleSplitComplex inSplit{(double *)inR, (double *)inI};
		DSPDoubleSplitComplex outSplit{outR, outI};
		vDSP_fft_zopD(fftSetup, &inSplit, 1, &outSplit, 1, log2, kFFTDirection_Forward);
	}

	void ifft(const Complex* input, Complex* output) {
		DSPDoubleSplitComplex splitComplex{ splitReal.data(), splitImag.data() };
		vDSP_ctozD((DSPDoubleComplex*)input, 2, &splitComplex, 1, _size);
		vDSP_fft_zipD(fftSetup, &splitComplex, 1, log2, kFFTDirection_Inverse);
		vDSP_ztocD(&splitComplex, 1, (DSPDoubleComplex*)output, 2, _size);
	}
	void ifft(const double *inR, const double *inI, double *outR, double *outI) {
		DSPDoubleSplitComplex inSplit{(double *)inR, (double *)inI};
		DSPDoubleSplitComplex outSplit{outR, outI};
		vDSP_fft_zopD(fftSetup, &inSplit, 1, &outSplit, 1, log2, kFFTDirection_Inverse);
	}

private:
	size_t _size = 0;
	bool hasSetup = false;
	FFTSetupD fftSetup;
	int log2 = 0;
	std::vector<double> splitReal, splitImag;
};

template<>
struct Pow2RealFFT<double> {
	static constexpr bool prefersSplit = true;

	using Complex = std::complex<double>;

	Pow2RealFFT(size_t size = 0) {
		resize(size);
	}
	~Pow2RealFFT() {
		if (hasSetup) vDSP_destroy_fftsetupD(fftSetup);
	}

	void resize(size_t size) {
		_size = size;
		if (hasSetup) vDSP_destroy_fftsetupD(fftSetup);
		if (!size) {
			hasSetup = false;
			return;
		}

		splitReal.resize(size);
		splitImag.resize(size);
		log2 = std::log2(size);
		fftSetup = vDSP_create_fftsetupD(log2, FFT_RADIX2);
		hasSetup = fftSetup;
	}

	void fft(const double* input, Complex* output) {
		double mul = 0.5f;
		vDSP_vsmulD(input, 2, &mul, splitReal.data(), 1, _size/2);
		vDSP_vsmulD(input + 1, 2, &mul, splitImag.data(), 1, _size/2);
		DSPDoubleSplitComplex tmpSplit{splitReal.data(), splitImag.data()};
		vDSP_fft_zripD(fftSetup, &tmpSplit, 1, log2, kFFTDirection_Forward);
		vDSP_ztocD(&tmpSplit, 1, (DSPDoubleComplex *)output, 2, _size/2);
	}
	void fft(const double *inR, double *outR, double *outI) {
		DSPDoubleSplitComplex outputSplit{outR, outI};
		double mul = 0.5f;
		vDSP_vsmulD(inR, 2, &mul, outR, 1, _size/2);
		vDSP_vsmulD(inR + 1, 2, &mul, outI, 1, _size/2);
		vDSP_fft_zripD(fftSetup, &outputSplit, 1, log2, kFFTDirection_Forward);
	}

	void ifft(const Complex * input, double * output) {
		DSPDoubleSplitComplex tmpSplit{splitReal.data(), splitImag.data()};
		vDSP_ctozD((DSPDoubleComplex*)input, 2, &tmpSplit, 1, _size/2);
		vDSP_fft_zripD(fftSetup, &tmpSplit, 1, log2, kFFTDirection_Inverse);
		DSPDoubleSplitComplex outputSplit{output, output + 1};
		vDSP_zvmovD(&tmpSplit, 1, &outputSplit, 2, _size/2);
	}
	void ifft(const double *inR, const double *inI, double *outR) {
		DSPDoubleSplitComplex inputSplit{(double *)inR, (double *)inI};
		DSPDoubleSplitComplex tmpSplit{splitReal.data(), splitImag.data()};
		vDSP_fft_zropD(fftSetup, &inputSplit, 1, &tmpSplit, 1, log2, kFFTDirection_Inverse);
		DSPDoubleSplitComplex outputSplit{outR, outR + 1};
		// We can't use vDSP_ztoc without knowing the alignment
		vDSP_zvmovD(&tmpSplit, 1, &outputSplit, 2, _size/2);
	}

private:
	size_t _size = 0;
	bool hasSetup = false;
	FFTSetupD fftSetup;
	int log2 = 0;
	std::vector<double> splitReal, splitImag;
};

}} // namespace
