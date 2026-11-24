#ifndef FFT2_FFT2_IPP_H
#define FFT2_FFT2_IPP_H

#include <ipp/ippcore.h>
#include <ipp/ipps.h>

#include <memory>
#include <cmath>
#include <complex>
#include <cassert>

namespace signalsmith { namespace linear {

namespace {

void checkStatus(IppStatus status) {
  assert(status == ippStsNoErr);
}

template<typename T, bool complex>
struct State{};

template<>
struct State<float, true> {
public:
    explicit State(size_t size) {
        IppStatus status;
        const auto order = static_cast<int>(std::log2(size));
        Ipp8u* initBuffer{ nullptr };

        int maxWorkingSize = 0;

        {
              int fftSpecSize, fftInitBuffSize, fftWorkBuffSize;
              status = ippsFFTGetSize_C_32fc(order, IPP_FFT_NODIV_BY_ANY, ippAlgHintFast, &fftSpecSize, &fftInitBuffSize, &fftWorkBuffSize);
              checkStatus(status);
              maxWorkingSize = std::max(maxWorkingSize, fftWorkBuffSize);
              specBuffer = ippsMalloc_8u(fftSpecSize);
              spec = (IppsFFTSpec_C_32fc*)specBuffer;
              if (fftInitBuffSize != 0) {
                    initBuffer = ippsMalloc_8u(fftInitBuffSize);
              }
              status = ippsFFTInit_C_32fc(&spec, order, IPP_FFT_NODIV_BY_ANY, ippAlgHintFast, specBuffer, initBuffer);

              checkStatus(status);
              if (initBuffer) {
                    ippsFree(initBuffer);
              }
        }

        {
              int fftSpecSize, fftInitBuffSize, fftWorkBuffSize;
              status = ippsFFTGetSize_C_32f(order, IPP_FFT_NODIV_BY_ANY, ippAlgHintFast, &fftSpecSize, &fftInitBuffSize, &fftWorkBuffSize);
              checkStatus(status);
              maxWorkingSize = std::max(maxWorkingSize, fftWorkBuffSize);
              specSplitBuffer = ippsMalloc_8u(fftSpecSize);
              specSplit = (IppsFFTSpec_C_32f*)specSplitBuffer;
              if (fftInitBuffSize != 0) {
                    initBuffer = ippsMalloc_8u(fftInitBuffSize);
              }
              status = ippsFFTInit_C_32f(&specSplit, order, IPP_FFT_NODIV_BY_ANY, ippAlgHintFast, specSplitBuffer, initBuffer);

              checkStatus(status);
              if (initBuffer) {
                    ippsFree(initBuffer);
              }
        }

        if(maxWorkingSize != 0) {
              workBuffer = ippsMalloc_8u(maxWorkingSize);
              workSize = maxWorkingSize;
        }
    }

    ~State() noexcept {
          if(specBuffer) {
                ippsFree(specBuffer);
          }
          if(specSplitBuffer) {
                ippsFree(specSplitBuffer);
          }
          if(workBuffer) {
            ippsFree(workBuffer);
        }
    }

    IppsFFTSpec_C_32fc *spec{nullptr};
    IppsFFTSpec_C_32f *specSplit{nullptr};
    Ipp8u *specBuffer{nullptr};
    Ipp8u *specSplitBuffer{nullptr};
    Ipp8u *workBuffer{nullptr};
    size_t workSize = 0;
};

template<>
struct State<float, false> {
public:
      explicit State(size_t size) {
            IppStatus status;
            const auto order = static_cast<int>(std::log2(size));
            Ipp8u* initBuffer{ nullptr };

            int maxWorkingSize = 0;

            int fftSpecSize, fftInitBuffSize, fftWorkBuffSize;
            status = ippsFFTGetSize_R_32f(order, IPP_FFT_NODIV_BY_ANY, ippAlgHintFast, &fftSpecSize, &fftInitBuffSize, &fftWorkBuffSize);
            checkStatus(status);
            maxWorkingSize = std::max(maxWorkingSize, fftWorkBuffSize);
            specBuffer = ippsMalloc_8u(fftSpecSize);
            spec = (IppsFFTSpec_R_32f*)specBuffer;
            if (fftInitBuffSize != 0) {
                  initBuffer = ippsMalloc_8u(fftInitBuffSize);
            }
            status = ippsFFTInit_R_32f(&spec, order, IPP_FFT_NODIV_BY_ANY, ippAlgHintFast, specBuffer, initBuffer);

            checkStatus(status);
            if (initBuffer) {
                  ippsFree(initBuffer);
            }

            if(maxWorkingSize != 0) {
                  workBuffer = ippsMalloc_8u(maxWorkingSize);
                  workSize = maxWorkingSize;
            }
      }

      ~State() noexcept {
            if(specBuffer) {
                  ippsFree(specBuffer);
            }
            if(workBuffer) {
                  ippsFree(workBuffer);
            }
      }

      IppsFFTSpec_R_32f *spec{nullptr};
      Ipp8u *specBuffer{nullptr};
      Ipp8u *workBuffer{nullptr};
      size_t workSize = 0;
};

template<>
struct State<double, true> {
      explicit State(size_t size) {
            IppStatus status;
            const auto order = static_cast<int>(std::log2(size));
            Ipp8u* initBuffer{ nullptr };

            int maxWorkingSize = 0;

            {
                  int fftSpecSize, fftInitBuffSize, fftWorkBuffSize;
                  status = ippsFFTGetSize_C_64fc(order, IPP_FFT_NODIV_BY_ANY, ippAlgHintFast, &fftSpecSize, &fftInitBuffSize, &fftWorkBuffSize);
                  checkStatus(status);
                  maxWorkingSize = std::max(maxWorkingSize, fftWorkBuffSize);
                  specBuffer = ippsMalloc_8u(fftSpecSize);
                  spec = (IppsFFTSpec_C_64fc*)specBuffer;
                  if (fftInitBuffSize != 0) {
                        initBuffer = ippsMalloc_8u(fftInitBuffSize);
                  }
                  status = ippsFFTInit_C_64fc(&spec, order, IPP_FFT_NODIV_BY_ANY, ippAlgHintFast, specBuffer, initBuffer);

                  checkStatus(status);
                  if (initBuffer) {
                        ippsFree(initBuffer);
                  }
            }

            {
                  int fftSpecSize, fftInitBuffSize, fftWorkBuffSize;
                  status = ippsFFTGetSize_C_64f(order, IPP_FFT_NODIV_BY_ANY, ippAlgHintFast, &fftSpecSize, &fftInitBuffSize, &fftWorkBuffSize);
                  checkStatus(status);
                  maxWorkingSize = std::max(maxWorkingSize, fftWorkBuffSize);
                  specSplitBuffer = ippsMalloc_8u(fftSpecSize);
                  specSplit = (IppsFFTSpec_C_64f*)specSplitBuffer;
                  if (fftInitBuffSize != 0) {
                        initBuffer = ippsMalloc_8u(fftInitBuffSize);
                  }
                  status = ippsFFTInit_C_64f(&specSplit, order, IPP_FFT_NODIV_BY_ANY, ippAlgHintFast, specSplitBuffer, initBuffer);

                  checkStatus(status);
                  if (initBuffer) {
                        ippsFree(initBuffer);
                  }
            }

            if(maxWorkingSize != 0) {
                  workBuffer = ippsMalloc_8u(maxWorkingSize);
                  workSize = maxWorkingSize;
            }
      }

      ~State() noexcept {
            if(specBuffer) {
                  ippsFree(specBuffer);
            }
            if(specSplitBuffer) {
                  ippsFree(specSplitBuffer);
            }
            if(workBuffer) {
                  ippsFree(workBuffer);
            }
      }

      IppsFFTSpec_C_64fc *spec{nullptr};
      IppsFFTSpec_C_64f *specSplit{nullptr};
      Ipp8u *specBuffer{nullptr};
      Ipp8u *specSplitBuffer{nullptr};
      Ipp8u *workBuffer{nullptr};
      size_t workSize = 0;
};

template<>
struct State<double, false> {
    explicit State(size_t size) {
        IppStatus status;
        const auto order = static_cast<int>(std::log2(size));
        Ipp8u* initBuffer{ nullptr };

        int maxWorkingSize = 0;

            int fftSpecSize, fftInitBuffSize, fftWorkBuffSize;
            status = ippsFFTGetSize_R_64f(order, IPP_FFT_NODIV_BY_ANY, ippAlgHintFast, &fftSpecSize, &fftInitBuffSize, &fftWorkBuffSize);
            checkStatus(status);
            maxWorkingSize = std::max(maxWorkingSize, fftWorkBuffSize);
            specBuffer = ippsMalloc_8u(fftSpecSize);
            spec = (IppsFFTSpec_R_64f*)specBuffer;
            if (fftInitBuffSize != 0) {
                  initBuffer = ippsMalloc_8u(fftInitBuffSize);
            }
            status = ippsFFTInit_R_64f(&spec, order, IPP_FFT_NODIV_BY_ANY, ippAlgHintFast, specBuffer, initBuffer);

            checkStatus(status);
            if (initBuffer) {
                  ippsFree(initBuffer);
            }


        if(maxWorkingSize != 0) {
              workBuffer = ippsMalloc_8u(maxWorkingSize);
              workSize = maxWorkingSize;
        }
    }

    ~State() noexcept {
          if(specBuffer) {
                ippsFree(specBuffer);
          }
          if(workBuffer) {
            ippsFree(workBuffer);
        }
    }

    IppsFFTSpec_R_64f *spec{nullptr};
    Ipp8u *specBuffer{nullptr};
    Ipp8u *workBuffer{nullptr};
    size_t workSize = 0;

};
} // namespace


namespace _impl {
      template<>
      void complexMul<float>(std::complex<float> *a, const std::complex<float> *b, const std::complex<float> *c, size_t size) {
            auto status = ippsMul_32fc((const Ipp32fc*)b, (const Ipp32fc*)c, (Ipp32fc*)a, int(size));
            checkStatus(status);
      }
      //template<>
      //void complexMulConj<float>(std::complex<float> *a, const std::complex<float> *b, const std::complex<float> *c, size_t size) {
      //      DSPSplitComplex aSplit = {(float *)a, (float *)a + 1};
      //      DSPSplitComplex bSplit = {(float *)b, (float *)b + 1};
      //      DSPSplitComplex cSplit = {(float *)c, (float *)c + 1};
      //      vDSP_zvmul(&cSplit, 2, &bSplit, 2, &aSplit, 2, size, -1);
      //}
      //template<>
      //void complexMul<float>(float *ar, float *ai, const float *br, const float *bi, const float *cr, const float *ci, size_t size) {
      //      DSPSplitComplex aSplit = {ar, ai};
      //      DSPSplitComplex bSplit = {(float *)br, (float *)bi};
      //      DSPSplitComplex cSplit = {(float *)cr, (float *)ci};
      //      vDSP_zvmul(&cSplit, 1, &bSplit, 1, &aSplit, 1, size, 1);
      //}
      //template<>
      //void complexMulConj<float>(float *ar, float *ai, const float *br, const float *bi, const float *cr, const float *ci, size_t size) {
      //      DSPSplitComplex aSplit = {ar, ai};
      //      DSPSplitComplex bSplit = {(float *)br, (float *)bi};
      //      DSPSplitComplex cSplit = {(float *)cr, (float *)ci};
      //      vDSP_zvmul(&cSplit, 1, &bSplit, 1, &aSplit, 1, size, -1);
      //}

      // doubles
      template<>
      void complexMul<double>(std::complex<double> *a, const std::complex<double> *b, const std::complex<double> *c, size_t size) {
            auto status = ippsMul_64fc((const Ipp64fc*)b, (const Ipp64fc*)c, (Ipp64fc*)a, int(size));
            checkStatus(status);
      }
      //template<>
      //void complexMulConj<double>(std::complex<double> *a, const std::complex<double> *b, const std::complex<double> *c, size_t size) {
      //      DSPDoubleSplitComplex aSplit = {(double *)a, (double *)a + 1};
      //      DSPDoubleSplitComplex bSplit = {(double *)b, (double *)b + 1};
      //      DSPDoubleSplitComplex cSplit = {(double *)c, (double *)c + 1};
      //      vDSP_zvmulD(&cSplit, 2, &bSplit, 2, &aSplit, 2, size, -1);
      //}
      //template<>
      //void complexMul<double>(double *ar, double *ai, const double *br, const double *bi, const double *cr, const double *ci, size_t size) {
      //      DSPDoubleSplitComplex aSplit = {ar, ai};
      //      DSPDoubleSplitComplex bSplit = {(double *)br, (double *)bi};
      //      DSPDoubleSplitComplex cSplit = {(double *)cr, (double *)ci};
      //      vDSP_zvmulD(&cSplit, 1, &bSplit, 1, &aSplit, 1, size, 1);
      //}
      //template<>
      //void complexMulConj<double>(double *ar, double *ai, const double *br, const double *bi, const double *cr, const double *ci, size_t size) {
      //      DSPDoubleSplitComplex aSplit = {ar, ai};
      //      DSPDoubleSplitComplex bSplit = {(double *)br, (double *)bi};
      //      DSPDoubleSplitComplex cSplit = {(double *)cr, (double *)ci};
      //      vDSP_zvmulD(&cSplit, 1, &bSplit, 1, &aSplit, 1, size, -1);
      //}
}

template<>
struct Pow2FFT<float> {
      static constexpr bool prefersSplit = true; // whether this FFT implementation is faster when given split-complex inputs

      Pow2FFT(size_t size = 0) {
        if(size > 0) {
            resize(size);
        }
    }

    void resize(size_t size) {
        m_state = std::make_unique<State<float, true>>(size);
    }

    void fft(const std::complex<float>* input, std::complex<float>* output) {
          auto* ippComplexIn = reinterpret_cast<const Ipp32fc*>(input);
          auto* ippComplexOut = reinterpret_cast<Ipp32fc*>(output);
          const auto status = ippsFFTFwd_CToC_32fc(ippComplexIn, ippComplexOut, m_state->spec, m_state->workBuffer);
          checkStatus(status);
    }

    void fft(const float *inR, const float *inI, float *outR, float *outI) {
          const auto status = ippsFFTFwd_CToC_32f((const Ipp32f*)inR, (const Ipp32f*)inI, (Ipp32f*)outR, (Ipp32f*)outI, m_state->specSplit, m_state->workBuffer);
          checkStatus(status);
    }

    void ifft(const std::complex<float>* input, std::complex<float>* output) {
          auto* ippComplexIn = reinterpret_cast<const Ipp32fc*>(input);
          auto* ippComplexOut = reinterpret_cast<Ipp32fc*>(output);
          const auto status = ippsFFTInv_CToC_32fc(ippComplexIn, ippComplexOut, m_state->spec, m_state->workBuffer);
          checkStatus(status);
    }

    void ifft(const float *inR, const float *inI, float *outR, float *outI) {
          const auto status = ippsFFTInv_CToC_32f((const Ipp32f*)inR, (const Ipp32f*)inI, (Ipp32f*)outR, (Ipp32f*)outI, m_state->specSplit, m_state->workBuffer);
          checkStatus(status);
    }

private:
  std::unique_ptr<State<float, true>> m_state{ nullptr };
};

template<>
struct Pow2RealFFT<float> {
      static constexpr bool prefersSplit = false; // whether this FFT implementation is faster when given split-complex inputs

      Pow2RealFFT(size_t size = 0) {
            if(size > 0) {
                  resize(size);
            }
      }

      void resize(size_t size) {
            hSize = size/2;
            size = hSize * 2;
            m_state = std::make_unique<State<float, false>>(size);

            // Either use the existing working buffer (if it's big enough), or allocate a new one using std::vector<>
            size_t complexAlign = alignof(std::complex<float>);
            size_t work = size_t(m_state->workBuffer);
            size_t aligned = (work + (complexAlign - 1)) & ~(complexAlign - 1);

            if (work + m_state->workSize - aligned >= size * sizeof(std::complex<float>)) {
                  tmp = (std::complex<float> *) aligned;
                  tmpVector.resize(0);
            } else {
                  tmpVector.resize(size);
                  tmp = tmpVector.data();
            }
      }

      void fft(const float *input, std::complex<float>* output) {
            const auto status = ippsFFTFwd_RToPerm_32f((const Ipp32f*)input, (Ipp32f*)output, m_state->spec, m_state->workBuffer);
            checkStatus(status);
      }

      void fft(const float *inR, float *outR, float *outI) {
            const auto status = ippsFFTFwd_RToPerm_32f((const Ipp32f*)inR, (Ipp32f*)tmp, m_state->spec, m_state->workBuffer);
            checkStatus(status);
            for (size_t i = 0; i < hSize; ++i) {
                  outR[i] = tmp[i].real();
                  outI[i] = tmp[i].imag();
            }
      }

      void ifft(const std::complex<float>* input, float *output) {
            const auto status = ippsFFTInv_PermToR_32f((const Ipp32f*)input, (Ipp32f*)output, m_state->spec, m_state->workBuffer);
            checkStatus(status);
      }

      void ifft(const float *inR, const float *inI, float *outR) {
            for (size_t i = 0; i < hSize; ++i) {
                  tmp[i] = { inR[i], inI[i] };
            }
            const auto status = ippsFFTInv_PermToR_32f((const Ipp32f*)tmp, (Ipp32f*)outR, m_state->spec, m_state->workBuffer);
            checkStatus(status);
      }

private:
      std::unique_ptr<State<float, false>> m_state{ nullptr };
      std::complex<float>* tmp = nullptr;
      std::vector<std::complex<float>> tmpVector;
      size_t hSize;
};

template<>
struct Pow2FFT<double> {
      static constexpr bool prefersSplit = true; // whether this FFT implementation is faster when given split-complex inputs
      
      Pow2FFT(size_t size = 0) {
        if(size > 0) {
            resize(size);
        }
    }

    void resize(size_t size) {
        m_state = std::make_unique<State<double, true>>(size);
    }

    void fft(const std::complex<double>* input, std::complex<double>* output) {
        auto* ippComplexIn = reinterpret_cast<const Ipp64fc*>(input);
        auto* ippComplexOut = reinterpret_cast<Ipp64fc*>(output);
        const auto status = ippsFFTFwd_CToC_64fc(ippComplexIn, ippComplexOut, m_state->spec, m_state->workBuffer);
        checkStatus(status);
    }

    void fft(const double *inR, const double *inI, double *outR, double *outI) {
          const auto status = ippsFFTFwd_CToC_64f((const Ipp64f*)inR, (const Ipp64f*)inI, (Ipp64f*)outR, (Ipp64f*)outI, m_state->specSplit, m_state->workBuffer);
          checkStatus(status);
    }

    void ifft(const std::complex<double>* input, std::complex<double>* output) {
        auto* ippComplexIn = reinterpret_cast<const Ipp64fc*>(input);
        auto* ippComplexOut = reinterpret_cast<Ipp64fc*>(output);
        const auto status = ippsFFTInv_CToC_64fc(ippComplexIn, ippComplexOut, m_state->spec, m_state->workBuffer);
        checkStatus(status);
    }

    void ifft(const double *inR, const double *inI, double *outR, double *outI) {
          const auto status = ippsFFTInv_CToC_64f((const Ipp64f*)inR, (const Ipp64f*)inI, (Ipp64f*)outR, (Ipp64f*)outI, m_state->specSplit, m_state->workBuffer);
          checkStatus(status);
    }

private:
  std::unique_ptr<State<double, true>> m_state{ nullptr };
};

template<>
struct Pow2RealFFT<double> {
      static constexpr bool prefersSplit = false; // whether this FFT implementation is faster when given split-complex inputs

      Pow2RealFFT(size_t size = 0) {
            if(size > 0) {
                  resize(size);
            }
      }

      void resize(size_t size) {
            hSize = size/2;
            size = hSize * 2;
            m_state = std::make_unique<State<double, false>>(size);

            // Either use the existing working buffer (if it's big enough), or allocate a new one using std::vector<>
            size_t complexAlign = alignof(std::complex<double>);
            size_t work = size_t(m_state->workBuffer);
            size_t aligned = (work + (complexAlign - 1)) & ~(complexAlign - 1);

            if (work + m_state->workSize - aligned >= size * sizeof(std::complex<double>)) {
                  tmp = (std::complex<double> *) aligned;
                  tmpVector.resize(0);
            } else {
                  tmpVector.resize(size);
                  tmp = tmpVector.data();
            }
      }

      void fft(const double *input, std::complex<double>* output) {
            const auto status = ippsFFTFwd_RToPerm_64f((const Ipp64f*)input, (Ipp64f*)output, m_state->spec, m_state->workBuffer);
            checkStatus(status);
      }

      void fft(const double *inR, double *outR, double *outI) {
            const auto status = ippsFFTFwd_RToPerm_64f((const Ipp64f*)inR, (Ipp64f*)tmp, m_state->spec, m_state->workBuffer);
            checkStatus(status);
            for (size_t i = 0; i < hSize; ++i) {
                  outR[i] = tmp[i].real();
                  outI[i] = tmp[i].imag();
            }
      }

      void ifft(const std::complex<double>* input, double *output) {
            const auto status = ippsFFTInv_PermToR_64f((const Ipp64f*)input, (Ipp64f*)output, m_state->spec, m_state->workBuffer);
            checkStatus(status);
      }

      void ifft(const double *inR, const double *inI, double *outR) {
            for (size_t i = 0; i < hSize; ++i) {
                  tmp[i] = { inR[i], inI[i] };
            }
            const auto status = ippsFFTInv_PermToR_64f((const Ipp64f*)tmp, (Ipp64f*)outR, m_state->spec, m_state->workBuffer);
            checkStatus(status);
      }

private:
      std::unique_ptr<State<double, false>> m_state{ nullptr };
      std::complex<double>* tmp = nullptr;
      std::vector<std::complex<double>> tmpVector;
      size_t hSize;
};

}}

#endif // include guard
