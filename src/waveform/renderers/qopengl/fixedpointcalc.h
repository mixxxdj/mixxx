/// functions to do fixed point calculations, used by qopengl::WaveformRendererRGB

#include <cmath>

// float to fixed point with 8 fractional bits, clipped at 4.0
inline uint32_t toFrac8(float x) {
    return std::min<uint32_t>(static_cast<uint32_t>(std::max(x, 0.f) * 256.f), 4 * 256);
}

// scaled sqrt lookable table to convert maxAll and maxAllNext as calculated
// in updatePaintNode back to y coordinates
class Frac16SqrtTableSingleton {
  public:
    static constexpr size_t frac16sqrtTableSize{(3 * 4 * 255 * 256) / 16 + 1};

    static Frac16SqrtTableSingleton& getInstance() {
        static Frac16SqrtTableSingleton instance;
        return instance;
    }

    inline float get(uint32_t x) const {
        // The maximum value of fact16x can be (as uint32_t) 3 * 4 * 255 * 256,
        // which would be exessive for the table size. We divide by 16 in order
        // to get a more reasonable size.
        return m_table[x >> 4];
    }

  private:
    float* m_table;
    Frac16SqrtTableSingleton()
            : m_table(new float[frac16sqrtTableSize]) {
        // In the original implementation, the result of sqrt(maxAll) is divided
        // by sqrt(3 * 255 * 255);
        // We get rid of that division and bake it into this table.
        // Additionally, we divide the index for the lookup by 16 (see get(...)),
        // so we need to invert that here.
        const float f = (3.f * 255.f * 255.f / 16.f);
        for (uint32_t i = 0; i < frac16sqrtTableSize; i++) {
            m_table[i] = std::sqrt(static_cast<float>(i) / f);
        }
    }
    ~Frac16SqrtTableSingleton() {
        delete[] m_table;
    }
    Frac16SqrtTableSingleton(const Frac16SqrtTableSingleton&) = delete;
    Frac16SqrtTableSingleton& operator=(const Frac16SqrtTableSingleton&) = delete;
};

inline float frac16_sqrt(uint32_t x) {
    return Frac16SqrtTableSingleton::getInstance().get(x);
}

inline uint32_t frac8Pow2ToFrac16(uint32_t x) {
    // x is the result of multiplying two fixedpoint values with 8 fraction bits,
    // thus x has 16 fraction bits, which is also what we want to return for this
    // function. We would naively return (x * x) >> 16, but x * x would overflow
    // the 32 bits for values > 1, so we shift before multiplying.
    x >>= 8;
    return (x * x);
}

inline uint32_t math_max_u32(uint32_t a, uint32_t b, uint32_t c) {
    return std::max(a, std::max(b, c));
}
