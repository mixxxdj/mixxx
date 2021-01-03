#include <benchmark/benchmark.h>
#include <gtest/gtest.h>

#include <QtDebug>
#include <QList>
#include <QPair>

#include "util/sample.h"
#include "util/timer.h"

namespace {

class SampleUtilTest : public testing::Test {
  protected:
    void SetUp() override {
        sizes.append(1024);
        sizes.append(1025);
        sizes.append(1026);
        sizes.append(1027);
        sizes.append(1028);

        for (int i = 0; i < sizes.size(); ++i) {
            int size = sizes[i];
            CSAMPLE* buffer = SampleUtil::alloc(size);
            ClearBuffer(buffer, size);
            buffers.append(buffer);
            if (size % 2 == 0)
                evenBuffers.append(i);
        }
    }
    void TearDown() override {
        for (int i = 0; i < buffers.size(); ++i) {
            SampleUtil::free(buffers[i]);
        }
        buffers.clear();
        evenBuffers.clear();
        sizes.clear();
    }

    void ClearBuffer(CSAMPLE* pBuffer, int length) {
        SampleUtil::clear(pBuffer, length);
    }

    void FillBuffer(CSAMPLE* pBuffer, CSAMPLE value, int length) {
        SampleUtil::fill(pBuffer, value, length);
    }

    void AssertWholeBufferEquals(CSAMPLE* pBuffer, CSAMPLE value, int iBufferLen) {
        for (int i = 0; i < iBufferLen; ++i) {
            EXPECT_FLOAT_EQ(pBuffer[i], value);
        }
    }

    QList<int> sizes;
    QList<CSAMPLE*> buffers;
    QList<int> evenBuffers;
};

TEST_F(SampleUtilTest, allocIs16ByteAligned) {
    foreach (CSAMPLE* buffer, buffers) {
        ASSERT_EQ(0U, reinterpret_cast<quintptr>(buffer) % 16);
    }
}

TEST_F(SampleUtilTest, applyGain1DoesNothing) {
    for (int i = 0; i < buffers.size(); ++i) {
        CSAMPLE* buffer = buffers[i];
        int size = sizes[i];
        FillBuffer(buffer, 1.0f, size);
        SampleUtil::applyGain(buffer, 1.0, size);
        AssertWholeBufferEquals(buffer, 1.0, size);
    }
}

TEST_F(SampleUtilTest, applyGain0ClearsBuffer) {
    for (int i = 0; i < buffers.size(); ++i) {
        CSAMPLE* buffer = buffers[i];
        int size = sizes[i];
        FillBuffer(buffer, 1.0f, size);
        SampleUtil::applyGain(buffer, 0.0, size);
        AssertWholeBufferEquals(buffer, 0.0, size);
    }
}

TEST_F(SampleUtilTest, applyGain) {
    for (int i = 0; i < buffers.size(); ++i) {
        CSAMPLE* buffer = buffers[i];
        int size = sizes[i];
        FillBuffer(buffer, 1.0f, size);
        SampleUtil::applyGain(buffer, 0.5, size);
        AssertWholeBufferEquals(buffer, 0.5, size);
    }
}

TEST_F(SampleUtilTest, applyAlternatingGain) {
    for (int i = 0; i < evenBuffers.size(); ++i) {
        int j = evenBuffers[i];
        CSAMPLE* buffer = buffers[j];
        int size = sizes[j];
        FillBuffer(buffer, 1.0f, size);
        SampleUtil::applyAlternatingGain(buffer, 0.5, -0.5, size);
        for (int s = 0; s < size; s += 2) {
            EXPECT_FLOAT_EQ(buffer[s], 0.5);
            EXPECT_FLOAT_EQ(buffer[s+1], -0.5);
        }
    }
}

TEST_F(SampleUtilTest, addWithGain) {
    for (int i = 0; i < buffers.size(); ++i) {
        CSAMPLE* buffer = buffers[i];
        int size = sizes[i];
        FillBuffer(buffer, 1.0f, size);
        CSAMPLE* buffer2 = SampleUtil::alloc(size);
        FillBuffer(buffer2, 1.0f, size);
        SampleUtil::addWithGain(buffer, buffer2, 1.0, size);
        AssertWholeBufferEquals(buffer, 2.0f, size);
        SampleUtil::addWithGain(buffer, buffer2, 2.0, size);
        AssertWholeBufferEquals(buffer, 4.0f, size);
        SampleUtil::free(buffer2);
    }
}


TEST_F(SampleUtilTest, add2WithGain) {
    for (int i = 0; i < buffers.size(); ++i) {
        CSAMPLE* buffer = buffers[i];
        int size = sizes[i];
        FillBuffer(buffer, 1.0f, size);
        CSAMPLE* buffer2 = SampleUtil::alloc(size);
        FillBuffer(buffer2, 1.0f, size);
        CSAMPLE* buffer3 = SampleUtil::alloc(size);
        FillBuffer(buffer3, 1.0f, size);
        SampleUtil::add2WithGain(buffer,
                                 buffer2, 1.0,
                                 buffer3, 1.0,
                                 size);
        AssertWholeBufferEquals(buffer, 3.0f, size);
        SampleUtil::add2WithGain(buffer,
                                 buffer2, 2.0,
                                 buffer3, 3.0,
                                 size);
        AssertWholeBufferEquals(buffer, 8.0f, size);
        SampleUtil::free(buffer2);
        SampleUtil::free(buffer3);
    }
}

TEST_F(SampleUtilTest, add3WithGain) {
    for (int i = 0; i < buffers.size(); ++i) {
        CSAMPLE* buffer = buffers[i];
        int size = sizes[i];
        FillBuffer(buffer, 1.0f, size);
        CSAMPLE* buffer2 = SampleUtil::alloc(size);
        FillBuffer(buffer2, 1.0f, size);
        CSAMPLE* buffer3 = SampleUtil::alloc(size);
        FillBuffer(buffer3, 1.0f, size);
        CSAMPLE* buffer4 = SampleUtil::alloc(size);
        FillBuffer(buffer4, 1.0f, size);
        SampleUtil::add3WithGain(buffer,
                                 buffer2, 1.0,
                                 buffer3, 1.0,
                                 buffer4, 1.0,
                                 size);
        AssertWholeBufferEquals(buffer, 4.0f, size);
        SampleUtil::add3WithGain(buffer,
                                 buffer2, 2.0,
                                 buffer3, 3.0,
                                 buffer4, 4.0,
                                 size);
        AssertWholeBufferEquals(buffer, 13.0f, size);
        SampleUtil::free(buffer2);
        SampleUtil::free(buffer3);
        SampleUtil::free(buffer4);
    }
}

TEST_F(SampleUtilTest, copyWithGain) {
    for (int i = 0; i < buffers.size(); ++i) {
        CSAMPLE* buffer = buffers[i];
        int size = sizes[i];
        FillBuffer(buffer, 1.0f, size);
        CSAMPLE* buffer2 = SampleUtil::alloc(size);
        FillBuffer(buffer2, 1.0f, size);
        SampleUtil::copyWithGain(buffer, buffer2, 1.0, size);
        AssertWholeBufferEquals(buffer, 1.0f, size);
        SampleUtil::copyWithGain(buffer, buffer2, 2.0, size);
        AssertWholeBufferEquals(buffer, 2.0f, size);
        SampleUtil::free(buffer2);
    }
}

TEST_F(SampleUtilTest, copyWithGainAliased) {
    for (int i = 0; i < buffers.size(); ++i) {
        CSAMPLE* buffer = buffers[i];
        int size = sizes[i];
        FillBuffer(buffer, 1.0f, size);
        SampleUtil::copyWithGain(buffer, buffer, 2.0, size);
        AssertWholeBufferEquals(buffer, 2.0f, size);
    }
}

TEST_F(SampleUtilTest, copy2WithGain) {
    for (int i = 0; i < buffers.size(); ++i) {
        CSAMPLE* buffer = buffers[i];
        int size = sizes[i];
        FillBuffer(buffer, 1.0f, size);
        CSAMPLE* buffer2 = SampleUtil::alloc(size);
        FillBuffer(buffer2, 1.0f, size);
        CSAMPLE* buffer3 = SampleUtil::alloc(size);
        FillBuffer(buffer3, 1.0f, size);
        SampleUtil::copy2WithGain(buffer,
                                  buffer2, 1.0,
                                  buffer3, 1.0,
                                  size);
        AssertWholeBufferEquals(buffer, 2.0f, size);
        SampleUtil::copy2WithGain(buffer,
                                  buffer2, 2.0,
                                  buffer3, 3.0,
                                  size);
        AssertWholeBufferEquals(buffer, 5.0f, size);
        SampleUtil::free(buffer2);
        SampleUtil::free(buffer3);
    }
}

TEST_F(SampleUtilTest, copy3WithGain) {
    for (int i = 0; i < buffers.size(); ++i) {
        CSAMPLE* buffer = buffers[i];
        int size = sizes[i];
        FillBuffer(buffer, 1.0f, size);
        CSAMPLE* buffer2 = SampleUtil::alloc(size);
        FillBuffer(buffer2, 1.0f, size);
        CSAMPLE* buffer3 = SampleUtil::alloc(size);
        FillBuffer(buffer3, 1.0f, size);
        CSAMPLE* buffer4 = SampleUtil::alloc(size);
        FillBuffer(buffer4, 1.0f, size);
        SampleUtil::copy3WithGain(buffer,
                                  buffer2, 1.0,
                                  buffer3, 1.0,
                                  buffer4, 1.0,
                                  size);
        AssertWholeBufferEquals(buffer, 3.0f, size);
        SampleUtil::copy3WithGain(buffer,
                                  buffer2, 2.0,
                                  buffer3, 3.0,
                                  buffer4, 4.0,
                                  size);
        AssertWholeBufferEquals(buffer, 9.0f, size);
        SampleUtil::free(buffer2);
        SampleUtil::free(buffer3);
        SampleUtil::free(buffer4);
    }
}

TEST_F(SampleUtilTest, convertS16ToFloat32) {
    // Shorts are asymmetric, so SAMPLE_MAX is less than -SAMPLE_MIN.
    const float expectedMax = static_cast<float>(SAMPLE_MAX) /
                              static_cast<float>(-SAMPLE_MIN);
    for (int i = 0; i < buffers.size(); ++i) {
        CSAMPLE* buffer = buffers[i];
        int size = sizes[i];
        SAMPLE* s16 = new SAMPLE[size];
        FillBuffer(buffer, 1.0f, size);
        for (int j = 0; j < size; ++j) {
            s16[j] = SAMPLE_MAX;
        }
        SampleUtil::convertS16ToFloat32(buffer, s16, size);
        for (int j = 0; j < size; ++j) {
            EXPECT_FLOAT_EQ(expectedMax, buffer[j]);
        }
        FillBuffer(buffer, 0.0f, size);
        for (int j = 0; j < size; ++j) {
            s16[j] = 0;
        }
        SampleUtil::convertS16ToFloat32(buffer, s16, size);
        for (int j = 0; j < size; ++j) {
            EXPECT_FLOAT_EQ(0.0f, buffer[j]);
        }
        FillBuffer(buffer, -1.0f, size);
        for (int j = 0; j < size; ++j) {
            s16[j] = SAMPLE_MIN;
        }
        SampleUtil::convertS16ToFloat32(buffer, s16, size);
        for (int j = 0; j < size; ++j) {
            EXPECT_FLOAT_EQ(-1.0f, buffer[j]);
        }
        delete [] s16;
    }
}

TEST_F(SampleUtilTest, sumAbsPerChannel) {
    for (int i = 0; i < evenBuffers.size(); ++i) {
        int j = evenBuffers[i];
        CSAMPLE* buffer = buffers[j];
        int size = sizes[j];
        FillBuffer(buffer, 1.0f, size);
        CSAMPLE fSumL = 0, fSumR = 0;
        SampleUtil::applyAlternatingGain(buffer, 1.0, 2.0, size);
        SampleUtil::sumAbsPerChannel(&fSumL, &fSumR, buffer, size);
        EXPECT_FLOAT_EQ(fSumL, size/2);
        EXPECT_FLOAT_EQ(fSumR, size);
    }
}

TEST_F(SampleUtilTest, interleaveBuffer) {
    for (int i = 0; i < buffers.size(); ++i) {
        CSAMPLE* buffer = buffers[i];
        int size = sizes[i];
        FillBuffer(buffer, 0.0f, size);
        CSAMPLE* buffer2 = SampleUtil::alloc(size);
        FillBuffer(buffer2, 0.0f, size);
        for (int j = 0; j < size; j++) {
            buffer[j] = j;
            buffer2[j] = -j;
        }
        CSAMPLE* buffer3 = SampleUtil::alloc(size*2);
        FillBuffer(buffer3, 0.0f, size*2);
        SampleUtil::interleaveBuffer(buffer3, buffer, buffer2, size);

        for (int j = 0; j < size; j++) {
            EXPECT_FLOAT_EQ(buffer3[j*2], j);
            EXPECT_FLOAT_EQ(buffer3[j*2+1], -j);
        }
    }
}

TEST_F(SampleUtilTest, deinterleaveBuffer) {
    for (int i = 0; i < buffers.size(); ++i) {
        CSAMPLE* buffer = buffers[i];
        int size = sizes[i];
        FillBuffer(buffer, 0.0f, size);
        CSAMPLE* buffer2 = SampleUtil::alloc(size);
        FillBuffer(buffer2, 0.0f, size);
        CSAMPLE* buffer3 = SampleUtil::alloc(size*2);
        FillBuffer(buffer3, 1.0f, size*2);
        for (int j = 0; j < size; j++) {
            buffer3[j*2] = j;
            buffer3[j*2+1] = -j;
        }
        SampleUtil::deinterleaveBuffer(buffer, buffer2, buffer3, size);

        for (int j = 0; j < size; j++) {
            EXPECT_FLOAT_EQ(buffer[j], j);
            EXPECT_FLOAT_EQ(buffer2[j], -j);
        }

        SampleUtil::free(buffer2);
        SampleUtil::free(buffer3);
    }
}

TEST_F(SampleUtilTest, reverse) {
    if (buffers.size() > 0 && sizes[0] > 10) {
        CSAMPLE* buffer = buffers[1];
        for (int i = 0; i < 10; ++i) {
            buffer[i] = i * 0.1f;
        }

        SampleUtil::reverse(buffer, 10);

        // check if right channel remains at odd index
        EXPECT_FLOAT_EQ(buffer[0], 0.8f);
        EXPECT_FLOAT_EQ(buffer[1], 0.9f);
        EXPECT_FLOAT_EQ(buffer[2], 0.6f);
        EXPECT_FLOAT_EQ(buffer[3], 0.7f);
        EXPECT_FLOAT_EQ(buffer[4], 0.4f);
        EXPECT_FLOAT_EQ(buffer[5], 0.5f);
        EXPECT_FLOAT_EQ(buffer[6], 0.2f);
        EXPECT_FLOAT_EQ(buffer[7], 0.3f);
        EXPECT_FLOAT_EQ(buffer[8], 0.0f);
        EXPECT_FLOAT_EQ(buffer[9], 0.1f);
    }
}

TEST_F(SampleUtilTest, copyReverse) {
    if (buffers.size() > 1 && sizes[0] > 10 && sizes[1] > 10)  {
        CSAMPLE* source = buffers[0];
        CSAMPLE* destination = buffers[1];
        for (int i = 0; i < 10; ++i) {
            source[i] = i * 0.1f;
        }

        SampleUtil::copyReverse(destination, source, 10);

        // check if right channel remains at odd index
        EXPECT_FLOAT_EQ(destination[0], 0.8f);
        EXPECT_FLOAT_EQ(destination[1], 0.9f);
        EXPECT_FLOAT_EQ(destination[2], 0.6f);
        EXPECT_FLOAT_EQ(destination[3], 0.7f);
        EXPECT_FLOAT_EQ(destination[4], 0.4f);
        EXPECT_FLOAT_EQ(destination[5], 0.5f);
        EXPECT_FLOAT_EQ(destination[6], 0.2f);
        EXPECT_FLOAT_EQ(destination[7], 0.3f);
        EXPECT_FLOAT_EQ(destination[8], 0.0f);
        EXPECT_FLOAT_EQ(destination[9], 0.1f);
    }
}

static void BM_MemCpy(benchmark::State& state) {
    SINT size = static_cast<SINT>(state.range(0));
    CSAMPLE* buffer = SampleUtil::alloc(size);
    SampleUtil::fill(buffer, 0.0f, size);
    CSAMPLE* buffer2 = SampleUtil::alloc(size);
    SampleUtil::fill(buffer2, 0.0f, size);

    while(state.KeepRunning()) {
        memcpy(buffer, buffer2, size * sizeof(CSAMPLE));
    }

    SampleUtil::free(buffer);
    SampleUtil::free(buffer2);
}
BENCHMARK(BM_MemCpy)->Range(64, 4096);

static void BM_StdCpy(benchmark::State& state) {
    SINT size = static_cast<SINT>(state.range(0));
    CSAMPLE* buffer = SampleUtil::alloc(size);
    SampleUtil::fill(buffer, 0.0f, size);
    CSAMPLE* buffer2 = SampleUtil::alloc(size);
    SampleUtil::fill(buffer2, 0.0f, size);

    while(state.KeepRunning()) {
        std::copy(buffer2, buffer2 + size, buffer);
    }

    SampleUtil::free(buffer);
    SampleUtil::free(buffer2);
}
BENCHMARK(BM_StdCpy)->Range(64, 4096);

static void BM_SampleUtilCopy(benchmark::State& state) {
    SINT size = static_cast<SINT>(state.range(0));
    CSAMPLE* buffer = SampleUtil::alloc(size);
    SampleUtil::fill(buffer, 0.0f, size);
    CSAMPLE* buffer2 = SampleUtil::alloc(size);
    SampleUtil::fill(buffer2, 0.0f, size);

    while(state.KeepRunning()) {
        SampleUtil::copy(buffer, buffer2, size);
    }

    SampleUtil::free(buffer);
    SampleUtil::free(buffer2);
}
BENCHMARK(BM_SampleUtilCopy)->Range(64, 4096);


/*
TEST_F(SampleUtilTest, copy3WithGainSpeed) {
    CSAMPLE* buffer = buffers[0];

    int size = sizes[0] - (rand() % 2) * 8; // prevent predicting loop size
    FillBuffer(buffer, 0.0f, size);
    CSAMPLE* buffer2 = SampleUtil::alloc(size);
    FillBuffer(buffer2, 0.0f, size);
    CSAMPLE* buffer3 = SampleUtil::alloc(size*2);
    FillBuffer(buffer3, 1.0f, size*2);
    for (int j = 0; j < size; j++) {
        buffer3[j*2] = j;
        buffer3[j*2+1] = -j;
    }

    // For ensure data is cached and equal start conditions
    SampleUtil::copy(buffer, buffer2, size);
    SampleUtil::copy2WithGain(buffer, buffer2, 1.1f, buffer3, 1.1f, size);

    qint64 elapsed;
    Timer t("");
    t.start();

    SampleUtil::copy2WithGain(buffer, buffer2, 1.1f, buffer3, 1.1f, size);

    elapsed = t.elapsed("");
    qDebug() << "copy2WithGain" << elapsed << "ns" << size;

//#########

    t.start();

    SampleUtil::copy1WithGain(buffer, buffer2, 1.1f, size);

    elapsed = t.elapsed("");
    qDebug() << "SampleUtil::copy1WithGain" << elapsed << "ns" << size;

//############

    t.start();

    SampleUtil::copy1WithGain(buffer, buffer2, 1.1f, size);

    elapsed = t.elapsed("");
    qDebug() << "SampleUtil::copy1WithGain" << elapsed << "ns" << size;

//############

    t.start();

    SampleUtil::addWithGain(buffer, buffer2, 1.1f, size);

    elapsed = t.elapsed("");
    qDebug() << "SampleUtil::addWithGain" << elapsed << "ns" << size;


//############

    t.start();

    CSAMPLE* __restrict__ buffer_ = buffer;
    CSAMPLE* __restrict__ buffer1_ = buffer2;
    CSAMPLE* __restrict__ buffer2_ = buffer3;

    for (int i = 0; i < size; ++i) {
        buffer_[i] += buffer1_[i] * 1.1f;
    }

    elapsed = t.elapsed("");
    qDebug() << "addWithGain as for loop" << elapsed << "ns" << size;

//############

    t.start();

    SampleUtil::addWithGain(buffer, buffer2, 1.1f, size);

    elapsed = t.elapsed("");
    qDebug() << "SampleUtil::addWithGain" << elapsed << "ns" << size;

//############



    t.start();

    SampleUtil::copy2WithGain(buffer, buffer2, 1.1f, buffer3, 1.1f, size);

    elapsed = t.elapsed("");
    qDebug() << "copy2WithGain" << elapsed << "ns" << size;

//#########

    SampleUtil::free(buffer2);
    SampleUtil::free(buffer3);
}
*/

static void BM_Copy2WithGain(benchmark::State& state) {
    SINT size = static_cast<SINT>(state.range(0));
    CSAMPLE* buffer = SampleUtil::alloc(size);
    SampleUtil::fill(buffer, 0.0f, size);
    CSAMPLE* buffer2 = SampleUtil::alloc(size);
    SampleUtil::fill(buffer2, 0.0f, size);
    CSAMPLE* buffer3 = SampleUtil::alloc(size);
    SampleUtil::fill(buffer3, 0.0f, size);

    while(state.KeepRunning()) {
        SampleUtil::copy2WithGain(buffer, buffer2, 1.1f, buffer3, 1.1f, size);
    }

    SampleUtil::free(buffer);
    SampleUtil::free(buffer2);
    SampleUtil::free(buffer3);
}
BENCHMARK(BM_Copy2WithGain)->Range(64, 4096);

static void BM_Copy2WithRampingGain(benchmark::State& state) {
    SINT size = static_cast<SINT>(state.range(0));
    CSAMPLE* buffer = SampleUtil::alloc(size);
    SampleUtil::fill(buffer, 0.0f, size);
    CSAMPLE* buffer2 = SampleUtil::alloc(size);
    SampleUtil::fill(buffer2, 0.0f, size);
    CSAMPLE* buffer3 = SampleUtil::alloc(size);
    SampleUtil::fill(buffer3, 0.0f, size);

    while(state.KeepRunning()) {
        SampleUtil::copy2WithRampingGain(buffer, buffer2, 1.1f, 1.2f, buffer3,
                                         1.1f, 1.2f, size);
    }

    SampleUtil::free(buffer);
    SampleUtil::free(buffer2);
    SampleUtil::free(buffer3);
}
BENCHMARK(BM_Copy2WithRampingGain)->Range(64, 4096);

}  // namespace
