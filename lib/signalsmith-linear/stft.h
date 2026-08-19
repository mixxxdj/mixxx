#ifndef SIGNALSMITH_AUDIO_LINEAR_STFT_H
#define SIGNALSMITH_AUDIO_LINEAR_STFT_H

#include "./fft.h"

namespace signalsmith { namespace linear {

enum {
	STFT_SPECTRUM_PACKED=0,
	STFT_SPECTRUM_MODIFIED=1,
	STFT_SPECTRUM_UNPACKED=2,
};

/// A self-normalising STFT, with variable position/window for output blocks
template<typename Sample, bool splitComputation=false, int spectrumType=STFT_SPECTRUM_PACKED>
struct DynamicSTFT {
	static constexpr bool modified = (spectrumType == STFT_SPECTRUM_MODIFIED);
	static constexpr bool unpacked = (spectrumType == STFT_SPECTRUM_UNPACKED);
	RealFFT<Sample, splitComputation, modified> fft;

	using Complex = std::complex<Sample>;

	enum class WindowShape {ignore, acg, kaiser};
	static constexpr WindowShape acg = WindowShape::acg;
	static constexpr WindowShape kaiser = WindowShape::kaiser;

	void configure(size_t inChannels, size_t outChannels, size_t blockSamples, size_t extraInputHistory=0, size_t intervalSamples=0, Sample asymmetry=0) {
		_analysisChannels = inChannels;
		_synthesisChannels = outChannels;
		_blockSamples = blockSamples;
		_fftSamples = fft.fastSizeAbove((blockSamples + 1)/2)*2;
		fft.resize(_fftSamples);
		_fftBins = _fftSamples/2 + (spectrumType == STFT_SPECTRUM_UNPACKED);
		
		_inputLengthSamples = _blockSamples + extraInputHistory;
		input.buffer.resize(_inputLengthSamples*_analysisChannels);

		output.buffer.resize(_blockSamples*_synthesisChannels);
		output.windowProducts.resize(_blockSamples);
		spectrumBuffer.resize(_fftBins*std::max(_analysisChannels, _synthesisChannels));
		timeBuffer.resize(_fftSamples);

		_analysisWindow.resize(_blockSamples);
		_synthesisWindow.resize(_blockSamples);
		setInterval(intervalSamples ? intervalSamples : blockSamples/4, acg, asymmetry);

		reset();
	}
	
	size_t blockSamples() const {
		return _blockSamples;
	}
	size_t fftSamples() const {
		return _fftSamples;
	}
	size_t defaultInterval() const {
		return _defaultInterval;
	}
	size_t bands() const {
		return _fftBins;
	}
	size_t analysisLatency() const {
		return _blockSamples - _analysisOffset;
	}
	size_t synthesisLatency() const {
		return _synthesisOffset;
	}
	size_t latency() const {
		return synthesisLatency() + analysisLatency();
	}
	Sample binToFreq(Sample b) const {
		return (modified ? b + Sample(0.5) : b)/_fftSamples;
	}
	Sample freqToBin(Sample f) const {
		return modified ? f*_fftSamples - Sample(0.5) : f*_fftSamples;
	}

	void reset(Sample productWeight=1) {
		input.pos = _blockSamples;
		output.pos = 0;
		for (auto &v : input.buffer) v = 0;
		for (auto &v : output.buffer) v = 0;
		for (auto &v : spectrumBuffer) v = 0;
		for (auto &v : output.windowProducts) v = 0;
		addWindowProduct();
		for (int i = int(_blockSamples) - int(_defaultInterval) - 1; i >= 0; --i) {
			output.windowProducts[i] += output.windowProducts[i + _defaultInterval];
		}
		for (auto &v : output.windowProducts) v = v*productWeight + almostZero;
		moveOutput(_defaultInterval); // ready for first block immediately
	}

	void writeInput(size_t channel, size_t offset, size_t length, const Sample *inputArray) {
		Sample *buffer = input.buffer.data() + channel*_inputLengthSamples;

		size_t offsetPos = (input.pos + offset)%_inputLengthSamples;
		size_t inputWrapIndex = _inputLengthSamples - offsetPos;
		size_t chunk1 = std::min(length, inputWrapIndex);
		for (size_t i = 0; i < chunk1; ++i) {
			size_t i2 = offsetPos + i;
			buffer[i2] = inputArray[i];
		}
		for (size_t i = chunk1; i < length; ++i) {
			size_t i2 = i + offsetPos -_inputLengthSamples;
			buffer[i2] = inputArray[i];
		}
	}
	void writeInput(size_t channel, size_t length, const Sample *inputArray) {
		writeInput(channel, 0, length, inputArray);
	}
	void moveInput(size_t samples, bool clearInput=false) {
		if (clearInput) {
			size_t inputWrapIndex = _inputLengthSamples - input.pos;
			size_t chunk1 = std::min(samples, inputWrapIndex);
			for (size_t c = 0; c < _analysisChannels; ++c) {
				Sample *buffer = input.buffer.data() + c*_inputLengthSamples;
				for (size_t i = 0; i < chunk1; ++i) {
					size_t i2 = input.pos + i;
					buffer[i2] = 0;
				}
				for (size_t i = chunk1; i < samples; ++i) {
					size_t i2 = i + input.pos - _inputLengthSamples;
					buffer[i2] = 0;
				}
			}
		}

		input.pos = (input.pos + samples)%_inputLengthSamples;
		_samplesSinceAnalysis += samples;
	}
	size_t samplesSinceAnalysis() const {
		return _samplesSinceAnalysis;
	}

	/// When no more synthesis is expected, let output taper away to 0 based on windowing.  Otherwise, the output will be scaled as if there's just a very long block interval, which can exaggerate artefacts and numerical errors.  You still can't read more than `blockSamples()` into the future.
	void finishOutput(Sample strength=1, size_t offset=0) {
		Sample maxWindowProduct = 0;
		
		size_t chunk1 = std::max(offset, std::min(_blockSamples, _blockSamples - output.pos));
		
		for (size_t i = offset; i < chunk1; ++i) {
			size_t i2 = output.pos + i;
			Sample &wp = output.windowProducts[i2];
			maxWindowProduct = std::max(wp, maxWindowProduct);
			wp += (maxWindowProduct - wp)*strength;
		}
		for (size_t i = chunk1; i < _blockSamples; ++i) {
			size_t i2 = i + output.pos - _blockSamples;
			Sample &wp = output.windowProducts[i2];
			maxWindowProduct = std::max(wp, maxWindowProduct);
			wp += (maxWindowProduct - wp)*strength;
		}
	}

	void readOutput(size_t channel, size_t offset, size_t length, Sample *outputArray) {
		Sample *buffer = output.buffer.data() + channel*_blockSamples;
		size_t offsetPos = (output.pos + offset)%_blockSamples;
		size_t outputWrapIndex = _blockSamples - offsetPos;
		size_t chunk1 = std::min(length, outputWrapIndex);
		for (size_t i = 0; i < chunk1; ++i) {
			size_t i2 = offsetPos + i;
			outputArray[i] = buffer[i2]/output.windowProducts[i2];
		}
		for (size_t i = chunk1; i < length; ++i) {
			size_t i2 = i + offsetPos - _blockSamples;
			outputArray[i] = buffer[i2]/output.windowProducts[i2];
		}
	}
	void readOutput(size_t channel, size_t length, Sample *outputArray) {
		return readOutput(channel, 0, length, outputArray);
	}
	void addOutput(size_t channel, size_t offset, size_t length, const Sample *newOutputArray) {
		length = std::min(_blockSamples, length);
		Sample *buffer = output.buffer.data() + channel*_blockSamples;
		size_t offsetPos = (output.pos + offset)%_blockSamples;
		size_t outputWrapIndex = _blockSamples - offsetPos;
		size_t chunk1 = std::min(length, outputWrapIndex);
		for (size_t i = 0; i < chunk1; ++i) {
			size_t i2 = offsetPos + i;
			buffer[i2] += newOutputArray[i]*output.windowProducts[i2];
		}
		for (size_t i = chunk1; i < length; ++i) {
			size_t i2 = i + offsetPos - _blockSamples;
			buffer[i2] += newOutputArray[i]*output.windowProducts[i2];
		}
	}
	void addOutput(size_t channel, size_t length, const Sample *newOutputArray) {
		return addOutput(channel, 0, length, newOutputArray);
	}
	void replaceOutput(size_t channel, size_t offset, size_t length, const Sample *newOutputArray) {
		length = std::min(_blockSamples, length);
		Sample *buffer = output.buffer.data() + channel*_blockSamples;
		size_t offsetPos = (output.pos + offset)%_blockSamples;
		size_t outputWrapIndex = _blockSamples - offsetPos;
		size_t chunk1 = std::min(length, outputWrapIndex);
		for (size_t i = 0; i < chunk1; ++i) {
			size_t i2 = offsetPos + i;
			buffer[i2] = newOutputArray[i]*output.windowProducts[i2];
		}
		for (size_t i = chunk1; i < length; ++i) {
			size_t i2 = i + offsetPos - _blockSamples;
			buffer[i2] = newOutputArray[i]*output.windowProducts[i2];
		}
	}
	void replaceOutput(size_t channel, size_t length, const Sample *newOutputArray) {
		return replaceOutput(channel, 0, length, newOutputArray);
	}
	void moveOutput(size_t samples) {
		if (samples == 1) { // avoid all the loops/chunks if we can
			for (size_t c = 0; c < _synthesisChannels; ++c) {
				output.buffer[output.pos + c*_blockSamples] = 0;
			}
			output.windowProducts[output.pos] = almostZero;
			if (++output.pos >= _blockSamples) output.pos = 0;
			return;
		}
		// Zero the output buffer as we cross it
		size_t outputWrapIndex = _blockSamples - output.pos;
		size_t chunk1 = std::min(samples, outputWrapIndex);
		for (size_t c = 0; c < _synthesisChannels; ++c) {
			Sample *buffer = output.buffer.data() + c*_blockSamples;
			for (size_t i = 0; i < chunk1; ++i) {
				size_t i2 = output.pos + i;
				buffer[i2] = 0;
			}
			for (size_t i = chunk1; i < samples; ++i) {
				size_t i2 = i + output.pos - _blockSamples;
				buffer[i2] = 0;
			}
		}
		for (size_t i = 0; i < chunk1; ++i) {
			size_t i2 = output.pos + i;
			output.windowProducts[i2] = almostZero;
		}
		for (size_t i = chunk1; i < samples; ++i) {
			size_t i2 = i + output.pos - _blockSamples;
			output.windowProducts[i2] = almostZero;
		}
		output.pos = (output.pos + samples)%_blockSamples;
		_samplesSinceSynthesis += samples;
	}
	size_t samplesSinceSynthesis() const {
		return _samplesSinceSynthesis;
	}
	
	Complex * spectrum(size_t channel) {
		return spectrumBuffer.data() + channel*_fftBins;
	}
	const Complex * spectrum(size_t channel) const {
		return spectrumBuffer.data() + channel*_fftBins;
	}

	Sample * analysisWindow() {
		return _analysisWindow.data();
	}
	const Sample * analysisWindow() const {
		return _analysisWindow.data();
	}
	// Sets the centre index of the window
	void analysisOffset(size_t offset) {
		_analysisOffset = offset;
	}
	size_t analysisOffset() const {
		return _analysisOffset;
	}
	Sample * synthesisWindow() {
		return _synthesisWindow.data();
	}
	const Sample * synthesisWindow() const {
		return _synthesisWindow.data();
	}
	// Sets the centre index of the window
	void synthesisOffset(size_t offset) {
		_synthesisOffset = offset;
	}
	size_t synthesisOffset() const {
		return _synthesisOffset;
	}
	
	void setInterval(size_t defaultInterval, WindowShape windowShape=WindowShape::ignore, Sample asymmetry=0) {
		_defaultInterval = defaultInterval;
		if (windowShape == WindowShape::ignore) return;

		if (windowShape == acg) {
			auto window = ApproximateConfinedGaussian::withBandwidth(double(_blockSamples)/defaultInterval);
			window.fill(_synthesisWindow, _blockSamples, asymmetry, false);
		} else if (windowShape == kaiser) {
			auto window = Kaiser::withBandwidth(double(_blockSamples)/defaultInterval, true);
			window.fill(_synthesisWindow,  _blockSamples, asymmetry, true);
		}

		_analysisOffset = _synthesisOffset = _blockSamples/2;
		if (_analysisChannels == 0) {
			for (auto &v : _analysisWindow) v = 1;
		} else if (asymmetry == 0) {
			forcePerfectReconstruction(_synthesisWindow, _blockSamples, _defaultInterval);
			for (size_t i = 0; i < _blockSamples; ++i) {
				_analysisWindow[i] = _synthesisWindow[i];
			}
		} else {
			for (size_t i = 0; i < _blockSamples; ++i) {
				_analysisWindow[i] = _synthesisWindow[_blockSamples - 1 - i];
			}
		}
		// Set offsets to peak's index
		for (size_t i = 0; i < _blockSamples; ++i) {
			if (_analysisWindow[i] > _analysisWindow[_analysisOffset]) _analysisOffset = i;
			if (_synthesisWindow[i] > _synthesisWindow[_synthesisOffset]) _synthesisOffset = i;
		}
	}
	
	void analyse(size_t samplesInPast=0) {
		for (size_t s = 0; s < analyseSteps(); ++s) {
			analyseStep(s, samplesInPast);
		}
	}
	size_t analyseSteps() const {
		return splitComputation ? _analysisChannels*(fft.steps() + 1) : _analysisChannels;
	}
	void analyseStep(size_t step, std::size_t samplesInPast=0) {
		size_t fftSteps = splitComputation ? fft.steps() : 0;
		size_t channel = step/(fftSteps + 1);
		step -= channel*(fftSteps + 1);
		
		if (step-- == 0) { // extra step at start of each channel: copy windowed input into buffer
			size_t offsetPos = (_inputLengthSamples*2 + input.pos - _blockSamples - samplesInPast)%_inputLengthSamples;
			size_t inputWrapIndex = _inputLengthSamples - offsetPos;
			size_t chunk1 = std::min(_analysisOffset, inputWrapIndex);
			size_t chunk2 = std::max(_analysisOffset, std::min(_blockSamples, inputWrapIndex));

			_samplesSinceAnalysis = samplesInPast;
			Sample *buffer = input.buffer.data() + channel*_inputLengthSamples;
			for (size_t i = 0; i < chunk1; ++i) {
				Sample w = modified ? -_analysisWindow[i] : _analysisWindow[i];
				size_t ti = i + (_fftSamples - _analysisOffset);
				size_t bi = offsetPos + i;
				timeBuffer[ti] = buffer[bi]*w;
			}
			for (size_t i = chunk1; i < _analysisOffset; ++i) {
				Sample w = modified ? -_analysisWindow[i] : _analysisWindow[i];
				size_t ti = i + (_fftSamples - _analysisOffset);
				size_t bi = i + offsetPos - _inputLengthSamples;
				timeBuffer[ti] = buffer[bi]*w;
			}
			for (size_t i = _analysisOffset; i < chunk2; ++i) {
				Sample w = _analysisWindow[i];
				size_t ti = i - _analysisOffset;
				size_t bi = offsetPos + i;
				timeBuffer[ti] = buffer[bi]*w;
			}
			for (size_t i = chunk2; i < _blockSamples; ++i) {
				Sample w = _analysisWindow[i];
				size_t ti = i - _analysisOffset;
				size_t bi = i + offsetPos - _inputLengthSamples;
				timeBuffer[ti] = buffer[bi]*w;
			}
			for (size_t i = _blockSamples - _analysisOffset; i < _fftSamples - _analysisOffset; ++i) {
				timeBuffer[i] = 0;
			}
			if (splitComputation) return;
		}
		auto *spectrumPtr = spectrum(channel);
		if (splitComputation) {
			fft.fft(step, timeBuffer.data(), spectrumPtr);
			if (unpacked && step == fft.steps() - 1) {
				spectrumPtr[_fftBins - 1] = spectrumPtr[0].imag();
				spectrumPtr[0].imag(0);
			}
		} else {
			fft.fft(timeBuffer.data(), spectrum(channel));
			if (unpacked) {
				spectrumPtr[_fftBins - 1] = spectrumPtr[0].imag();
				spectrumPtr[0].imag(0);
			}
		}
	}

	void synthesise() {
		for (size_t s = 0; s < synthesiseSteps(); ++s) {
			synthesiseStep(s);
		}
	}
	size_t synthesiseSteps() const {
		return splitComputation ? (_synthesisChannels*(fft.steps() + 1) + 1) : _synthesisChannels;
	}
	void synthesiseStep(size_t step) {
		if (step == 0) { // Extra first step which adds in the effective gain for a pure analysis-synthesis cycle
			addWindowProduct();
			if (splitComputation) return;
		}
		if (splitComputation) --step;

		size_t fftSteps = splitComputation ? fft.steps() : 0;
		size_t channel = step/(fftSteps + 1);
		step -= channel*(fftSteps + 1);

		auto *spectrumPtr = spectrum(channel);
		if (unpacked && step == 0) { // re-pack
			spectrumPtr[0].imag(spectrumPtr[_fftBins - 1].real());
		}

		if (splitComputation) {
			if (step < fftSteps) {
				fft.ifft(step, spectrumPtr, timeBuffer.data());
				return;
			}
		} else {
			fft.ifft(spectrumPtr, timeBuffer.data());
		}
		
		// extra step after each channel's FFT
		Sample *buffer = output.buffer.data() + channel*_blockSamples;
		size_t outputWrapIndex = _blockSamples - output.pos;
		size_t chunk1 = std::min(_synthesisOffset, outputWrapIndex);
		size_t chunk2 = std::min(_blockSamples, std::max(_synthesisOffset, outputWrapIndex));
		
		for (size_t i = 0; i < chunk1; ++i) {
			Sample w = modified ? -_synthesisWindow[i] : _synthesisWindow[i];
			size_t ti = i + (_fftSamples - _synthesisOffset);
			size_t bi = output.pos + i;
			buffer[bi] += timeBuffer[ti]*w;
		}
		for (size_t i = chunk1; i < _synthesisOffset; ++i) {
			Sample w = modified ? -_synthesisWindow[i] : _synthesisWindow[i];
			size_t ti = i + (_fftSamples - _synthesisOffset);
			size_t bi = i + output.pos - _blockSamples;
			buffer[bi] += timeBuffer[ti]*w;
		}
		for (size_t i = _synthesisOffset; i < chunk2; ++i) {
			Sample w = _synthesisWindow[i];
			size_t ti = i - _synthesisOffset;
			size_t bi = output.pos + i;
			buffer[bi] += timeBuffer[ti]*w;
		}
		for (size_t i = chunk2; i < _blockSamples; ++i) {
			Sample w = _synthesisWindow[i];
			size_t ti = i - _synthesisOffset;
			size_t bi = i + output.pos - _blockSamples;
			buffer[bi] += timeBuffer[ti]*w;
		}
	}

#define COMPAT_SPELLING(name, alt) \
	template<class ...Args> \
	void alt(Args &&...args) { \
		name(std::forward<Args>(args)...); \
	}
	COMPAT_SPELLING(analyse, analyze);
	COMPAT_SPELLING(analyseStep, analyseStep);
	COMPAT_SPELLING(analyseSteps, analyzeSteps);
	COMPAT_SPELLING(synthesise, synthesize);
	COMPAT_SPELLING(synthesiseStep, synthesizeStep);
	COMPAT_SPELLING(synthesiseSteps, synthesizeSteps);

	/// Input (only available so we can save/restore the input state)
	struct Input {
		void swap(Input &other) {
			std::swap(pos, other.pos);
			std::swap(buffer, other.buffer);
		}
		
		Input & operator=(const Input &other) {
			pos = other.pos;
			buffer.assign(other.buffer.begin(), other.buffer.end());
			return *this;
		}
	private:
		friend struct DynamicSTFT;
		size_t pos = 0;
		std::vector<Sample> buffer;
	};
	Input input;
	
	/// Output (only available so we can save/restore the output state)
	struct Output {
		void swap(Output &other) {
			std::swap(pos, other.pos);
			std::swap(buffer, other.buffer);
			std::swap(windowProducts, other.windowProducts);
		}

		Output & operator=(const Output &other) {
			pos = other.pos;
			buffer.assign(other.buffer.begin(), other.buffer.end());
			windowProducts.assign(other.windowProducts.begin(), other.windowProducts.end());
			return *this;
		}
	private:
		friend struct DynamicSTFT;
		size_t pos = 0;
		std::vector<Sample> buffer;
		std::vector<Sample> windowProducts;
	};
	Output output;

private:
	static constexpr Sample almostZero = 1e-30;

	size_t _analysisChannels, _synthesisChannels, _inputLengthSamples, _blockSamples, _fftSamples, _fftBins;
	size_t _defaultInterval = 0;

	std::vector<Sample> _analysisWindow, _synthesisWindow;
	size_t _analysisOffset = 0, _synthesisOffset = 0;

	std::vector<Complex> spectrumBuffer;
	std::vector<Sample> timeBuffer;

	size_t _samplesSinceSynthesis = 0, _samplesSinceAnalysis = 0;
	
	void addWindowProduct() {
		_samplesSinceSynthesis = 0;

		int windowShift = int(_synthesisOffset) - int(_analysisOffset);
		size_t wMin = std::max<ptrdiff_t>(0, windowShift);
		size_t wMax = std::min<ptrdiff_t>(_blockSamples, int(_blockSamples) + windowShift);

		Sample *windowProduct = output.windowProducts.data();
		size_t outputWrapIndex = _blockSamples - output.pos;
		size_t chunk1 = std::min<size_t>(wMax, std::max<size_t>(wMin, outputWrapIndex));
		for (size_t i = wMin; i < chunk1; ++i) {
			Sample wa = _analysisWindow[i - windowShift];
			Sample ws = _synthesisWindow[i];
			size_t bi = output.pos + i;
			windowProduct[bi] += wa*ws*_fftSamples;
		}
		for (size_t i = chunk1; i < wMax; ++i) {
			Sample wa = _analysisWindow[i - windowShift];
			Sample ws = _synthesisWindow[i];
			size_t bi = i + output.pos - _blockSamples;
			windowProduct[bi] += wa*ws*_fftSamples;
		}
	}
	
	// Copied from DSP library `windows.h`
	class Kaiser {
		inline static double bessel0(double x) {
			const double significanceLimit = 1e-4;
			double result = 0;
			double term = 1;
			double m = 0;
			while (term > significanceLimit) {
				result += term;
				++m;
				term *= (x*x)/(4*m*m);
			}

			return result;
		}
		double beta;
		double invB0;
		
		static double heuristicBandwidth(double bandwidth) {
			return bandwidth + 8/((bandwidth + 3)*(bandwidth + 3)) + 0.25*std::max(3 - bandwidth, 0.0);
		}
	public:
		Kaiser(double beta) : beta(beta), invB0(1/bessel0(beta)) {}

		static Kaiser withBandwidth(double bandwidth, bool heuristicOptimal=false) {
			return Kaiser(bandwidthToBeta(bandwidth, heuristicOptimal));
		}
		static double bandwidthToBeta(double bandwidth, bool heuristicOptimal=false) {
			if (heuristicOptimal) { // Heuristic based on numerical search
				bandwidth = heuristicBandwidth(bandwidth);
			}
			bandwidth = std::max(bandwidth, 2.0);
			double alpha = std::sqrt(bandwidth*bandwidth*0.25 - 1);
			return alpha*M_PI;
		}
		
		template<typename Data>
		void fill(Data &&data, size_t size, double warp, bool isForSynthesis) const {
			double invSize = 1.0/size;
			size_t offsetI = (size&1) ? 1 : (isForSynthesis ? 0 : 2);
			for (size_t i = 0; i < size; ++i) {
				double r = (2*i + offsetI)*invSize - 1;
				r = (r + warp)/(1 + r*warp);
				double arg = std::sqrt(1 - r*r);
				data[i] = bessel0(beta*arg)*invB0;
			}
			if (warp) {
				// Warp window vertically as well, to restore some width
				for (size_t i = 0; i < size; ++i) {
					data[i] *= std::sqrt((warp + 1)/(1 + warp*(2*data[i] - 1)));
				}
			}
		}
	};

	class ApproximateConfinedGaussian {
		double gaussianFactor;
		
		double gaussian(double x) const {
			return std::exp(-x*x*gaussianFactor);
		}
	public:
		static double bandwidthToSigma(double bandwidth) {
			return 0.3/std::sqrt(bandwidth);
		}
		static ApproximateConfinedGaussian withBandwidth(double bandwidth) {
			return ApproximateConfinedGaussian(bandwidthToSigma(bandwidth));
		}

		ApproximateConfinedGaussian(double sigma) : gaussianFactor(0.0625/(sigma*sigma)) {}
	
		/// Fills an arbitrary container
		template<typename Data>
		void fill(Data &&data, size_t size, double warp, bool isForSynthesis) const {
			double invSize = 1.0/size;
			double offsetScale = gaussian(1)/(gaussian(3) + gaussian(-1));
			double norm = 1/(gaussian(0) - 2*offsetScale*(gaussian(2)));
			size_t offsetI = (size&1) ? 1 : (isForSynthesis ? 0 : 2);
			for (size_t i = 0; i < size; ++i) {
				double r = (2*i + offsetI)*invSize - 1;
				r = (r + warp)/(1 + r*warp);
				data[i] = norm*(gaussian(r) - offsetScale*(gaussian(r - 2) + gaussian(r + 2)));
			}
			if (warp) {
				// Warp window vertically as well, to restore some width
				for (size_t i = 0; i < size; ++i) {
					data[i] *= std::sqrt((warp + 1)/(1 + warp*(2*data[i] - 1)));
				}
			}
		}
	};

	template<typename Data>
	void forcePerfectReconstruction(Data &&data, size_t windowLength, size_t interval) {
		for (size_t i = 0; i < interval; ++i) {
			double sum2 = 0;
			for (size_t index = i; index < windowLength; index += interval) {
				sum2 += data[index]*data[index];
			}
			double factor = 1/std::sqrt(sum2);
			for (size_t index = i; index < windowLength; index += interval) {
				data[index] *= factor;
			}
		}
	}
};

}} // namespace

#endif // include guard
